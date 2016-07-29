/******************************************************************************
*
*  PaxosSessionServer.c 	--- server part of Paxos-Session Library
*
*  Copyright (C) 2010-2016 triTech Inc. All Rights Reserved.
*
*  See GPL-LICENSE.txt for copyright and license details.
*
*  This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation; either version 3 of the License, or (at your option) any later
* version. 
*
*  This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with
* this program. If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/*!
 *	$B:n@.(B			$BEOJUE59'(B
 *	$B;n83(B			$B1v?,>o=U!"5WJ]@56H(B
 *	$BFC5v!"Cx:n8"(B	$B%H%i%$%F%C%/(B
 *					$BFC4j(B2010-538664(PCT/JP2010/066906)
 *
 */

//#define	_LOCK_   /*** define -> LOCK Debug printing ON ***/

#include	<limits.h>

#define	_PAXOS_SESSION_SERVER_
#include	"PaxosSession.h"

#define	SLOG( pAcceptor, Level, fmt, ... ) \
	if( pAcceptor->c_pLog != NULL  ) \
		Log( pAcceptor->c_pLog, Level, " %d [%ld:%u] %s-%u %s:"fmt, \
			Level, _ThId(pthread_self()), time(0), \
			__FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define	BACKUP_SIZE_ONCE	100000000LL	/* 100M */

int
PaxosAcceptorLogCreate( PaxosAcceptor_t* pAcceptor, 
						int Size, int Flags, FILE* pFile, int Level )
{
	ASSERT( pAcceptor->c_pLog == NULL );
	pAcceptor->c_pLog	= LogCreate( Size, 
					pAcceptor->c_IF.if_fMalloc, pAcceptor->c_IF.if_fFree,
					Flags, pFile, Level );
	return( 0 );
}

void
PaxosAcceptorLogDestroy( PaxosAcceptor_t* pAcceptor )
{
	LogDestroy( pAcceptor->c_pLog );
	pAcceptor->c_pLog	= NULL;
}

struct Log*
PaxosAcceptorGetLog( PaxosAcceptor_t* pAcceptor )
{
	return( pAcceptor->c_pLog );
}

int
PaxosAcceptorSetLog( PaxosAcceptor_t* pAcceptor, struct Log* pLog )
{
	pAcceptor->c_pLog	= pLog;
	return( 0 );
}

struct Log*
PaxosAcceptGetLog( PaxosAccept_t* pAccept )
{
	return( pAccept->a_pAcceptor->c_pLog );
}

void*
(*PaxosAcceptorGetfMalloc( PaxosAcceptor_t* pAcceptor ))(size_t)
{
	return( pAcceptor->c_IF.if_fMalloc );
}

void
(*PaxosAcceptorGetfFree( PaxosAcceptor_t* pAcceptor ))(void*)
{
	return( pAcceptor->c_IF.if_fFree );
}

void*
(*PaxosAcceptGetfMalloc( PaxosAccept_t* pAccept ))(size_t)
{
	return( pAccept->a_pAcceptor->c_IF.if_fMalloc );
}

void
(*PaxosAcceptGetfFree( PaxosAccept_t* pAccept ))(void*)
{
	return( pAccept->a_pAcceptor->c_IF.if_fFree );
}

int
_AcceptSet( GElmCtrl_t *pGE, GElm_t *pE, 
			void *pKey, void **ppKey, void *pArg, bool_t bLoad )
{
	PaxosAccept_t*	pA = container_of( pE, PaxosAccept_t, a_GE );
	PaxosClientId_t* pId = (PaxosClientId_t*)pKey;

	pA->a_Id 	= *pId;

	*ppKey	= &pA->a_Id;

	return( 0 );
}

int
_AcceptUnset( GElmCtrl_t *pGE, GElm_t *pE, void **ppKey, bool_t bSave )
{
	PaxosAccept_t*	pA = container_of( pE, PaxosAccept_t, a_GE );
	Msg_t*			pMsg;

	ASSERT( pA );

	if( pA->a_Opened )	return( -1 );

	while( (pMsg = list_first_entry( &pA->a_Rpls.r_Msg, Msg_t, m_Lnk ) ) ) {
		list_del_init( &pMsg->m_Lnk );
		MsgDestroy( pMsg );
	}
	if( pA->a_Events.e_pRaised ) {
		MsgDestroy( pA->a_Events.e_pRaised );
		pA->a_Events.e_pRaised = NULL;
	}
	HashDestroy( &pA->a_Tag );

	*ppKey	= &pA->a_Id;

	return( 0 );
}

GElm_t*
_AcceptAlloc( GElmCtrl_t *pGE )
{
	PaxosAcceptor_t	*pAcceptor = pGE->ge_pTag;
	PaxosAccept_t*	pA;

	pA = (*pAcceptor->c_IF.if_fMalloc)( sizeof(PaxosAccept_t) );

	memset( pA, 0, sizeof(*pA) );

	MtxInit( &pA->a_Mtx );

	pA->a_pFd	= NULL;

	pA->a_Rpls.r_Hold		= FALSE;
	list_init( &pA->a_Rpls.r_Msg );

	pA->a_pAcceptor	= pAcceptor;
	pA->a_Opened	= FALSE;

	CndInit( &pA->a_Accepted.a_Cnd );
	pA->a_Accepted.a_Cmd	= -1;

	pA->a_Events.e_Cnt = 0; 
	pA->a_Events.e_Send = FALSE;

	HashInit( &pA->a_Tag, PRIMARY_1009, HASH_CODE_INT, HASH_CMP_INT,
		NULL, pAcceptor->c_IF.if_fMalloc, pAcceptor->c_IF.if_fFree, NULL );

	pA->a_Create	= time(0);

	return( &pA->a_GE );
}

int
_AcceptFree( GElmCtrl_t *pGE, GElm_t *pE )
{
	PaxosAcceptor_t	*pAcceptor = pGE->ge_pTag;
	PaxosAccept_t*	pA = container_of( pE, PaxosAccept_t, a_GE );

	(*pAcceptor->c_IF.if_fFree)( pA );

	return( 0 );
}

int
HashSessionCode( void* pKey )
{
	PaxosClientId_t* pId = (PaxosClientId_t*)pKey;
	uint32_t	Ret;

	DBG_ENT();

	Ret	= (uint32_t)(pId->i_Addr.sin_addr.s_addr) + pId->i_Pid + pId->i_No;

	RETURN( (int)Ret );
}

int
HashSessionCompare( void* pKeyA, void* pKeyB )
{
	PaxosClientId_t* pIdA = (PaxosClientId_t*)pKeyA;
	PaxosClientId_t* pIdB = (PaxosClientId_t*)pKeyB;

	DBG_ENT();

	if( pIdA->i_Addr.sin_addr.s_addr == pIdB->i_Addr.sin_addr.s_addr
		&& pIdA->i_Pid == pIdB->i_Pid 
		&& pIdA->i_Time == pIdB->i_Time 
		&& pIdA->i_No == pIdB->i_No 
		&& pIdA->i_Reuse == pIdB->i_Reuse )
			RETURN( 0 );
	else	RETURN( -1 );
}

int
PaxosAcceptorGetSize()
{
	return( sizeof(PaxosAcceptor_t) );
}

struct Paxos*
PaxosAcceptorGetPaxos( PaxosAcceptor_t* pAcceptor )
{
	return( pAcceptor->c_pPaxos );
}

int
PaxosAcceptorMyId( PaxosAcceptor_t *pAcceptor )
{
	return( PaxosMyId( pAcceptor->c_pPaxos ) );
}

int _AcceptPut( PaxosAcceptor_t* pAcceptor, PaxosAccept_t* pAccept );

#define	SNAP_ACCEPT		1
#define	MARSHAL_END		-1

int
_PaxosAcceptorMarshal( IOReadWrite_t *pW, PaxosAcceptor_t* pAcceptor )
{
	int	Ret = 0;
	PaxosAccept_t*	pAccept;
	int32_t	tag;
	Msg_t*	pMsg;
	int	i;

//DBG_PRT("=== Marshal START ===\n" );
	list_for_each_entry( pAccept, &pAcceptor->c_Accept.ge_Lnk, a_GE.e_Lnk ) {

		if( pAccept->a_Accepted.a_Cmd == PAXOS_SESSION_CLOSE )	continue;

		tag = SNAP_ACCEPT;
		IOMarshal( pW, (char*)&tag, sizeof(tag) );

		IOMarshal( pW, (char*)pAccept, sizeof(*pAccept) );

		list_for_each_entry( pMsg, &pAccept->a_Rpls.r_Msg, m_Lnk ) {

			IOMarshal( pW, (char*)pMsg, sizeof(*pMsg) );

			for( i = 0; i < pMsg->m_N; i++ ) {

				IOMarshal( pW, (char*)&pMsg->m_aVec[i], 
										sizeof(pMsg->m_aVec[i]) );
				IOMarshal( pW, pMsg->m_aVec[i].v_pVoid, 
										pMsg->m_aVec[i].v_Size );
			}
		}
		if( (pMsg = pAccept->a_Events.e_pRaised) ) {

			IOMarshal( pW, (char*)pMsg, sizeof(*pMsg) );

			for( i = 0; i < pMsg->m_N; i++ ) {

				IOMarshal( pW, (char*)&pMsg->m_aVec[i], 
										sizeof(pMsg->m_aVec[i]) );
				IOMarshal( pW, pMsg->m_aVec[i].v_pVoid, 
										pMsg->m_aVec[i].v_Size );
			}
		}
		if( pAccept->a_Opened && pAcceptor->c_IF.if_fAdaptorMarshal ) {
			Ret = (pAcceptor->c_IF.if_fAdaptorMarshal)( pW, pAccept );
		}
	}
//DBG_PRT("=== Marshal End ===\n");

	return( Ret );
}

int
PaxosAcceptorUnmarshal( IOReadWrite_t *pR, PaxosAcceptor_t* pAcceptor )
{
	PaxosAccept_t* pAccept, Accept;
	int	j;
	int	Ret = 0;
	int32_t	tag;
	Msg_t		*pMsg, Msg;
	MsgVec_t	Vec;
	int			i;
	char		*pBuf;

//DBG_PRT("=== Unmarshal START ===\n" );
	while( IOUnmarshal( pR, &tag, sizeof(tag) ) > 0 ) {
		switch( tag ) {
		case SNAP_ACCEPT:
			IOUnmarshal( pR, (char*)&Accept, sizeof(Accept) );
			pAccept	= PaxosAcceptGet( pAcceptor, &Accept.a_Id, TRUE );
			ASSERT( pAccept );
			off_t off = sizeof(GElm_t);
			memcpy( (void *)pAccept + off, (void *)&Accept + off, 
											sizeof(*pAccept) - off);

			MtxInit( &pAccept->a_Mtx );
			CndInit( &pAccept->a_Accepted.a_Cnd );
			pAccept->a_Accepted.a_Cnt	= 0;

			pAccept->a_Create	= time(0);
			pAccept->a_pFd 			= NULL;

			HashInit( &pAccept->a_Tag, PRIMARY_1009, 
				HASH_CODE_INT, HASH_CMP_INT,
				NULL, pAcceptor->c_IF.if_fMalloc, pAcceptor->c_IF.if_fFree, 
				NULL );
			pAccept->a_pAcceptor	= pAcceptor;

			list_init( &pAccept->a_Rpls.r_Msg );

			for( j = 0; j < pAccept->a_Rpls.r_Cnt; j++ ) {

				IOUnmarshal( pR, (char*)&Msg, sizeof(Msg) );
				pMsg	= MsgCreate( Msg.m_Size,
										pAcceptor->c_IF.if_fMalloc,
										pAcceptor->c_IF.if_fFree );
				for( i = 0; i < Msg.m_N; i++ ) {
					IOUnmarshal( pR, (char*)&Vec, sizeof(Vec) );
					pBuf	= MsgMalloc( pMsg, Vec.v_Size );			
					IOUnmarshal( pR, pBuf, Vec.v_Size );
					Vec.v_pStart	= 
							pBuf + (Vec.v_pStart - (char*)Vec.v_pVoid);
					Vec.v_pVoid		= pBuf;
					Vec.v_Flags	|= VEC_MINE;
					MsgAdd( pMsg, &Vec );
				}
				list_add_tail( &pMsg->m_Lnk, &pAccept->a_Rpls.r_Msg);
			}
			if( pAccept->a_Events.e_Cnt > 0 ) {

				IOUnmarshal( pR, (char*)&Msg, sizeof(Msg) );
				pMsg	= MsgCreate( Msg.m_Size,
										pAcceptor->c_IF.if_fMalloc,
										pAcceptor->c_IF.if_fFree );
				for( i = 0; i < Msg.m_N; i++ ) {
					IOUnmarshal( pR, (char*)&Vec, sizeof(Vec) );
					pBuf	= MsgMalloc( pMsg, Vec.v_Size );			
					IOUnmarshal( pR, pBuf, Vec.v_Size );
					Vec.v_pStart	= 
							pBuf + (Vec.v_pStart - (char*)Vec.v_pVoid);
					Vec.v_pVoid		= pBuf;
					Vec.v_Flags	|= VEC_MINE;
					MsgAdd( pMsg, &Vec );
				}
				pAccept->a_Events.e_pRaised	= pMsg;
			}
			if( pAccept->a_Opened && pAcceptor->c_IF.if_fAdaptorUnmarshal ) {
				Ret = (pAcceptor->c_IF.if_fAdaptorUnmarshal)( pR, pAccept );
			}
			break;

		default:
			ABORT();
			break;
		}
	}
//DBG_PRT("=== Unmarshal END ===\n");
	return( Ret );
}

void*
PaxosAcceptGetTag( PaxosAccept_t* pAccept, int Family )
{
	void*	p;

	MtxLock( &pAccept->a_Mtx );

	p = HashGet( &pAccept->a_Tag, INT32_PNT( Family ) );

	MtxUnlock( &pAccept->a_Mtx );
	return( p );
}

void
PaxosAcceptSetTag( PaxosAccept_t* pAccept, int Family, void* pTag )
{

	MtxLock( &pAccept->a_Mtx );

	if( pTag )	HashPut( &pAccept->a_Tag,INT32_PNT(Family),pTag);
	else		HashRemove( &pAccept->a_Tag, INT32_PNT(Family) );

	MtxUnlock( &pAccept->a_Mtx );
}

#define	ACCEPTED_SEQ( pAccept, pH ) \
	({	(pAccept)->a_Id.i_Seq		= (pH)->h_Id.i_Seq; \
		(pAccept)->a_Accepted.a_Start	= (pH)->h_Id.i_Seq; \
		(pAccept)->a_Accepted.a_Cmd	= (pH)->h_Cmd;})

PaxosAccept_t*
PaxosAcceptGet( PaxosAcceptor_t* pAcceptor, 
					PaxosClientId_t* pId, bool_t bCreate )
{
	PaxosAccept_t*	pAccept;

	pAccept	= (PaxosAccept_t*)GElmGet( &pAcceptor->c_Accept, pId, 
										NULL, bCreate, TRUE );

	return( pAccept );
}

int
PaxosAcceptPut( PaxosAccept_t* pAccept )
{
	PaxosAcceptor_t*	pAcceptor;
	int	Ret;

	DBG_ENT();

	pAcceptor	= pAccept->a_pAcceptor;

	Ret = GElmPut( &pAcceptor->c_Accept, &pAccept->a_GE, TRUE, TRUE );

	DBG_EXT( Ret );
	return( Ret );
}

PaxosClientId_t*
PaxosAcceptorGetClientId( PaxosAccept_t* pAccept )
{
	return( &pAccept->a_Id );
}

int _PaxosAcceptReplyByMsg( PaxosAccept_t* pAccept, Msg_t* );

int
_PaxosAcceptIsAccepted( PaxosAccept_t* pAccept, PaxosSessionHead_t* pH )
{
	int	Ret = 0;
	int	n;

	ASSERT( pAccept );

	if( pH->h_Cmd == PAXOS_SESSION_OPEN
		&& pAccept->a_Accepted.a_Cmd != -1
		&& pAccept->a_Accepted.a_Cmd != PAXOS_SESSION_OPEN ) {

		SLOG( pAccept->a_pAcceptor, LogINF,
		"### REUSE Pid=%d-%d-%d Start=%d End=%d Reply=%d Cmd=0x%x ###",
		pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, pAccept->a_Id.i_Reuse,
		pAccept->a_Accepted.a_Start, pAccept->a_Accepted.a_End,
		pAccept->a_Accepted.a_Reply, pAccept->a_Accepted.a_Cmd );

#if 1
		while ( pAccept->a_Opened ) {
			MtxUnlock( &pAccept->a_Mtx );

			SLOG(pAccept->a_pAcceptor, LogINF,
			"pAccept=%p, e_Cnt=%d, i_No=%d",
			pAccept, pAccept->a_GE.e_Cnt, pH->h_Id.i_No);
			usleep(20000);

			MtxLock( &pAccept->a_Mtx );
		}
#endif

		pAccept->a_Opened = FALSE;
		pAccept->a_Accepted.a_Cnt = 0;
		pAccept->a_Accepted.a_Start = 0;
		pAccept->a_Accepted.a_End = 0;
		pAccept->a_Accepted.a_Reply = 0;
		pAccept->a_Accepted.a_Master = FALSE;
		pAccept->a_Accepted.a_Cmd = -1;
		pAccept->a_Rpls.r_Cnt = 0;
		pAccept->a_Rpls.r_Hold = FALSE;
		pAccept->a_Events.e_Cnt = 0;
		pAccept->a_Events.e_Omitted = 0;
		pAccept->a_Events.e_Send = FALSE;
	}
	if( CMP( pH->h_Id.i_Seq, pAccept->a_Accepted.a_Reply ) < 0 ) {
/* $B=EJ#MW5a$G1~Ez:Q$_(B ??? */
		SLOG( pAccept->a_pAcceptor, LogERR,
		"##### ALREADY REPLYED pAccept=%p "
		"Pid=%d-%d Seq=%d Try=%d Cmd=%d Start=%d End=%d Reply=%d", 
		pAccept, pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Seq, pH->h_Id.i_Try,
		pH->h_Cmd,
		pAccept->a_Accepted.a_Start, pAccept->a_Accepted.a_End, 
		pAccept->a_Accepted.a_Reply);
		return( -1 );
	}

	if( pAccept->a_Accepted.a_Reply == pH->h_Id.i_Seq ) {
		if( pAccept->a_Accepted.a_Cmd == pH->h_Cmd ) {

			n = _PaxosAcceptReplyByMsg( pAccept, NULL );

			SLOG( pAccept->a_pAcceptor, LogERR,
			"##### DUPRICATE REPLY pAccept=%p "
			"Pid=%d-%d Seq=%d Cmd=%d n=%d", 
			pAccept, pH->h_Id.i_Pid, pH->h_Id.i_No, 
			pH->h_Id.i_Seq, pH->h_Cmd , n );

			Ret = 1;
		} else {
ASSERT( CMP(pAccept->a_Accepted.a_Start, pH->h_Id.i_Seq) > 0 );
			Ret = -2;
		}
	} else if( pAccept->a_Accepted.a_Start == pH->h_Id.i_Seq
			&& pAccept->a_Accepted.a_Cmd == pH->h_Cmd ) {

		SLOG( pAccept->a_pAcceptor, LogERR,
		"##### DUPRICATE REQUEST IGNORE Start=%u End=%u Reply=%u "
		"Pid=%d-%d Seq=%d Cmd=%d", 
		pAccept->a_Accepted.a_Start, pAccept->a_Accepted.a_End, 
		pAccept->a_Accepted.a_Reply,
		pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Seq, pH->h_Cmd );

		Ret = -3;
	}
	return( Ret );
}

int
PaxosAcceptIsAccepted( PaxosAccept_t* pAccept, PaxosSessionHead_t* pH )
{
	int	Ret;

	MtxLock( &pAccept->a_Mtx );

	Ret = _PaxosAcceptIsAccepted( pAccept, pH );

	MtxUnlock( &pAccept->a_Mtx );

	return( Ret );
}

/*
 *	$B%7!<%1%s%9$NDI$$1[$7$r6X;_$9$k(B
 *		$B%j!<%@8rBX;~$K99?7$H;2>H$,<j=gA08e(B
 *		$BD>A099?7$rBT$D(B
 */
int
PaxosAcceptedStart( PaxosAccept_t* pAccept, PaxosSessionHead_t* pH )
{
	int	Ret;

	MtxLock( &pAccept->a_Mtx );

	if( pH->h_Cmd == PAXOS_SESSION_OPEN )	goto set;

	while( CMP( pH->h_Id.i_SeqWait, pAccept->a_Accepted.a_End ) > 0 ) {

		pAccept->a_Accepted.a_Cnt++;

		Ret = CndTimedWaitWithMs( &pAccept->a_Accepted.a_Cnd, 
			&pAccept->a_Mtx,
			PaxosDecidedTimeout( pAccept->a_pAcceptor->c_pPaxos )*1000 );
		if( Ret ) {

			SLOG( pAccept->a_pAcceptor, LogERR,
			"TIMEOUT pFd=%p Opened=%d Start=%u End=%d Cnt=%d "
			"i_Pid=%d-%d i_Seq=%u i_SeqWait=%u", 
			pAccept->a_pFd, pAccept->a_Opened, 
			pAccept->a_Accepted.a_Start, pAccept->a_Accepted.a_End, 
			pAccept->a_Accepted.a_Cnt,
			pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Seq, pH->h_Id.i_SeqWait );
		}
		pAccept->a_Accepted.a_Cnt--;
	}
set:
	ACCEPTED_SEQ( pAccept, pH );

	MtxUnlock( &pAccept->a_Mtx );

	return( 0 );

}

int
PaxosAcceptedEnd( PaxosAccept_t* pAccept, PaxosClientId_t* pId )
{
	MtxLock( &pAccept->a_Mtx );

ASSERT( pId->i_Seq == pAccept->a_Accepted.a_Start );

	pAccept->a_Accepted.a_End	= pId->i_Seq;

	if( pAccept->a_Accepted.a_Cnt > 0 ) {
		CndBroadcast( &pAccept->a_Accepted.a_Cnd );
	}

	MtxUnlock( &pAccept->a_Mtx );

	return( 0 );
}

/*
 *	Called by Timer Thread
 */
int
_SessionAbortSend( PaxosAcceptor_t *pAcceptor, PaxosAccept_t *pAccept )
{
	PaxosSessionHead_t	M;
	int	Ret;

	SESSION_MSGINIT( &M, PAXOS_SESSION_ABORT, PAXOS_OK, 0, sizeof(M) );

	M.h_Master	= pAcceptor->c_MyId;
	M.h_Id	= pAccept->a_Id;
	M.h_Id.i_Seq++;

	Ret = PaxosRequest( pAcceptor->c_pPaxos, M.h_Len, &M, FALSE );

	SLOG( pAcceptor, LogERR,
	"#### ABORT(Ret=%d) pAccept=%p "
	"INET=%s PID=%d-%d Opened=%d Cnt=%d pFd=%p Create=%u Access=%u Health=%u",
	Ret, pAccept, inet_ntoa( pAccept->a_Id.i_Addr.sin_addr), 
	pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, 
	pAccept->a_Opened, pAccept->a_GE.e_Cnt, pAccept->a_pFd, 
	(uint32_t)pAccept->a_Create, (uint32_t)pAccept->a_Access, 
	(uint32_t)pAccept->a_Health );

	return( Ret );
}

int
SessionGarbage( TimerEvent_t* pE )
{
	PaxosAcceptor_t* pAcceptor = (PaxosAcceptor_t*)pE->e_pArg;
	PaxosAccept_t	*pAccept;
	struct Paxos	*pPaxos = pAcceptor->c_pPaxos;

//	DBG_ENT();

	//SLOG( pAcceptor, "### Garbage ENT");

	if( !PaxosIsIamMaster( pAcceptor->c_pPaxos ) ) RETURN( 0 );

//DBG_PRT("ENT\n");
	GElmCtrlLock( &pAcceptor->c_Accept );
	list_for_each_entry( pAccept, &pAcceptor->c_Accept.ge_Lnk, a_GE.e_Lnk ) {

//SLOG( pAcceptor, "#### GARBAGE pAccept=%p "
//"INET=%s PID=%d-%d Opened=%d Cnt=%d pFd=%p create=%u",
//pAccept, inet_ntoa( pAccept->a_Id.i_Addr.sin_addr), 
//pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, 
//pAccept->a_Opened, pAccept->a_GE.e_Cnt, pAccept->a_pFd,
//(uint32_t)pAccept->a_Create);

		MtxLock( &pAccept->a_Mtx );

		if( pAccept->a_pFd == NULL
			&& pAccept->a_Access + PaxosDecidedTimeout(pPaxos)/1000000LL 
							< time(0) ) {

			SLOG( pAcceptor, LogERR, "#### Abortsend ####");
			_SessionAbortSend( pAcceptor, pAccept );
		}
		MtxUnlock( &pAccept->a_Mtx );
	}
	GElmCtrlUnlock( &pAcceptor->c_Accept );

	PaxosAcceptorTimerSet( pAcceptor, 
				PaxosMasterElectionTimeout(pPaxos)*PaxosGetMax(pPaxos)*2/1000,
				SessionGarbage, (void*)pAcceptor );

	//SLOG( pAcceptor, "### Garbage EXT");

//	DBG_EXT( 0 );
	return( 0 );
}

int
_PaxosAcceptFreeReply( PaxosAccept_t* pAccept )
{
	Msg_t* pMsg;

	while( (pMsg = list_first_entry( &pAccept->a_Rpls.r_Msg, Msg_t, m_Lnk ) ) ) {
		list_del_init( &pMsg->m_Lnk );
		MsgDestroy( pMsg );
	}
	pAccept->a_Rpls.r_Cnt = 0;
	return( 0 );
}

int
PaxosAcceptHold( PaxosAccept_t* pAccept )
{
	MtxLock( &pAccept->a_Mtx );

	_PaxosAcceptFreeReply( pAccept );
	pAccept->a_Rpls.r_Hold	= TRUE;

	MtxUnlock( &pAccept->a_Mtx );
	return( 0 );
}

int
_PaxosAcceptRelease( PaxosAccept_t* pAccept )
{
	PaxosAcceptor_t*	pAcceptor;
	PaxosSessionHead_t*		pRpl;
	int	Ret;
	Msg_t		*pMsg;
	MsgVec_t	*pVec;
	int			i;

	if( pAccept->a_Rpls.r_Hold )	return( -1 );

	pAcceptor	= pAccept->a_pAcceptor;

	if( !pAcceptor->c_Enabled || pAccept->a_pFd == NULL )	return( 0 );

	if( pAccept->a_Accepted.a_Master 
			&& !PaxosIsIamMaster( pAcceptor->c_pPaxos ) ) {
			return( 0 );
	}

	pMsg	= list_first_entry( &pAccept->a_Rpls.r_Msg, Msg_t, m_Lnk );
	pRpl	= (PaxosSessionHead_t*)MsgFirst( pMsg );

	pRpl->h_Id		= pAccept->a_Id;

	pRpl->h_From	= pAcceptor->c_MyId;
	pRpl->h_Master	= PaxosGetMaster( pAcceptor->c_pPaxos );
	pRpl->h_TimeoutUsec	= PaxosMasterElectionTimeout( pAcceptor->c_pPaxos );
	pRpl->h_UnitUsec = PaxosUnitUsec( pAcceptor->c_pPaxos );
	pRpl->h_Reader 	= PaxosGetReader( pAcceptor->c_pPaxos );
	pRpl->h_Ver		= pAcceptor->c_pSessionCell->c_Version;

	pRpl->h_Cksum64	= 0;
	pRpl->h_Len		= 0;
	list_for_each_entry( pMsg, &pAccept->a_Rpls.r_Msg, m_Lnk ) {
		pRpl->h_Len	+= MsgTotalLen( pMsg );
	}

	uint64_t	sum64, cksum64;
	int	res;

	sum64	= 0; res = 0;
	list_for_each_entry( pMsg, &pAccept->a_Rpls.r_Msg, m_Lnk ) {
		cksum64	= MsgCksum64( pMsg, 0, 0, &res );
		sum64 	+= ~cksum64;
		sum64	+= ( ~cksum64 > sum64 );
SLOG( pAcceptor, LogDBG, "MsgTotalLen=%d res=%d", MsgTotalLen(pMsg), res );
	}
	pRpl->h_Cksum64	= ~sum64;

	list_for_each_entry( pMsg, &pAccept->a_Rpls.r_Msg, m_Lnk ) {
		for( i = 0; i < pMsg->m_N; i++ ) {
			pVec	= &pMsg->m_aVec[i];
			if( pVec->v_Flags & VEC_DISABLED )	continue;

			Ret = SendStream( pAccept->a_pFd->e_FdEvent.fd_Fd, 
						pVec->v_pStart, pVec->v_Len );
		}
	}
	return( Ret );
}

int
PaxosAcceptRelease( PaxosAccept_t* pAccept )
{
	int	Ret;

	MtxLock( &pAccept->a_Mtx );

	pAccept->a_Rpls.r_Hold	= FALSE;
	Ret = _PaxosAcceptRelease( pAccept );

	MtxUnlock( &pAccept->a_Mtx );
	return( Ret );
}

int
_PaxosAcceptReplyByMsg( PaxosAccept_t* pAccept, Msg_t* pMsg )
{
	int	Ret = 0;

	/* Dupricate */
	if( !pMsg )	Ret = _PaxosAcceptRelease( pAccept );
	else {

		pAccept->a_Accepted.a_Reply	= pAccept->a_Accepted.a_Start;

		if( !pAccept->a_Rpls.r_Hold ) Ret = _PaxosAcceptFreeReply( pAccept );

		list_init( &pMsg->m_Lnk );
		list_add_tail( &pMsg->m_Lnk, &pAccept->a_Rpls.r_Msg );
		pAccept->a_Rpls.r_Cnt++;

		if( !pAccept->a_Rpls.r_Hold ) Ret = _PaxosAcceptRelease( pAccept );
	}
	return( Ret );
}

int
PaxosAcceptReplyByMsg( PaxosAccept_t* pAccept, Msg_t* pMsg )
{
	int Ret;

	MtxLock( &pAccept->a_Mtx );

	pAccept->a_Accepted.a_Master	= TRUE;

	Ret = _PaxosAcceptReplyByMsg( pAccept, pMsg );

	MtxUnlock( &pAccept->a_Mtx );
	return( Ret );
}

int
PaxosAcceptReplyReadByMsg( PaxosAccept_t* pAccept, Msg_t* pMsg )
{
	int Ret;

	MtxLock( &pAccept->a_Mtx );

	pAccept->a_Accepted.a_Master	= FALSE;

	Ret = _PaxosAcceptReplyByMsg( pAccept, pMsg );

	MtxUnlock( &pAccept->a_Mtx );
	return( Ret );
}

int
_PaxosAcceptSend( PaxosAccept_t* pAccept, PaxosSessionHead_t* pRplHead )
{
	int	Ret = 0;
	PaxosAcceptor_t*	pAcceptor;

	DBG_ENT();

	ASSERT( pAccept );
	pAcceptor	= pAccept->a_pAcceptor;

	pRplHead->h_Id		= pAccept->a_Id;
	pRplHead->h_From	= pAcceptor->c_MyId;
	pRplHead->h_TimeoutUsec	= PaxosMasterElectionTimeout( 
								pAcceptor->c_pPaxos );
	pRplHead->h_UnitUsec = PaxosUnitUsec( pAcceptor->c_pPaxos );
	pRplHead->h_Reader 	= PaxosGetReader( pAcceptor->c_pPaxos );
	pRplHead->h_Ver		= pAcceptor->c_pSessionCell->c_Version;

	if( pAccept->a_pFd ) {

		pRplHead->h_Cksum64	= 0;
		if( pAcceptor->c_bCksum ) {
			pRplHead->h_Cksum64	= in_cksum64(pRplHead, pRplHead->h_Len, 0);
		}
		Ret = SendStream( pAccept->a_pFd->e_FdEvent.fd_Fd, 
						(char*)pRplHead, pRplHead->h_Len );
	}
	RETURN( Ret );
}

int
PaxosAcceptSend( PaxosAccept_t* pAccept, PaxosSessionHead_t* pRplHead )
{
	int Ret;

	MtxLock( &pAccept->a_Mtx );

	Ret = _PaxosAcceptSend( pAccept, pRplHead );

	MtxUnlock( &pAccept->a_Mtx );
	return( Ret );
}

int
PaxosAcceptSendByMsg( PaxosAccept_t* pAccept, Msg_t *pM )
{
	PaxosAcceptor_t		*pAcceptor;
	PaxosSessionHead_t	*pRplHead;
	MsgVec_t	*pVec;
	int	Ret;
	int	i;

	MtxLock( &pAccept->a_Mtx );

	pRplHead	= (PaxosSessionHead_t*)MsgFirst( pM );

	pAcceptor	= pAccept->a_pAcceptor;

	pRplHead->h_Id		= pAccept->a_Id;
	pRplHead->h_From	= pAcceptor->c_MyId;
	pRplHead->h_TimeoutUsec	= PaxosMasterElectionTimeout( 
								pAcceptor->c_pPaxos );
	pRplHead->h_UnitUsec = PaxosUnitUsec( pAcceptor->c_pPaxos );
	pRplHead->h_Reader 	= PaxosGetReader( pAcceptor->c_pPaxos );
	pRplHead->h_Ver		= pAcceptor->c_pSessionCell->c_Version;
	pRplHead->h_Cksum64	= 0;
	pRplHead->h_Cksum64	= MsgCksum64( pM, 0, 0, NULL );

	if( pAccept->a_pFd ) {
		for( i = 0; i < pM->m_N; i++ ) {
			pVec	= &pM->m_aVec[i];
			if( pVec->v_Flags & VEC_DISABLED )	continue;

			Ret = SendStream( pAccept->a_pFd->e_FdEvent.fd_Fd, 
					pVec->v_pStart, pVec->v_Len );
			if( Ret < 0 )	goto err;
		}
	}
	MtxUnlock( &pAccept->a_Mtx );

	MsgDestroy( pM );

	return( 0 );
err:
	MtxUnlock( &pAccept->a_Mtx );
	MsgDestroy( pM );
	return( -1 );
}

int
PaxosAcceptSendDataByMsg( PaxosAccept_t *pAccept, Msg_t *pM )
{
	PaxosSessionHead_t	Head;
	PaxosAcceptor_t*	pAcceptor;
	MsgVec_t	*pVec;
	int	Ret;
	int	i;
	MsgVec_t	Vec;

	pAcceptor	= pAccept->a_pAcceptor;

	SESSION_MSGINIT( &Head, PAXOS_SESSION_DATA, 0, 0, 
						sizeof(Head) + MsgTotalLen(pM) );

	Head.h_Id		= pAccept->a_Id;
	Head.h_From		= pAcceptor->c_MyId;
	Head.h_TimeoutUsec	= PaxosMasterElectionTimeout( 
								pAcceptor->c_pPaxos );
	Head.h_UnitUsec = PaxosUnitUsec( pAcceptor->c_pPaxos );
	Head.h_Reader 	= PaxosGetReader( pAcceptor->c_pPaxos );
	Head.h_Master = PaxosGetMaster( pAcceptor->c_pPaxos );
	Head.h_Epoch = PaxosEpoch( pAcceptor->c_pPaxos );

	Head.h_Cksum64	= 0;

	MsgVecSet( &Vec, 0, &Head, sizeof(Head), sizeof(Head) );
	MsgInsert( pM, &Vec );

	Head.h_Cksum64	= MsgCksum64( pM, 0, 0, NULL );

	MtxLock( &pAccept->a_Mtx );

	for( i = 0; i < pM->m_N; i++ ) {
		pVec	= &pM->m_aVec[i];
		if( pVec->v_Flags & VEC_DISABLED )	continue;

		Ret = SendStream( pAccept->a_pFd->e_FdEvent.fd_Fd, 
					pVec->v_pStart, pVec->v_Len );
		if( Ret < 0 )	goto err;
	}
	MsgDestroy( pM );

	MtxUnlock( &pAccept->a_Mtx );

	return( 0 );
err:
	MsgDestroy( pM );
	MtxUnlock( &pAccept->a_Mtx );
	return( -1 );
}

int
ElectEventSend( PaxosAcceptor_t* pAcceptor, PaxosSessionHead_t* pEvent )
{
	PaxosAccept_t*	pAccept;

	DBG_ENT();

again:
	SLOG( pAcceptor, LogERR, "### ELECTION h_Code=%d h_Master=%d ###", 
					pEvent->h_Code, pEvent->h_Master );

	if( MtxTrylock( &pAcceptor->c_Mtx ) ) {
		SLOG( pAcceptor, LogERR, " MtxTryLock(c_Mtx) ERROR" );
		RETURN( 0 );
	}

	list_for_each_entry( pAccept, &pAcceptor->c_Accept.ge_Lnk, a_GE.e_Lnk ) {

		if( MtxTrylock( &pAccept->a_Mtx ) )	 {
			MtxUnlock( &pAcceptor->c_Mtx );
			usleep( 20*1000 );
			goto again;
		}

		if( pAccept->a_pFd ) {

			SLOG( pAcceptor, LogINF,
				"->pAccept=%p INET[%s] PID[%d-%d]", pAccept, 
				inet_ntoa(pAccept->a_Id.i_Addr.sin_addr), 
				pAccept->a_Id.i_Pid, pAccept->a_Id.i_No );

			_PaxosAcceptSend( pAccept, pEvent );
		}
		MtxUnlock( &pAccept->a_Mtx );
	}
	MtxUnlock( &pAcceptor->c_Mtx );
	RETURN( 0 );
}

int
PaxosAcceptOpenReplyByMsg( PaxosAccept_t* pAccept, PaxosSessionHead_t* pH, 
											int Error )
{
	PaxosAcceptor_t*	pAcceptor;
	PaxosSessionHead_t *pRpl;
	int	Ret;

	pAcceptor	= pAccept->a_pAcceptor;
	pRpl = SESSION_MSGALLOC( PAXOS_SESSION_OPEN, 0, Error, sizeof(*pRpl) );	

	Ret	= ACCEPT_REPLY_MSG( pAccept, TRUE, pRpl, sizeof(*pRpl), pRpl->h_Len );
	return( Ret );
}

TimerEvent_t*
PaxosAcceptorTimerSet( PaxosAcceptor_t* pAcceptor, int64_t Ms, 
					int(*fEvent)(TimerEvent_t*), void* pArg )
{
	TimerEvent_t*	pE;

	pE = TimerSet( &pAcceptor->c_Timer, (int)Ms, fEvent, pArg );

	return( pE );
}

void *
PaxosAcceptorTimerCancel( PaxosAcceptor_t *pAcceptor, TimerEvent_t *pE )
{
	void *pArg;

	pArg = TimerCancel( &pAcceptor->c_Timer, pE );

	return( pArg );
}

int
PaxosAcceptReplyCloseByMsg( PaxosAccept_t* pAccept, PaxosSessionHead_t* pH, 
									int Error )
{
	PaxosSessionHead_t *pRpl;
	int	Ret;
	PaxosAcceptor_t*	pAcceptor;


	ASSERT( pAccept );

	pAcceptor	= pAccept->a_pAcceptor;

	pRpl = SESSION_MSGALLOC( PAXOS_SESSION_CLOSE, 0, Error, sizeof(*pRpl) );	

	Ret	= ACCEPT_REPLY_MSG( pAccept, TRUE, pRpl, sizeof(*pRpl), pRpl->h_Len );

	return( Ret );
}

/*
 *	Critical section
 */
int
Election( struct Paxos* pPaxos, PaxosNotify_t Notify, 
						int server, uint32_t Epoch, int32_t From )
{
	PaxosSessionHead_t	Event;
	PaxosAcceptor_t*	pAcceptor;
	bool_t				bSend = TRUE;

	pAcceptor	= (PaxosAcceptor_t*)PaxosGetTag( pPaxos );

/* Edge Event */
	switch( Notify ) {
	case PAXOS_Im_NOT_MASTER:
		SLOG( pAcceptor, LogERR,
			"=== %d:I'm not Master (MasterWas=%d server=%d) by %d ===", 
			server, pAcceptor->c_MasterWas, server, From );

		bSend = TRUE;
		pAcceptor->c_MasterWas	= server;

		break;

	case PAXOS_Im_MASTER:
		SLOG( pAcceptor, LogSYS,
			"=== I'm Master by %d (MasterWas=%d) ===", 
			From, pAcceptor->c_MasterWas );

		bSend = TRUE;
		pAcceptor->c_MasterWas	= server;

		PaxosAccept_t	*pAccept;

		list_for_each_entry(pAccept, &pAcceptor->c_Accept.ge_Lnk, a_GE.e_Lnk) {
			pAccept->a_Access	= time(0);
SLOG( pAcceptor, LogDBG, "#### ImMaster pAccept=%p "
"INET=%s PID=%d-%d Opened=%d Cnt=%d pFd=%p Create=%u Access=%u",
pAccept, inet_ntoa( pAccept->a_Id.i_Addr.sin_addr), 
pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, 
pAccept->a_Opened, pAccept->a_GE.e_Cnt, pAccept->a_pFd, 
(uint32_t)pAccept->a_Create,
(uint32_t)pAccept->a_Access );
		}

		PaxosAcceptorTimerSet( pAcceptor, 
				PaxosMasterElectionTimeout(pPaxos)*PaxosGetMax(pPaxos)*2/1000,
				SessionGarbage, (void*)pAcceptor );
		SLOG( pAcceptor, LogSYS, "### Garbage TIMEOUT=%d usec",
				PaxosMasterElectionTimeout(pPaxos)*PaxosGetMax(pPaxos)*2 );
		break;

	case PAXOS_MASTER_ELECTED:
		SLOG( pAcceptor, LogSYS, "=== %d is Elected by %d (MasterWas=%d) ===", 
							server, From, pAcceptor->c_MasterWas );

		bSend = ( server != -1 && pAcceptor->c_MasterWas != server );
		pAcceptor->c_MasterWas	= server;

		break;

	case PAXOS_MASTER_CANCELED:
		SLOG( pAcceptor, LogSYS, "=== %d is Canceled by %d (MasterWas=%d) ===",
							server, From, pAcceptor->c_MasterWas );

		bSend = FALSE;
		pAcceptor->c_MasterWas	= -1;

		break;

	default:
		SLOG( pAcceptor, LogERR, "=== DEFAULT %d ?! %d by %d ===", 
							Notify, server, From );
		bSend	= FALSE;
		break;
	}
	if( bSend ) {
		SESSION_MSGINIT( &Event, PAXOS_SESSION_ELECTION, 
								Notify, 0, sizeof(Event) );
		Event.h_Master	= server;
		Event.h_Epoch	= Epoch;

		SLOG( pAcceptor, LogINF, 
			"### ELECTION h_Code=%d h_Master=%d by %d ###", 
			Event.h_Code, Event.h_Master, From );

		ElectEventSend( pAcceptor, &Event );
	}

	return( 0 );
}

#define	OUTOFBAND_ALLOC( pAcceptor, pId ) \
	({AcceptorOutOfBand_t *_p; \
		_p = (pAcceptor->c_IF.if_fMalloc)(sizeof(AcceptorOutOfBand_t) ); \
		if(_p) { \
			memset( _p, 0, sizeof(*_p) ); \
			_p->o_Id = *pId; \
			list_init( &_p->o_Lnk ); \
			_p->o_Create = time(0); \
		} _p;})

static int
AcceptorHashOutOfBandCode( void* pKey )
{
	PaxosClientId_t* pId = (PaxosClientId_t*)pKey;
	uint32_t	Ret;

	DBG_ENT();

	Ret	= (uint32_t)(pId->i_Addr.sin_addr.s_addr) 
			+ pId->i_Pid + pId->i_No + pId->i_Seq + pId->i_Try;

	RETURN( (int)Ret );
}

static int
AcceptorHashOutOfBandCompare( void* pKeyA, void* pKeyB )
{
	PaxosClientId_t* pIdA = (PaxosClientId_t*)pKeyA;
	PaxosClientId_t* pIdB = (PaxosClientId_t*)pKeyB;

	DBG_ENT();

	if( pIdA->i_Addr.sin_addr.s_addr == pIdB->i_Addr.sin_addr.s_addr
		&& pIdA->i_Pid == pIdB->i_Pid
		&& pIdA->i_No == pIdB->i_No
		&& pIdA->i_Time == pIdB->i_Time
		&& pIdA->i_Reuse == pIdB->i_Reuse
		&& pIdA->i_Seq == pIdB->i_Seq
		&& pIdA->i_Try == pIdB->i_Try )
			RETURN( 0 );
	else	RETURN( -1 );
}

int
_AcceptorOutOfBandCache( PaxosAcceptor_t* pAcceptor, AcceptorOutOfBand_t* pOut )
{
	AcceptorOutOfBand_t*	pLast;
	int	Ret;
	int	len;

	len = pOut->o_pData->h_Len;
	list_for_each_entry( pLast,&pAcceptor->c_OutOfBand.o_Lnk,o_Lnk ) {

		if( pAcceptor->c_OutOfBand.o_MemUsed + len
								< pAcceptor->c_OutOfBand.o_MemLimit ) break;
		if ( pLast == pOut ) continue;
		if ( !pLast->o_pData) continue;
		if( !pLast->o_bSaved ) {
			Ret = (pAcceptor->c_IF.if_fOutOfBandSave)(pAcceptor,pLast->o_pData);
			ASSERT( !Ret );
			pLast->o_bSaved	= TRUE;
		}

		pAcceptor->c_OutOfBand.o_MemUsed	-= pLast->o_pData->h_Len;

		(pAcceptor->c_IF.if_fFree)( pLast->o_pData );
		pLast->o_pData	= NULL;
	}
	pAcceptor->c_OutOfBand.o_MemUsed	+= pOut->o_pData->h_Len;

	ASSERT(pAcceptor->c_OutOfBand.o_MemUsed 
							< pAcceptor->c_OutOfBand.o_MemLimit);

	return( 0 );
}

PaxosSessionHead_t*
_PaxosAcceptorOutOfBandGet( PaxosAcceptor_t* pAcceptor, PaxosClientId_t* pId )
{
	AcceptorOutOfBand_t*	pOut;
	PaxosSessionHead_t	*pH;

	pOut = (AcceptorOutOfBand_t*)HashGet( &pAcceptor->c_OutOfBand.o_Hash, pId );

	if( pOut ) {

		if( pOut->o_pData )	return( pOut->o_pData );

		pH = (pAcceptor->c_IF.if_fOutOfBandLoad)( pAcceptor, pId );
		if( !pH )	goto err;

		if( pH->h_Cksum64 ) {
			assert( !in_cksum64( pH, pH->h_Len, 0 ) );
		}
		pOut->o_pData	= pH;

		_AcceptorOutOfBandCache( pAcceptor, pOut );
	} else {
		pH = (pAcceptor->c_IF.if_fOutOfBandLoad)( pAcceptor, pId );
		if( !pH )	goto err;

		pOut = OUTOFBAND_ALLOC( pAcceptor, pId );
		HashPut( &pAcceptor->c_OutOfBand.o_Hash, &pOut->o_Id, pOut );
		list_add_tail( &pOut->o_Lnk, &pAcceptor->c_OutOfBand.o_Lnk );
		pAcceptor->c_OutOfBand.o_Cnt++;

		pOut->o_pData	= pH;

		_AcceptorOutOfBandCache( pAcceptor, pOut );
	}
	return( pH );
err:
	return( NULL );
}

PaxosSessionHead_t*
_PaxosAcceptOutOfBandGet( PaxosAccept_t* pAccept, PaxosClientId_t* pId )
{
	return( _PaxosAcceptorOutOfBandGet( pAccept->a_pAcceptor, pId ) );
}

int
PaxosAcceptOutOfBandLock( PaxosAccept_t* pAccept )
{
	MtxLock( &pAccept->a_pAcceptor->c_OutOfBand.o_Mtx );
	return( 0 );
}

int
PaxosAcceptOutOfBandUnlock( PaxosAccept_t* pAccept )
{
	MtxUnlock( &pAccept->a_pAcceptor->c_OutOfBand.o_Mtx );
	return( 0 );
}

int
_OutOfBandRemove( PaxosAcceptor_t *pAcceptor, AcceptorOutOfBand_t *pOut )
{

	if( pOut->o_bSaved ) {
		(pAcceptor->c_IF.if_fOutOfBandRemove)( pAcceptor, &pOut->o_Id );
	}

	list_del_init( &pOut->o_Lnk );
	HashRemove( &pAcceptor->c_OutOfBand.o_Hash, &pOut->o_Id );
	pAcceptor->c_OutOfBand.o_Cnt--;

	if( pOut->o_pData ) {
		pAcceptor->c_OutOfBand.o_MemUsed	-= pOut->o_pData->h_Len;
		(pAcceptor->c_IF.if_fFree)( pOut->o_pData );
	}
	pOut->o_pData	= NULL;
	(pAcceptor->c_IF.if_fFree)( pOut );

	return( 0 );
}

AcceptorOutOfBand_t*
_PaxosAcceptorOutOfBandPush( PaxosAcceptor_t* pAcceptor, PaxosSessionHead_t* pM )
{
	AcceptorOutOfBand_t *pOut;
	/*
	 *	Checksum
	 */
	if( pM->h_Cksum64 ) {
		assert( !in_cksum64( pM, pM->h_Len, 0 ) );
	}
#ifdef KKK
#else
	if( pM->h_Len >= pAcceptor->c_OutOfBand.o_MemLimit ) {
		int len = sizeof(*pM);
SLOG( pAcceptor, LogDBG, "large OOB DATA(%d)>=%d", 
pM->h_Len,pAcceptor->c_OutOfBand.o_MemLimit);
		PaxosSessionHead_t *pMsg
			= (PaxosSessionHead_t*)(pAcceptor->c_IF.if_fMalloc)( len );
		memcpy(	pMsg, pM, len);
		pMsg->h_Len = len;
		pMsg->h_Cksum64	= 0;
		if( pAcceptor->c_bCksum ) {
			pMsg->h_Cksum64	= in_cksum64( pMsg, len, 0 );
		}
		(pAcceptor->c_IF.if_fFree)( pM );
		pM = pMsg;
	}
#endif
	/*
	 *	Cache
	 */
	pOut = (AcceptorOutOfBand_t*)HashGet( &pAcceptor->c_OutOfBand.o_Hash, 
												&pM->h_Id );

	if( !pOut ) {	// New One
		pOut = OUTOFBAND_ALLOC( pAcceptor, &pM->h_Id );
		HashPut( &pAcceptor->c_OutOfBand.o_Hash, &pOut->o_Id, pOut );
		list_add_tail( &pOut->o_Lnk, &pAcceptor->c_OutOfBand.o_Lnk );
		pAcceptor->c_OutOfBand.o_Cnt++;

	} else { 		// Replace

		if( pOut->o_bSaved ) {
			(pAcceptor->c_IF.if_fOutOfBandRemove)( pAcceptor, &pOut->o_Id );
			pOut->o_bSaved	= FALSE;
		}
	}

	pOut->o_pData	= pM;
	pOut->o_Last	= time(0) + (PaxosDecidedTimeout( pAcceptor->c_pPaxos )
				     + PaxosMasterElectionTimeout( pAcceptor->c_pPaxos ))/1000000LL;

	_AcceptorOutOfBandCache( pAcceptor, pOut );

	return( pOut );
}

int
PaxosAcceptorOutOfBandPush( PaxosAcceptor_t* pAcceptor, PaxosSessionHead_t* pM )
{
	AcceptorOutOfBand_t	*pOut;

	MtxLock( &pAcceptor->c_OutOfBand.o_Mtx );
	pOut	= _PaxosAcceptorOutOfBandPush( pAcceptor, pM );
	MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );
	if( pOut )	return( 0 );
	else		return( -1 );
}

int
PaxosAcceptorOutOfBandPull( PaxosAcceptor_t* pAcceptor, 
								PaxosSessionHead_t* pM, int fd )
{
	PaxosSessionHead_t*	pH;

	MtxLock( &pAcceptor->c_OutOfBand.o_Mtx );

	pH	= _PaxosAcceptorOutOfBandGet( pAcceptor, &pM->h_Id );

	if( pH == NULL ) {
		SLOG( pAcceptor, LogERR, "Can't get OutOfBand:Pid=%d-%d Seq=%d",
				pM->h_Id.i_Pid, pM->h_Id.i_No, pM->h_Id.i_Seq );
		close( fd );
		goto err1;
	}
	if( pAcceptor->c_bCksum ) {
		int	r;

		r	= offsetof( PaxosSessionHead_t, h_From ) % 8;

		pH->h_Cksum64	= in_cksum64_update( pH->h_Cksum64,
						&pH->h_From, sizeof(pH->h_From), r,
						&pAcceptor->c_MyId, sizeof(pAcceptor->c_MyId), r ); 
	}
	pH->h_From	= pAcceptor->c_MyId;

	if( SendStream( fd, (char*)pH, pH->h_Len ) ) {
		goto err1;
	}

	MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );
	return( 0 );
err1:
	MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );
SLOG( pAcceptor, LogERR, "SEND ERROR errno=%d", errno );
	return( -1 );
}

int
CONNECT_TO( PaxosAcceptor_t *pAcceptor, int j )
{
	int	fd;
	struct linger	Linger;
	struct sockaddr_in	Peer;
	

	if( (fd = pAcceptor->c_pFds[j] ) >= 0 ) {
		close( fd );
SLOG( pAcceptor, LogDBG, "CLOSE fd=%d", fd );
	}
	Peer	= pAcceptor->c_pSessionCell->c_aPeerAddr[j].a_Addr;
	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(connect(fd, (struct sockaddr*)&Peer, sizeof(Peer)) < 0 ) {

		SLOG( pAcceptor, LogERR,
		"CONNECT ERROR fd=%d errno=%dj=%d peerPort=%d\n", 
		fd, errno, j,ntohs(Peer.sin_port) );

		close( fd );
		fd = -1; 
	 } else {
		memset( &Linger, 0, sizeof(Linger) );
		Linger.l_onoff	= 1;
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(Linger));
#ifdef	ZZZ
#else
		int	flag = 1;
		setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
#endif
	}
	pAcceptor->c_pFds[j] = fd;
	return( fd );
}

int
PaxosAcceptorOutOfBandValidate( PaxosAcceptor_t* pAcceptor, uint32_t seq, 
										PaxosSessionHead_t* pH, int j )
{
	PaxosSessionHead_t	Req;
	PaxosSessionHead_t	*pM;
	int	fd;
	int	Ret = 0;
	AcceptorOutOfBand_t	*pOut;

	MtxLock( &pAcceptor->c_OutOfBand.o_Mtx );

	pOut = (AcceptorOutOfBand_t*)HashGet( &pAcceptor->c_OutOfBand.o_Hash, 
												&pH->h_Id );
	if( !pOut ) {

	/*
	 * $B$3$3$KMh$k$N$O:FMW5a$NH&$G!"(BpOut$B$,$J$$$N$O40N;$7$F$$$k(B
	 * $B$=$3$G!"(BPaxos$B9g0U$rDL$9$?$a(BOK$B$H$7$F=EJ#MW5a$O>e0L$G%A%'%C%/$5$l$k(B
	 */
	if( j == pAcceptor->c_MyId )  {
		SLOG( pAcceptor, LogINF,
			"j(%d) == pAcceptor->c_MyId Try=%d", j, pH->h_Id.i_Try );
		MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );
		return( 0 );
	}
		MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );

		SESSION_MSGINIT( &Req, PAXOS_SESSION_OB_PULL, 0, 0, sizeof(Req) );	
		Req.h_Id	= pH->h_Id;

		if( (fd = pAcceptor->c_pFds[j]) < 0 ) {
			if( (fd = CONNECT_TO( pAcceptor, j )) < 0 ) { 
				Ret = -1; 
				goto ret;
			}
		}
		Req.h_Cksum64	= 0;
		if( pAcceptor->c_bCksum ) {
			Req.h_Cksum64	= in_cksum64( &Req, sizeof(Req), 0 );
		}
		if( SendStream( fd, (char*)&Req, sizeof(Req) ) < 0 ) {
			if( (fd = CONNECT_TO( pAcceptor, j ) ) < 0 ) { 
				Ret = -1; 
				goto ret;
			}
			if( SendStream( fd, (char*)&Req, sizeof(Req) ) < 0 ) {
SLOG( pAcceptor, LogERR, "SEND ERROR fd=%d j=%d errno=%d", fd, j, errno );
				close( fd );
				pAcceptor->c_pFds[j] = -1;
				Ret = -1; 
				goto ret;
			}
		}
		pM	= (PaxosSessionHead_t*)PaxosAcceptorRecvMsg( pAcceptor, fd );
		if( !pM ) {
SLOG( pAcceptor, LogERR, "RECV ERROR fd=%d j=%d errno=%d", fd, j, errno );
			close( fd );
			pAcceptor->c_pFds[j] = -1;
			 Ret = -1; 
			goto ret;
		}
		MtxLock( &pAcceptor->c_OutOfBand.o_Mtx );
		pOut = _PaxosAcceptorOutOfBandPush( pAcceptor, pM );
	}
	/* Seq$B=g$KJB$SJQ$($k(B */
	if( !pOut->o_bValidate || (pOut->o_bValidate && pOut->o_Seq != seq ) ) {

		AcceptorOutOfBand_t	*pPrev;
		int	cnt;

		list_del_init( &pOut->o_Lnk );
		pOut->o_bValidate	= TRUE;
		pOut->o_Seq			= seq;

		cnt = 0;
		list_for_each_entry_reverse( pPrev, 
						&pAcceptor->c_OutOfBand.o_Lnk, o_Lnk ) {

			if( !pPrev->o_bValidate )	continue;
			if( cnt++ > PAXOS_MAX )		break;

			if( CMP( pPrev->o_Seq, pOut->o_Seq ) < 0 )	goto insert;
		}
//		if( cnt )	ABORT();
		if( cnt > PAXOS_MAX )	ABORT();
insert:
		if( pPrev )	list_add( &pOut->o_Lnk, &pPrev->o_Lnk );
		else		list_add_tail(&pOut->o_Lnk, &pAcceptor->c_OutOfBand.o_Lnk);
	} 

DBG_TRC("INSERT seq=%u\n", seq );

	MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );
ret:
	RETURN( Ret );
}

int
_AcceptorOutOfBandMarshal(struct Paxos *pPaxos, void* pVoid, IOReadWrite_t *pW)
{
	PaxosAcceptor_t* pAcceptor;
	PaxosSessionHead_t	*pH = (PaxosSessionHead_t*)pVoid;
	int32_t	Len;

	pAcceptor = (PaxosAcceptor_t*)PaxosGetTag( pPaxos );
	if( (pH = _PaxosAcceptorOutOfBandGet( pAcceptor, &pH->h_Id ) ) ) {
		Len	= pH->h_Len;
		(pW->IO_Write)( pW, &Len, sizeof(Len) );
		(pW->IO_Write)( pW, pH, pH->h_Len );
	} else {
		ABORT();
	}
	return( 0 );
}

int
_AcceptorOutOfBandUnmarshal( struct Paxos *pPaxos, void* pVoid, 
								IOReadWrite_t *pR )
{
	PaxosAcceptor_t* pAcceptor;
	PaxosSessionHead_t	*pM;
	int32_t	Len;

	pAcceptor = (PaxosAcceptor_t*)PaxosGetTag( pPaxos );
	(pR->IO_Read)( pR, &Len, sizeof(Len) );
	pM	= (PaxosSessionHead_t*)(pAcceptor->c_IF.if_fMalloc)( Len );
	(pR->IO_Read)( pR, pM, Len );

	_PaxosAcceptorOutOfBandPush( pAcceptor, pM );
	return( 0 );
}

int
Notify( struct Paxos* pPaxos, PaxosMode_t Mode, PaxosNotify_t Notify, 
	uint32_t seq, int server, uint32_t epoch, int32_t len, void *pC, int from )
{
	PaxosSessionHead_t*	pPaxosHead = (PaxosSessionHead_t*)pC;
	int		Ret = 0;
	PaxosAcceptor_t* pAcceptor;

	DBG_ENT();

	pAcceptor = (PaxosAcceptor_t*)PaxosGetTag( pPaxos );
ASSERT( pAcceptor );

	switch( Notify ) {
	case PAXOS_NOOP:
		SLOG( pAcceptor, LogERR, "=== %d:NOOP[%d] ===", server, seq );
		break;

	case PAXOS_REJECTED:
		SLOG( pAcceptor, LogERR, 
			"=== Rejected === Cmd=%d Seq=%d", pPaxosHead->h_Cmd, seq );
		Ret = (pAcceptor->c_IF.if_fRejected)( pAcceptor, pPaxosHead );
		break;

	case PAXOS_VALIDATE:
		DBG_TRC("=== VALIDATE(seq=%u) ===\n",  seq );
		Ret = (pAcceptor->c_IF.if_fValidate)(pAcceptor, seq, pPaxosHead, from);
		break;

	case PAXOS_NOT_GET_PAGE:
		SLOG( pAcceptor, LogSYS, "=== NOT_GET_PAGE(seq=%u) ===",  seq );
		Ret = (pAcceptor->c_IF.if_fAbort)(pAcceptor, seq );
	default:
		ABORT();
		break;

	}
	RETURN( Ret );
}

void*
ThreadExecute( void* pArg )
{
	struct Paxos*		pPaxos = (struct Paxos*)pArg;
	PaxosExecute_t*		pH;
	PaxosSessionHead_t*	pPSH;
	PaxosAcceptor_t* 	pAcceptor;
	PaxosAccept_t*		pAccept;
	int	Ret;
	uint32_t	seq, backSeq;
	bool_t		Validate;
	bool_t	bFree;
	Msg_t	*pMsg;
	PaxosClientId_t	Id;
	timespec_t	Now;

	pAcceptor = (PaxosAcceptor_t*)PaxosGetTag( pPaxos );
ASSERT( pAcceptor );

	while( (pMsg = PaxosRecvFromPaxos( pPaxos ) ) ) {

		bFree	= TRUE;
		pH	= (PaxosExecute_t*)MsgFirst( pMsg );
		seq				= pH->e_Seq;
		Validate		= pH->e_Validate;
		backSeq			= pH->e_BackSeq;
SLOG(pAcceptor,LogDBG,"seq=%u backSeq=%u Validate=%d", seq, backSeq, Validate );

		switch( (int)pH->e_Notify ) {

		case PAXOS_ACCEPTED:
			pPSH	= (PaxosSessionHead_t*)MsgNext( pMsg );
			Id	= pPSH->h_Id;

			/*
			 *	OPEN$B$OFCJL(B
			 *	Replica$B$N;~!"@\B3$,$J$$>l9g$,$"$k$N$G(BpAccept$B$r:n$k(B
			 */
			if( pPSH->h_Cmd == PAXOS_SESSION_OPEN ) {
				pAccept	= PaxosAcceptGet( pAcceptor, &pPSH->h_Id, TRUE );
			} else {
				pAccept	= PaxosAcceptGet( pAcceptor, &pPSH->h_Id, FALSE );
			}
if( pAccept )
SLOG(pAcceptor,LogDBG,"Count=%d,Cmd=0x%x",pAccept->a_GE.e_Cnt,pPSH->h_Cmd);
			if( pAccept && 
				pPSH->h_Cmd != PAXOS_SESSION_ABORT ) {
//SLOG(pAcceptor,LogINF,"Count=%d,Cmd=0x%x",pAccept->a_GE.e_Cnt,pPSH->h_Cmd);

			/* PAXOS_SESSION_ABORT$B$OFbIt$N%,!<%Y%8%3%^%s%I$G1~Ez$,$$$i$J$$(B */

			/* request$B$G$O40A4$KGS=|$G$-$J$$(B */
				if( PaxosAcceptIsAccepted( pAccept, pPSH ) ) {

SLOG( pAcceptor, LogINF,
"### DUPRICATE INSTANCE pAccept=%p seq=%u",pAccept, seq );
					PaxosAcceptPut( pAccept );
					goto next;
				}
				/* $BDL?.2sO)>c32$G%5!<%P$,(Bgarbage$B$7$?$,!"(B
				   $B%/%i%$%"%s%H$OCN$i$:$K(Bbind$B$G%;%C%7%g%s9=B$BN$,:n$i$l$?!#(B
				   $B%5!<%P$N>c328!CN;~4V$,Bg$J$N$GLGB?$K5/$i$J$$!#(B
				*/
				if( CMP( Id.i_SeqWait, pAccept->a_Accepted.a_End ) > 0 ) {

					PaxosSessionHead_t	Rpl;

					SLOG( pAcceptor, LogINF,
					"i_SeqWait(%d) > a_End(%d) pAccept=%p a_Opened=%d a_Cmd=%d",
						Id.i_SeqWait, pAccept->a_Accepted.a_End,
						pAccept, pAccept->a_Opened, 
						pAccept->a_Accepted.a_Cmd );

					SESSION_MSGINIT( &Rpl, pPSH->h_Cmd, 0,
									ERR_SESSION_NOT_FOUND, sizeof(Rpl) );
					Rpl.h_Master = PaxosGetMaster( pAcceptor->c_pPaxos );
					Rpl.h_Epoch = PaxosEpoch( pAcceptor->c_pPaxos );
					Ret = PaxosAcceptSend( pAccept, &Rpl );

					PaxosAcceptPut( pAccept );
					goto next;
				}
				PaxosAcceptedStart( pAccept, pPSH );	// ????
			}

			switch( pPSH->h_Cmd ) {

			case PAXOS_SESSION_OPEN:

				ASSERT( pAccept );

				Ret = (pAcceptor->c_IF.if_fSessionOpen)( pAccept, pPSH );
				if( !Ret )	pAccept->a_Opened	= ACCEPT_OPENED;
				PaxosAcceptOpenReplyByMsg( pAccept, pPSH, Ret );
				SLOG( pAcceptor, LogDBG, "PAXOS_SESSION_OPEN: Ret=%d, seq=%d", Ret, seq );
ASSERT( Ret == 0 );
				break;

			case PAXOS_SESSION_CLOSE:
			case PAXOS_SESSION_ABORT:
			/*
			 *	Master$B$,J#?t$N;~!"(BCLOSE$B$,J#?t2sH/9T$5$l(B,
			 * pAccept$B$,(BNULL$B$N>l9g$,$"$k(B
			 */
				if( pAccept ) {
					if( pAccept->a_Opened ) {
						Ret = (pAcceptor->c_IF.if_fSessionClose)( pAccept, 
															pPSH );
if( Ret ) SLOG( pAcceptor, LogERR,
"### CLOSE:Ret=%d SessionClose seq=%u Pid=%d-%d Seq=%d####",
Ret, seq, pPSH->h_Id.i_Pid, pPSH->h_Id.i_No, pPSH->h_Id.i_Seq );
					}
					else {
SLOG( pAcceptor, LogERR,
"### CLOSE:NOT OPENED seq=%u Pid=%d-%d Seq=%u Cnt=%d ####",
	seq, pPSH->h_Id.i_Pid, pPSH->h_Id.i_No, pPSH->h_Id.i_Seq, 
	pAccept->a_GE.e_Cnt );
						Ret = -1;
					}
				/* OutOfBand garbage */
					AcceptorOutOfBand_t	*pOut, *pW;

					MtxLock( &pAcceptor->c_OutOfBand.o_Mtx );
					list_for_each_entry_safe( pOut, pW, 
								&pAcceptor->c_OutOfBand.o_Lnk, o_Lnk ) {

						if( pOut->o_bValidate ) 	continue;

						if( !HashSessionCompare(&pAccept->a_Id, &pOut->o_Id) ) {
SLOG( pAcceptor, LogERR, "REMOVE Pid=%d-%d Reuse=%d Try=%d Seq=%d", 
pOut->o_Id.i_Pid, pOut->o_Id.i_No, pOut->o_Id.i_Reuse, 
pOut->o_Id.i_Try, pOut->o_Id.i_Seq );
							_OutOfBandRemove( pAcceptor, pOut );
						}
					}
					MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );

					if( pPSH->h_Cmd == PAXOS_SESSION_CLOSE ) {
						PaxosAcceptReplyCloseByMsg( pAccept, pPSH, Ret );
					}
					pAccept->a_Opened	= 0;
SLOG( pAcceptor, LogDBG, "### AcceptPut:Cnt=%d ####", pAccept->a_GE.e_Cnt );
					PaxosAcceptPut( pAccept );
					pAccept = NULL;

				} else {
				/*
				 * pAccept$B$,$J$$$N$O4{$K(BCLOSE$B$,<uM}$5$l=hM}$5$l$?(B
				 * $B$3$3$KE~C#$9$k$N$O:FMW5a$G$"$k(B
				 * $BK\:FMW5a$O!"?7(BMaster$B$,:G=i$NMW5a$r<u$1$D$F$$$J$$$N$GEjF~$7$?(B
				 * $B$7$+$k$KK\%l%W%j%+$O=hM}:Q$_$G;q8;$r2rJ|$7$?$N$GL5;k$5$l$k(B
				 * $B1~Ez$O?7(BMaster$B$,JV$9$3$H$K$J$k(B
				 * $B$5$i$K!"(Bclient$B$,:FMW5a$r$7!"?7?7(BMaster$B$G;q8;$,$J$$>l9g$O!"(B
				 * $B4{$K!"MW5a<uIU%9%l%C%I$G(BClose$B$5$l$?$H$9$k(B
				 * Master$B$G$O@\B3;~$K(BpAccept$B$r:n$k$N$G!"1~Ez$O2DG=$G$"$k(B
				 */ 
					SLOG( pAcceptor, LogERR,
					"### CLOSE:NO ACCEPT seq=%u Pid=%d-%d Seq=%u Try=%d ####",
					seq, pPSH->h_Id.i_Pid, pPSH->h_Id.i_No, 
					pPSH->h_Id.i_Seq, pPSH->h_Id.i_Try );
					Ret = 0;
				}
				break;

			case PAXOS_SESSION_EVENT:
				if( pAccept )	PaxosAcceptEventGet( pAccept, pPSH );
SLOG( pAcceptor, LogDBG, "Count=%d",pAccept->a_GE.e_Cnt );
				break;

			case PAXOS_SESSION_CHANGE_MEMBER:
				{PaxosSessionChangeMemberReq_t	*pReq;
				PaxosSessionChangeMemberRpl_t	Rpl;

				SLOG( pAcceptor, LogINF,
				"### CHANGE_MEMBER:pAccept=%p pFd=%p ####",
				pAccept, pAccept->a_pFd );

				pReq = (PaxosSessionChangeMemberReq_t*)pPSH;
				pAcceptor->c_pSessionCell->c_aPeerAddr[pReq->c_Id].a_Addr
					= pReq->c_Addr;

				SEQ_INC( pAcceptor->c_pSessionCell->c_Version );

				SESSION_MSGINIT( &Rpl, pPSH->h_Cmd, 0, 0, sizeof(Rpl) );
				Ret = PaxosAcceptSend( pAccept, (PaxosSessionHead_t*)&Rpl );
				}
				break;

			default:
				/* OutOfBand Garbage */
				TIMESPEC( Now );

				if( Validate 
				&& TIMESPECCOMPARE(PaxosCatchupRecvLast(pPaxos), Now) < 0 ) {
ASSERT( CMP(backSeq,seq) <= 0 );
					/*
					 *	OutOfBand remove
					 */
					AcceptorOutOfBand_t	*pOut, *pW;

					MtxLock( &pAcceptor->c_OutOfBand.o_Mtx );

					list_for_each_entry_safe( pOut, pW,
							&pAcceptor->c_OutOfBand.o_Lnk, o_Lnk ) {

						/* $B;~4V$b!)(B */
						if( time(0) < pOut->o_Last ) break;
						if( !pOut->o_bValidate )	continue;
						if( CMP( pOut->o_Seq, backSeq ) >= 0 )	break;

						_OutOfBandRemove( pAcceptor, pOut );
					}
					MtxUnlock( &pAcceptor->c_OutOfBand.o_Mtx );
				}
				if( pAccept ) {
					if( !pAccept->a_Opened ) {
						SLOG( pAcceptor, LogERR, 
						"=== DEFAULT: NOT OPENED seq=%u Pid=%d-%d Seq=%d ===",
						seq, pPSH->h_Id.i_Pid, pPSH->h_Id.i_No, 
						pPSH->h_Id.i_Seq );
					}
ASSERT( pAccept->a_Opened );
					Ret = (pAcceptor->c_IF.if_fAccepted)( pAccept, pPSH );
					/* $B>e0L$G;OKv$7$F$$$kH&(B */
					bFree	= FALSE;
				} else {
					SLOG( pAcceptor, LogSYS,
					"=== DEFAULT: ALREADY CLOSED seq=%u "
					"Pid=%d-%d Seq=%d Try=%d ===",
					seq, pPSH->h_Id.i_Pid, pPSH->h_Id.i_No, 
					pPSH->h_Id.i_Seq, pPSH->h_Id.i_Try );

ABORT();
				}
				break;
			}

			if( pAccept ) {

				PaxosAcceptedEnd( pAccept, &Id );
				PaxosAcceptPut( pAccept );
SLOG( pAcceptor, LogDBG, "Count=%d",pAccept->a_GE.e_Cnt );
			}
			break;
		}
next:
		MsgDestroy( pMsg );
		if( bFree )	(pAcceptor->c_IF.if_fFree)( pPSH );
	}
	return( NULL );
}

PaxosSessionHead_t*
PaxosAcceptorRecvMsg( PaxosAcceptor_t *pAcceptor, int fd )
{
	PaxosSessionHead_t	*pM, M;
	int	Ret;
	
	errno = 0;
	if( PeekStream( fd, (char*)&M, sizeof(M) ) )	return( NULL );

	pM	= (PaxosSessionHead_t*)(pAcceptor->c_IF.if_fMalloc)( M.h_Len );
	if( !pM ) {
SLOG( pAcceptor, LogDBG, "h_Len=%d", M.h_Len );
		return( NULL );
	}

	if( (Ret = RecvStream( fd, (char*)pM, M.h_Len ) ) ) {
SLOG( pAcceptor, LogDBG, "Ret=%d h_Len=%d", Ret, M.h_Len );
		(pAcceptor->c_IF.if_fFree)( pM );
		return( NULL );
	}
	if( pM->h_Cksum64 ) {
		assert( !in_cksum64( pM, pM->h_Len, 0 ) );
	}
	return( pM );
}

int
PaxosAcceptorInitUsec( struct PaxosAcceptor *pAcceptor, int Id, 
	int	ServerCore,	int ServerExtension, int64_t MemLimit, bool_t bCksum,
	PaxosSessionCell_t*	pSessionCell,
	PaxosCell_t* 		pPaxosCell,
	int64_t				UnitUsec,
	int	Workers,
	PaxosAcceptorIF_t* pIF )
{
	int	j;
	struct Paxos* pPaxos;
	int	l;
	int	Maximum;

	Maximum	= ServerCore + ServerExtension;

	memset( pAcceptor, 0, sizeof(*pAcceptor) );

	pAcceptor->c_IF	= *pIF;
	if( !pAcceptor->c_IF.if_fMalloc )	pAcceptor->c_IF.if_fMalloc	= Malloc;
	if( !pAcceptor->c_IF.if_fFree )		pAcceptor->c_IF.if_fFree	= Free;

	pAcceptor->c_MasterWas		= -1;
	pAcceptor->c_Enabled		= FALSE;
	pAcceptor->c_MyId			= Id;

	l	= sizeof(PaxosSessionCell_t) 
		+ sizeof(PaxosAddr_t)*Maximum;
	pAcceptor->c_pSessionCell	= (PaxosSessionCell_t*)
									(pAcceptor->c_IF.if_fMalloc)( l );
	l	= sizeof(PaxosSessionCell_t) 
		+ sizeof(PaxosAddr_t)*pSessionCell->c_Maximum;
	memcpy( pAcceptor->c_pSessionCell, pSessionCell, l );
	pAcceptor->c_pSessionCell->c_Max 	= ServerCore;
	pAcceptor->c_pSessionCell->c_Maximum = Maximum;

	pAcceptor->c_pFds	= (int*)
						(pAcceptor->c_IF.if_fMalloc)(sizeof(int)*Maximum );
	for( j = 0; j < Maximum; j++ ) {
		pAcceptor->c_pFds[j]	= -1;
	}

	MtxInit( &pAcceptor->c_Mtx );

	GElmCtrlInit( &pAcceptor->c_Accept, NULL,
				_AcceptAlloc, _AcceptFree, _AcceptSet, _AcceptUnset, NULL,
				20000, PRIMARY_1009, HashSessionCode, HashSessionCompare, NULL,
				Malloc, Free, NULL );

	pAcceptor->c_Accept.ge_pTag	= pAcceptor;

	FdEventCtrlCreate( &pAcceptor->c_FdEvent.f_Ctrl );

	MtxInit( &pAcceptor->c_OutOfBand.o_Mtx );
	pAcceptor->c_OutOfBand.o_MemLimit	= MemLimit;

	pAcceptor->c_bCksum	= bCksum;

	list_init( &pAcceptor->c_OutOfBand.o_Lnk );
	pAcceptor->c_OutOfBand.o_Cnt	= 0;
	HashInit( &pAcceptor->c_OutOfBand.o_Hash, PRIMARY_4003, 
				AcceptorHashOutOfBandCode, AcceptorHashOutOfBandCompare,
				NULL, 
				pAcceptor->c_IF.if_fMalloc, pAcceptor->c_IF.if_fFree, 
				NULL );
	pPaxos = (struct Paxos*)(pAcceptor->c_IF.if_fMalloc)( PaxosGetSize() );
	pAcceptor->c_pPaxos 			= pPaxos;

	pAcceptor->c_Workers	= Workers;
	pAcceptor->c_aMsg		= (Queue_t*)(pAcceptor->c_IF.if_fMalloc)
											( sizeof(Queue_t)*Workers );
	for( j = 0; j < Workers; j++ ) {
		QueueInit( &pAcceptor->c_aMsg[j] );
	}

	PaxosInitUsec( pPaxos, Id, 
			ServerCore, ServerExtension,
			pPaxosCell, UnitUsec, bCksum,
			Election, Notify, 
			_AcceptorOutOfBandMarshal, _AcceptorOutOfBandUnmarshal,
			pAcceptor->c_IF.if_fMalloc, pAcceptor->c_IF.if_fFree,
			pAcceptor->c_IF.if_fLoad );
	PaxosSetTag( pPaxos, pAcceptor );
	return( 0 );
}

typedef struct ConnectAbort {
	PaxosAcceptor_t	*ca_pAcceptor;
	PaxosClientId_t	ca_Id;
} ConnectAbort_t;

int
ConnectAbortTimeout( TimerEvent_t* pE )
{
	ConnectAbort_t* pConnectAbort	= (ConnectAbort_t*)pE->e_pArg;
	PaxosAcceptor_t* pAcceptor		= pConnectAbort->ca_pAcceptor;
	PaxosAccept_t	*pAccept;
	int	Ret;
	PaxosClientId_t	Id;

	DBG_ENT();

	MtxLock( &pAcceptor->c_Mtx );

	Id = pConnectAbort->ca_Id;

	pAccept = PaxosAcceptGet( pAcceptor, &Id, FALSE );

	if( pAccept && PaxosIsIamMaster( pAcceptor->c_pPaxos )
		&& pAccept->a_pFd 
		&& pAccept->a_Health + DT_SESSION_HEALTH < time(0) ) {

		Ret = _SessionAbortSend( pAcceptor, pAccept );
		if( Ret ) {
			SLOG( pAcceptor, LogERR, "ABORT SEND ERROR Ret=%d", Ret );
		}
	} else {
		SLOG( pAcceptor, LogERR, "#### ABORT NOT SENT INET=%s PID=%d-%d",
			inet_ntoa( Id.i_Addr.sin_addr), Id.i_Pid, Id.i_No );
	}
	if( pAccept )	PaxosAcceptPut( pAccept );

	MtxUnlock( &pAcceptor->c_Mtx );

	(pAcceptor->c_IF.if_fFree)( pConnectAbort );

	DBG_EXT( Ret );
	return( Ret );
}

int
PaxosDisconnectAccept( PaxosAccept_t *pAccept )
{
	PaxosAcceptor_t	*pAcceptor	= pAccept->a_pAcceptor;
	int	Ret;

	DBG_ENT();
	errno = 0;
	/*
	 * $B6/@)@ZCG(B
	 * $BD>$A$K(BCLOSE$B$7;q8;$r2rJ|(B
	 */

	MtxLock( &pAccept->a_Mtx );

	pAccept->a_pFd	= NULL;

	if( pAccept->a_Opened 
			&& pAccept->a_Opened != ACCEPT_CLOSING ) {

		if( PaxosIsIamMaster( pAcceptor->c_pPaxos ) ) {
			if( errno ) {

				ConnectAbort_t	*pConnectAbort;	

				pConnectAbort	= (ConnectAbort_t*)
					(pAcceptor->c_IF.if_fMalloc)(sizeof(ConnectAbort_t));

				pConnectAbort->ca_pAcceptor	= pAcceptor;
				pConnectAbort->ca_Id	= pAccept->a_Id;

/*
 *	$B%/%i%$%"%s%H$O%^%9%?!<$,8N>c$7$?>l9g!"(B
 *	$B%^%9%?!<8rBX$HG'<1$7!"(BElection$B$NG\BT$D(B
 */
#define	ABORT_TIMER	\
(PaxosMasterElectionTimeout(pAcceptor->c_pPaxos)*2/1000+DT_SESSION_HEALTH_MS)

				SLOG( pAcceptor,LogERR,"errno=%d pAccept=%p TimerSet=%dms", 
						errno, pAccept, ABORT_TIMER );

				PaxosAcceptorTimerSet( pAcceptor, ABORT_TIMER,
					ConnectAbortTimeout, pConnectAbort );	
			} else {
SLOG( pAcceptor, LogSYS,"#### AbortSend ####");
				Ret = _SessionAbortSend( pAcceptor, pAccept );
				if( Ret ) {
					SLOG( pAcceptor, LogERR, "ABORT SEND ERROR Ret=%d", Ret );
				}
			}
		}
	}
	MtxUnlock( &pAccept->a_Mtx );
	DBG_EXT(0);
	return( 0 );
}

void*
ThreadWorker( void* pA )
{
	ThreadAcceptorArg_t *pArg = (ThreadAcceptorArg_t*)pA;
	PaxosAccept_t*	pAccept = NULL;
	PaxosSessionHead_t	*pH = NULL;
	int	Ret = 0;
	int	fd;
	PaxosClientId_t	Id;
	PaxosSessionHead_t	Rpl;
	PaxosAcceptor_t*	pAcceptor;
	IOSocket_t	*pIO;
	Msg_t	*pMsg;
	PaxosAcceptorFdEvent_t	*pEv;
	int	No;
	Msg_t		*pMsgRpl;
	MsgVec_t	Vec;
	int	l;
	struct Paxos	*pPaxos;

	DBG_ENT();

	pAcceptor	= pArg->a_pAcceptor;
	No		= pArg->a_No;
	(*pAcceptor->c_IF.if_fFree)(pArg);

	while( 1 ) {

		QueueWaitEntry( &pAcceptor->c_aMsg[No], pMsg, m_Lnk );
		if( !pMsg ) {
			SLOG( pAcceptor, LogINF,
					"QueueWaitEntry:c_Msg-pMsg=NULL No=%d", No );
			continue;
		}
		pH	= (PaxosSessionHead_t*)MsgFirst(pMsg);

		pEv	= (PaxosAcceptorFdEvent_t*)pMsg->m_pTag0;
		pAccept		= pEv->e_pAccept;
		pIO			= &pEv->e_IOSock;
		fd			= pIO->s_Fd;

SLOG( pAcceptor, LogDBG, "<<<<< START fd=%d pH=%p >>>>>", fd, pH );
		/* $B@5>o(Bclose$B!"@ZCG(B */
		/* pEv$B$r;2>H$7$?J}$,$h$$(B */
		if( !pH ) {
			goto disconnect;
		}

		Id	= pH->h_Id;
SLOG( pAcceptor, LogDBG,
"==>pH CMD=0x%x fd=%d INET=%s [PID=%d No=%d Seq=%d] CODE=%d pAccept=%p", 
pH->h_Cmd, fd, inet_ntoa(Id.i_Addr.sin_addr), 
Id.i_Pid, Id.i_No, Id.i_Seq, pH->h_Code, pAccept );

		/* $B%9%l%C%I$O(Bfreeze$BCf$O(Bfreeze$B$7$?%/%i%$%"%s%H$N$_<u$1IU$1$k(B */
		/* $B$?$@$7!"%X%k%9%A%'%C%/$ODL$9(B */
		if( pH->h_Cmd != PAXOS_SESSION_HEALTH 
				&& pAcceptor->c_pFdFreeze 
				&& pAcceptor->c_pFdFreeze != pEv ) {

			SLOG( pAcceptor, LogINF, "FREEZE->discard CMD=0x%x INET=%s "
				"[PID=%d No=%d Seq=%d] CODE=%d pAccept=%p", 
				pH->h_Cmd, inet_ntoa(Id.i_Addr.sin_addr), 
				Id.i_Pid, Id.i_No, Id.i_Seq, pH->h_Code, pAccept );

			MsgDestroy( pMsg );
			goto next;
		}
		/* $B%;%C%7%g%s4IM}(B(Accept Obj)$B$H4X78$J$$(B */
		switch( (int)pH->h_Cmd ) {

		case PAXOS_SESSION_ACCEPTOR: {
			PaxosSessionAcceptorRpl_t	RplAcceptor;

			RplAcceptor.a_Acceptor	= *pAcceptor;
			RplAcceptor.a_pAcceptor	= pAcceptor;

			IOWrite( (IOReadWrite_t*)pIO, &RplAcceptor, sizeof(RplAcceptor) );
			MsgDestroy( pMsg );
			goto next; }

		case PAXOS_SESSION_DUMP: {
			PaxosSessionDumpReq_t	*pReqDump = (PaxosSessionDumpReq_t*)pH;

			IOWrite((IOReadWrite_t*)pIO, pReqDump->d_pStart, pReqDump->d_Size );
			MsgDestroy( pMsg );
			goto next; }

		/*
		 * $B1~Ez$rJV$5$J$$$,!"%m%C%/$,<h$l$J$1$l$P<!$NMW5a$r<h$j$K9T$+$J$$(B
		 */
		case PAXOS_SESSION_LOCK:
			Ret = 0;
			if( pAcceptor->c_pFdFreeze ) {
				Ret	= EEXIST;
			} else {
				PaxosAcceptorFreeze( pAcceptor, pEv );

				Ret = (pAcceptor->c_IF.if_fLock)();					// AP

				if( Ret ) {
					PaxosAcceptorDefreeze(pAcceptor);
				} else {
					pAcceptor->c_pFdFreeze	= pEv;
					SLOG( pAcceptor, LogINF, "PAXOS_SESSION_LOCK:c_pFdFreeze=%p", pAcceptor->c_pFdFreeze );
				}
			}
			pH->h_Error = Ret;
			SLOG( pAcceptor, LogINF, "PAXOS_SESSION_LOCK:Ret=%d", Ret );
			SendStream( fd, (char*)pH, sizeof(*pH) );

			MsgDestroy( pMsg );
			goto next;

		case PAXOS_SESSION_UNLOCK:
//DBG_PRT("UNLOCK\n");
			SLOG( pAcceptor, LogSYS, "UNLOCK" );
			(pAcceptor->c_IF.if_fUnlock)();
			PaxosAcceptorDefreeze( pAcceptor );
			pAcceptor->c_pFdFreeze	= NULL;
			SendStream( fd, (char*)pH, sizeof(*pH) );	// 2014.01.30 added
			MsgDestroy( pMsg );
			goto next;

		case PAXOS_SESSION_GET_MEMBER:

			SLOG( pAcceptor, LogSYS, "GET_MEMBER" );
			pMsgRpl = MsgCreate( 2,	PaxosAcceptGetfMalloc(pAccept), 
									PaxosAcceptGetfFree(pAccept) );

			l	= sizeof(PaxosSessionCell_t) 
				+ sizeof(PaxosAddr_t)*pAcceptor->c_pSessionCell->c_Maximum;
			pH->h_Len	= sizeof(*pH) + l;

			MsgVecSet( &Vec, 0, pH, sizeof(*pH), sizeof(*pH) );
			MsgAdd( pMsgRpl, &Vec );

			MsgVecSet( &Vec, 0, (char*)pAcceptor->c_pSessionCell, l, l );
			MsgAdd( pMsgRpl, &Vec );

			PaxosAcceptSendByMsg( pAccept, pMsgRpl );

			MsgDestroy( pMsg );
			goto next;

		case PAXOS_SESSION_BACKUP:
//DBG_PRT("BACKUP\n");
			SLOG( pAcceptor, LogSYS, "BACKUP" );

			/* pH$B%G!<%?$OJ]B8$5$l$k(B */
			pMsg->m_aVec[0].v_Flags &= ~VEC_MINE;
			MsgDestroy( pMsg );

			if( pAcceptor->c_IF.if_fBackup ) 
				(pAcceptor->c_IF.if_fBackup)( pH );
			SendStream( fd, (char*)pH, sizeof(*pH) );
			goto next;

		case PAXOS_SESSION_TRANSFER:
//DBG_PRT("TRANSFER\n");
			SLOG( pAcceptor, LogSYS, "TRANSFER" );
			if( pAcceptor->c_IF.if_fTransferSend ) 
				(pAcceptor->c_IF.if_fTransferSend)( (IOReadWrite_t*)pIO );
			MsgDestroy( pMsg );
			LogDump( pAcceptor->c_pLog );
			goto next;

		case PAXOS_SESSION_MARSHAL:
//DBG_PRT("MARSHAL\n");
			SLOG( pAcceptor, LogSYS, "MARSHAL" );
			PaxosMarshal(PaxosAcceptorGetPaxos(pAcceptor), 
									(IOReadWrite_t*)pIO );		// Paxos

			if( pAcceptor->c_IF.if_fGlobalMarshal )
				(pAcceptor->c_IF.if_fGlobalMarshal)( (IOReadWrite_t*)pIO );
			_PaxosAcceptorMarshal( (IOReadWrite_t*)pIO, pAcceptor );

			IOMarshalEnd( (IOReadWrite_t*)pIO );
			MsgDestroy( pMsg );
			goto next;

		case PAXOS_SESSION_AUTONOMIC:
			SLOG( pAcceptor, LogSYS, "AUTONOMIC" );
			PaxosAutonomic(PaxosAcceptorGetPaxos(pAcceptor), pH->h_From );
			MsgDestroy( pMsg );
			goto next;

		case PAXOS_SESSION_SHUTDOWN:
			SLOG( pAcceptor, LogSYS, "SHUTDOWN" );
			Ret = 0;
			if( pAcceptor->c_IF.if_fShutdown ) {
				pPaxos = PaxosAcceptorGetPaxos( pAcceptor );
				Ret = (pAcceptor->c_IF.if_fShutdown)( pAcceptor, 
							(PaxosSessionShutdownReq_t*)pH, 
							PaxosGetSeq( pPaxos ) );
			}

			if( pAcceptor->c_pLog != NULL  )	LogDump( pAcceptor->c_pLog );
			IOWrite( (IOReadWrite_t*)pIO, &Ret, sizeof(Ret) );

			MsgDestroy( pMsg );
			EXIT( 0 );

		case PAXOS_SESSION_ANY:
			(pAcceptor->c_IF.if_fAny)( pAcceptor, pH, fd );
			MsgDestroy( pMsg );
			goto next;
		}

		/* $B%;%C%7%g%s4IM}(B(Accept Obj)$B$H4X78$"$j(B */

		switch( (int)pH->h_Cmd ) {
		case PAXOS_SESSION_HEALTH:
			pAccept	= PaxosAcceptGet( pAcceptor, &Id, FALSE );
			ASSERT( pAccept );

//SLOG( pAcceptor, "pAccept=%p a_Cnt=%d", pAccept, pAccept->a_GE.e_Cnt );

			pAccept->a_Health = time(0);

			PaxosAcceptPut( pAccept );

			MsgDestroy( pMsg );
			goto next;

		/* $B@\B3$G$b(Bmaster$B$rJV$5$J$$$H?7(Bmaster$B$K@\B3$,3NN)$7$J$$(B */
		case PAXOS_SESSION_BIND:

			pAccept	= PaxosAcceptGet( pAcceptor, &Id, TRUE );

			pAccept->a_pFd	= pEv;
			pEv->e_pAccept	= pAccept;
			DBG_TRC("BINDED pAccept=%p a_Cnt=%d fd=%d a_pFd=%p\n", 
					pAccept, pAccept->a_GE.e_Cnt, fd, pAccept->a_pFd );

			SLOG( pAcceptor, LogINF, "BINDED pAccept=%p e_Cnt=%d fd=%d a_pFd=%p", 
					pAccept, pAccept->a_GE.e_Cnt, fd, pAccept->a_pFd );
			MsgDestroy( pMsg );
			/* BIND$B$G$O(BPaxosAcceptPut$B$r$7$J$$$GJ];}(B */
			goto next;

		case PAXOS_SESSION_ELECTION:
DBG_PRT("ELECTION???\n");
ABORT();
		}

		pAccept	= PaxosAcceptGet( pAcceptor, &Id, TRUE );

		ASSERT( pAccept );
		pAccept->a_Access	= time(0);

		/*
		 * $B<uM}:Q$_$+=hM}:Q$_$+(B
		 * $BDLHV$O;2>H5Z$S99?7<B9TIt$G@_Dj$5$l$k(B
		 * $B<B9T$,CY$l$?>l9gJ#?tMW5a$bEjF~$5$l$k(B
		 */
		Ret = PaxosAcceptIsAccepted( pAccept, pH );
		if( Ret ) {

			SLOG( pAcceptor, LogERR,
			"REPLYED or ACCEPTED Ret=%d pH=%p Pid=%d-%d-%d "
			"Seq=%d Try=%d Cmd=0x%x",
			Ret, pH, pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Reuse,
			pH->h_Id.i_Seq, pH->h_Id.i_Try, pH->h_Cmd );


			/* $B%/%i%$%"%s%H$,BT$D$3$H$K$J$k$N$G(Bmaster$B$G$J$$;~$ODLCN$9$k(B */
			if( !PaxosIsIamMaster( pAcceptor->c_pPaxos ) ) {
//DBG_PRT("Not Master\n");
				SESSION_MSGINIT( &Rpl, pH->h_Cmd, PAXOS_Im_NOT_MASTER, 
												0, sizeof(Rpl) );
				Rpl.h_Master = PaxosGetMaster( pAcceptor->c_pPaxos );
				Rpl.h_Epoch = PaxosEpoch( pAcceptor->c_pPaxos );
				SLOG( pAcceptor, LogINF,
				"SEND=>MyId=%d h_Master=%d", pAcceptor->c_MyId, Rpl.h_Master );
				Ret = PaxosAcceptSend( pAccept, &Rpl );
			}
			MsgDestroy( pMsg );

			PaxosAcceptPut( pAccept );
			goto next;
		}
		switch( pH->h_Cmd ) {

		case PAXOS_SESSION_CLOSE:
			MtxLock( &pAccept->a_Mtx );

			int	Opened;

			/*
			 * $B4{$K%/%m!<%:$5$l$F$$$k(B $B%^%9%?8rBX$G1~Ez$G$-$J$+$C$?(B
			 * $BDL?.>c32$G%"%\!<%H$7$F$$$k>l9g$,$"$k!#(B
			 */
			if( !pAccept->a_Opened ) {

				SLOG( pAcceptor, LogERR,
				"CLOSE:NOT OPENED Accept=%p Pid=%d-%d-%d Seq=%d Master=%d "
				"a_Opened=%d a_Cmd=%d",
				pAccept, pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, 
				pAccept->a_Id.i_Reuse, pAccept->a_Id.i_Seq,
				PaxosGetMaster( pAcceptor->c_pPaxos), 
				pAccept->a_Opened, pAccept->a_Accepted.a_Cmd );

				SLOG( pAcceptor, LogERR,
				"pH Pid=%d-%d-%d Seq=%d Try=%d Cmd=0x%x",
				pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Reuse,
				pH->h_Id.i_Seq, pH->h_Id.i_Try, pH->h_Cmd );

				SESSION_MSGINIT( &Rpl, pH->h_Cmd, PAXOS_OK, 
								ERR_SESSION_NOT_FOUND, sizeof(Rpl) );
				Rpl.h_Master = PaxosGetMaster( pAcceptor->c_pPaxos );
				Rpl.h_Epoch = PaxosEpoch( pAcceptor->c_pPaxos );
				Ret = _PaxosAcceptSend( pAccept, &Rpl );

				MtxUnlock( &pAccept->a_Mtx );
				MsgDestroy( pMsg );
			PaxosAcceptPut( pAccept );
				goto next;
			}
			if( pAccept->a_Opened == 0 
				|| pAccept->a_Opened == ACCEPT_CLOSING ) {

				SLOG( pAcceptor, LogERR, "pH Pid=%d-%d-%d Seq=%d Try=%d",
				pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Reuse, 
				pH->h_Id.i_Seq, pH->h_Id.i_Try);

				MtxUnlock( &pAccept->a_Mtx );
				MsgDestroy( pMsg );
				PaxosAcceptPut( pAccept );
				goto next;
			}
			Opened	= pAccept->a_Opened;
			pAccept->a_Opened	= ACCEPT_CLOSING;

			MtxUnlock( &pAccept->a_Mtx );

			Ret = PaxosRequest( pAcceptor->c_pPaxos, pH->h_Len, pH, FALSE );
			/*$B!!K\%9%l%C%I$OJD$8$k$,!"(BpAccept$B$O%*!<%W%s>uBV$J$N$G:o=|$5$l$J$$(B */
			if( Ret ) {
			/* $BEjF~<:GT$J$N$G1~Ez$rJV$9(B */
				SLOG( pAcceptor, LogERR,
				"CLOSE: Ret=%d pH=%p Pid=%d-%d-%d Seq=%d Try=%d Cmd=0x%x",
				Ret, pH, pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Reuse,
				pH->h_Id.i_Seq, pH->h_Id.i_Try, pH->h_Cmd );

				MtxLock( &pAccept->a_Mtx );
				if( pAccept->a_Opened == ACCEPT_CLOSING ) {
					pAccept->a_Opened = Opened;
				}
				MtxUnlock( &pAccept->a_Mtx );
			}
			break;

		case PAXOS_SESSION_OPEN:
			MtxLock( &pAccept->a_Mtx );
			if( pAccept->a_Opened ) {

				SLOG( pAcceptor, LogERR,
				"OPENING OPENED CLOSING a_Opened=%d "
				"Pid=%d-%d Seq=%u i_Seq=%u Master=%d",
				pAccept->a_Opened, pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, 
				pAccept->a_Id.i_Seq, pH->h_Id.i_Seq, 
				PaxosGetMaster( pAcceptor->c_pPaxos) );

ASSERT( pAccept->a_Opened != ACCEPT_CLOSING );

				/* OPENING$B$G%^%9%?!<8rBX$N$?$aEjF~$G$-$J$$>l9g$,$"$k(B*/
				MtxUnlock( &pAccept->a_Mtx );
				MsgDestroy( pMsg );
			PaxosAcceptPut( pAccept );
				goto next;
			} else {
				/*
				 * Master$B8rBe;~$K(BOpen$BMW5a$,J#?t2sMh$k2DG=@-$"$j(B
				 *	$B:G=i$O%?%$%`%"%&%HCM$,$o$+$i$J$$(B
				 */
				if( pAccept->a_Opened != 0 ) {
					SLOG( pAcceptor, LogERR,
					"Pid=%d-%d Seq=%u i_Seq=%u Master=%d",
					pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, 
					pAccept->a_Id.i_Seq, pH->h_Id.i_Seq,
					PaxosGetMaster( pAcceptor->c_pPaxos) );
				}
				pAccept->a_Opened	= ACCEPT_OPENING;
			}
			MtxUnlock( &pAccept->a_Mtx );
			Ret = PaxosRequest( pAcceptor->c_pPaxos, pH->h_Len, pH, FALSE );
			if( Ret ) {
			/* $BEjF~<:GT$J$N$G1~Ez$rJV$9(B */

				SLOG( pAcceptor, LogERR,
				"OPEN: Ret=%d pH=%p Pid=%d-%d-%d Seq=%d Try=%d Cmd=0x%x",
				Ret, pH, pH->h_Id.i_Pid, pH->h_Id.i_No, pH->h_Id.i_Reuse,
				pH->h_Id.i_Seq, pH->h_Id.i_Try, pH->h_Cmd );

				MtxLock( &pAccept->a_Mtx );
				if( pAccept->a_Opened == ACCEPT_OPENING ) {
					pAccept->a_Opened = 0;
				}
				MtxUnlock( &pAccept->a_Mtx );
			}
			break;

		case PAXOS_SESSION_EVENT:
		case PAXOS_SESSION_CHANGE_MEMBER:
			SLOG( pAcceptor, LogSYS,
				"### EVENT(%d)/CHANGE_MEMBER(%d) h_Cmd=%d ####",
				PAXOS_SESSION_EVENT, PAXOS_SESSION_CHANGE_MEMBER, pH->h_Cmd );
			Ret = PaxosRequest( pAcceptor->c_pPaxos, pH->h_Len, pH, FALSE );
			break;

		default:
			/*
			 * $B;2>HJ,;6$O!"%*!<%W%sA0$NMW5a$"$j(B
			 * $B=g=x@)8f$O>e0L(B
			 */
			Ret = (pAcceptor->c_IF.if_fRequest)( pAcceptor, pH, fd );
			break;
		}
		if( Ret ) {
			SESSION_MSGINIT( &Rpl, pH->h_Cmd, Ret,0, sizeof(Rpl) );
			Rpl.h_Master = PaxosGetMaster( pAcceptor->c_pPaxos );
			Rpl.h_Epoch = PaxosEpoch( pAcceptor->c_pPaxos );
			Ret = PaxosAcceptSend( pAccept, &Rpl );

			SLOG( pAcceptor, LogERR, "SEND Ret=%d Master=%d Epoch=%d", 
					Ret, Rpl.h_Master, Rpl.h_Epoch );
		}

		MsgDestroy( pMsg );
		PaxosAcceptPut( pAccept );
		goto next;

ABORT();
disconnect:
		if( pAcceptor->c_pFdFreeze /*Locked*/ ) {
			SLOG( pAcceptor, LogINF, "disconnect:c_pFdFreeze=%p", pAcceptor->c_pFdFreeze );
			(pAcceptor->c_IF.if_fUnlock)();
			PaxosAcceptorDefreeze( pAcceptor );
			pAcceptor->c_pFdFreeze	= NULL;
		}
		if( pAccept ) {

			PaxosDisconnectAccept( pAccept );

			SLOG( pAcceptor, LogINF,
			"PUT:pAccept=%p e_Cnt=%d", pAccept, pAccept->a_GE.e_Cnt );

			PaxosAcceptPut( pAccept );
			pAccept	= NULL;
		}
		Ret = shutdown( fd, 2 );
		close( fd );

		SLOG( pAcceptor, LogINF, "PUT:pEv=%p", pEv );
		Free( pEv );

next:
		SLOG( pAcceptor, LogDBG, "<<<<< END fd=%d Master=%d >>>>>", 
			fd, PaxosIsIamMaster(pAcceptor->c_pPaxos) );
	}
	DBG_EXT( Ret );
	pthread_exit( (void*)NULL );
	return( NULL );
}

int
PaxosAcceptorGetLoad( PaxosAcceptor_t *pAcceptor )
{
	return( FdEvCnt( &pAcceptor->c_FdEvent.f_Ctrl) );
}

/*
 *	Event
 */
int
PaxosAcceptEventRaise( PaxosAccept_t* pAccept, PaxosSessionHead_t *pH )
{
	PaxosSessionHead_t	*pRpl;
	int	Ret = 0;
	PaxosAcceptor_t*	pAcceptor;
	Msg_t		*pMsg;
	MsgVec_t	Vec;


	MtxLock( &pAccept->a_Mtx );

	pAcceptor = pAccept->a_pAcceptor;

	if( pAccept->a_Events.e_pRaised )	pMsg = pAccept->a_Events.e_pRaised;
	else {
		pMsg = MsgCreate( 2, 	PaxosAcceptGetfMalloc(pAccept), 
								PaxosAcceptGetfFree(pAccept) );
		pAccept->a_Events.e_pRaised	= pMsg;
	}
	if( !pAccept->a_Events.e_Send ) {

		if( pAccept->a_Events.e_Cnt < EVENT_MAX ) {

			MsgVecSet( &Vec, VEC_MINE, pH, pH->h_Len, pH->h_Len );
			MsgAdd( pMsg, &Vec );

			pAccept->a_Events.e_Cnt++;
		} else {
			pAccept->a_Events.e_Omitted++;
			(pAcceptor->c_IF.if_fFree)( pH );
		}
	} else  {
ASSERT( pAccept->a_Events.e_Cnt == 0 );

		pRpl = (PaxosSessionHead_t*)
				SESSION_MSGALLOC( PAXOS_SESSION_EVENT, 0, 0, 
									sizeof(*pRpl) + pH->h_Len );	
		pRpl->h_EventCnt		= 1;
		pRpl->h_EventOmitted	= pAccept->a_Events.e_Omitted; // to be 0

		MsgVecSet( &Vec, VEC_MINE, pRpl, sizeof(*pRpl), sizeof(*pRpl) );
		MsgAdd( pMsg, &Vec );

		MsgVecSet( &Vec, VEC_MINE, pH, pH->h_Len, pH->h_Len );
		MsgAdd( pMsg, &Vec );

		Ret = _PaxosAcceptReplyByMsg( pAccept, pMsg );

		pAccept->a_Events.e_pRaised	= NULL;
		pAccept->a_Events.e_Send	= FALSE;
	}
	MtxUnlock( &pAccept->a_Mtx );
	return( Ret );
}

int
PaxosAcceptEventGet( PaxosAccept_t* pAccept, PaxosSessionHead_t* pH )
{
	int	Ret = 0;
	PaxosSessionHead_t	*pRpl;
	Msg_t		*pMsg;
	MsgVec_t	Vec;

	DBG_ENT();

	ASSERT( pAccept );

	MtxLock( &pAccept->a_Mtx );

	if( pAccept->a_Events.e_Cnt > 0 ) {

		pMsg	= pAccept->a_Events.e_pRaised;

		pRpl	= (PaxosSessionHead_t*) MsgMalloc( pMsg, sizeof(*pRpl) );
		SESSION_MSGINIT( pRpl, PAXOS_SESSION_EVENT, 0, 0, 
							sizeof(*pRpl) + MsgTotalLen(pMsg) );	

		pRpl->h_EventCnt		= pAccept->a_Events.e_Cnt ;
		pRpl->h_EventOmitted	= pAccept->a_Events.e_Omitted; // to be 0

		MsgVecSet( &Vec, VEC_MINE, pRpl, sizeof(*pRpl), sizeof(*pRpl) );
		MsgInsert( pMsg, &Vec );

		pAccept->a_Events.e_Cnt		= 0;
		pAccept->a_Events.e_Omitted	= 0;

		Ret = _PaxosAcceptReplyByMsg( pAccept, pMsg );
		pAccept->a_Events.e_pRaised = NULL;

	} else {
		pAccept->a_Events.e_Send = TRUE;
	}
	MtxUnlock( &pAccept->a_Mtx );
	return( Ret );
}

struct sockaddr_in*
PaxosAcceptorGetAddr( struct PaxosAcceptor* pAcceptor, int j )
{
	return( PaxosGetAddr( pAcceptor->c_pPaxos, j ) );
}

int AcceptorAcceptHandler( FdEvent_t *pEv, FdEvMode_t Mode );
int AcceptorRecvHandler( FdEvent_t *pE, FdEvMode_t Mode );
#define	FD_NRM	0
#define	FD_ADM	1

static	int
PaxosAcceptorStart( PaxosAcceptor_t* pAcceptor,
				PaxosStartMode_t StartMode, uint32_t nextDecide )
{
	pthread_t	th;
	int			i;
	struct sockaddr_in	Port;
	int	ListenFd;
	ThreadAcceptorArg_t	*pArg;
	int	Ret;

	TimerInit( &pAcceptor->c_Timer, TIMER_BTREE,
				0, 0, 0,	// Heap
				pAcceptor->c_IF.if_fMalloc, 
				pAcceptor->c_IF.if_fFree,	// Memory
				5, 			// Btree Order
				NULL );
	TimerEnable( &pAcceptor->c_Timer );
	TimerStart( &pAcceptor->c_Timer );

	/* $B9g0U=hM}(B */
	pthread_create( &th, NULL, ThreadExecute, (void*)pAcceptor->c_pPaxos );
	for( i = 0; i < pAcceptor->c_Workers; i++ ) {
		pArg = (ThreadAcceptorArg_t*)
				(*pAcceptor->c_IF.if_fMalloc)(sizeof(*pArg));
		pArg->a_pAcceptor	= pAcceptor;
		pArg->a_No			= i;

		pthread_create( &th, NULL, ThreadWorker, pArg );
	}

	/* 	Session Port */
	if( (ListenFd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
		LOG( PaxosAcceptorGetLog( pAcceptor), LogERR, "ListenFd=%d, errno=%d", ListenFd, errno);
		return( -1 );
	}
	/* Keep alive */
	int	on = 1;
	Ret = setsockopt(ListenFd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
	if( Ret ) {
		LOG( PaxosAcceptorGetLog( pAcceptor), LogERR, "Ret=%d, errno=%d", Ret, errno);
		return( -1 );
	}
	setsockopt( ListenFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	/* TCP_NODELAY */
	int	flag = 1;
	setsockopt( ListenFd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

	Port = pAcceptor->c_pSessionCell->c_aPeerAddr[pAcceptor->c_MyId].a_Addr;
	if( bind( ListenFd, (struct sockaddr*)&Port, sizeof(Port) ) < 0 ) {
		LOG( PaxosAcceptorGetLog( pAcceptor), LogERR, "errno=%d", errno);
		return( -1 );
	}
//DBG_PRT("BIND(tcp):%s-%d\n", inet_ntoa( Port.sin_addr ), ntohs( Port.sin_port ) );
	listen( ListenFd, 5 );

	FdEvent_t	*pEv;
	uint64_t	U64;

	U64	= ListenFd;

	pEv = (FdEvent_t*)(*pAcceptor->c_IF.if_fMalloc )( sizeof(*pEv) );
#ifdef	ZZZ
	FdEventInit( pEv, FD_NRM, ListenFd, EPOLLIN, 
						pAcceptor, AcceptorAcceptHandler );
#else
	FdEventInit( pEv, FD_NRM, ListenFd, FD_EVENT_READ, 
						pAcceptor, AcceptorAcceptHandler );
#endif

	Ret = FdEventAdd( &pAcceptor->c_FdEvent.f_Ctrl, U64, pEv );
	LOG( PaxosAcceptorGetLog( pAcceptor ), LogINF, "Ret=%d, fd=%d, pEv=%p", Ret, pEv->fd_Fd, pEv );
	if( Ret < 0 )	goto err;

	/* 	Admin Port */
	struct sockaddr_un	AdmAddr;
	char	PATH[PAXOS_CELL_NAME_MAX];
	int	Fd;

	sprintf( PATH, PAXOS_SESSION_ADMIN_PORT_FMT, 
		pAcceptor->c_pSessionCell->c_Name,
		PaxosMyId( PaxosAcceptorGetPaxos(pAcceptor) ) );
	unlink( PATH );

	memset( &AdmAddr, 0, sizeof(AdmAddr) );
	AdmAddr.sun_family	= AF_UNIX;
	strcpy( AdmAddr.sun_path, PATH );

	if( (Fd = socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 ) {
		goto err;
	}
	if( bind( Fd, (struct sockaddr*)&AdmAddr, sizeof(AdmAddr) ) < 0 ) {
		goto err;
	}
	listen( Fd, 5 );

	U64	= Fd;
	pEv = (FdEvent_t*)(*pAcceptor->c_IF.if_fMalloc )( sizeof(*pEv) );
#ifdef	ZZZ
	FdEventInit( pEv, FD_ADM, Fd, EPOLLIN, pAcceptor, AcceptorAcceptHandler );
#else
	FdEventInit( pEv, FD_ADM, Fd, FD_EVENT_READ, 
					pAcceptor, AcceptorAcceptHandler );
#endif

	Ret = FdEventAdd( &pAcceptor->c_FdEvent.f_Ctrl, U64, pEv );
	LOG( PaxosAcceptorGetLog( pAcceptor ), LogINF, "Ret=%d, fd=%d, pEv=%p", Ret, pEv->fd_Fd, pEv );
	if( Ret < 0 )	goto err;

	/*
	 *	Paxos$B9g0U3+;O(B
	 */
	PaxosStart( pAcceptor->c_pPaxos, StartMode, nextDecide );	

	pAcceptor->c_Enabled	= TRUE;

	/* FdLoop */

	Ret = FdEventLoop( &pAcceptor->c_FdEvent.f_Ctrl, -1 );
//perror("");
DBG_PRT("Ret=%d\n", Ret );
	if( Ret < 0 )	goto err;

	return( 0 );
err:
	return( -1 );
}

int
PaxosAcceptDump( PaxosAccept_t *pAccept )
{
	DBG_PRT("=== Accept(%p) pFd=%p Cnt=%d Create=%lu Access=%lu ===\n", 
		pAccept, pAccept->a_pFd, pAccept->a_GE.e_Cnt, 
		pAccept->a_Create, pAccept->a_Access );
	DBG_PRT("Pid=%u-%u-%d Open=%d Start=%u End=%u Reply=%u Cmd=0x%x\n",
		pAccept->a_Id.i_Pid, pAccept->a_Id.i_No, pAccept->a_Id.i_Reuse,
		pAccept->a_Opened,
		pAccept->a_Accepted.a_Start, pAccept->a_Accepted.a_End,
		pAccept->a_Accepted.a_Reply, pAccept->a_Accepted.a_Cmd );
	return( 0 );
}

int
PaxosSessionBackup( PaxosAcceptor_t *pAcceptor, IOReadWrite_t *pIO, Msg_t *pMsg, int Size )
{
	PaxosSessionHead_t	Req;
	PaxosSessionHead_t	Rpl;
	int	len;
	int	i;
	int	retry;
	MsgVec_t	*pVec;
	int	error;
	int	Ret = -1;

	len = ( pMsg != NULL ) ? MsgTotalLen( pMsg ) : 0;
	SESSION_MSGINIT( &Req, PAXOS_SESSION_BACKUP, 0, 0, sizeof(Req)+len );
	Req.h_From	= pAcceptor->c_MyId;
	Req.h_EventCnt	= Size;

#define NUM_BACKUP_RETRY	1 /* retry for file changed in backup process */
	for (retry = 0; retry < NUM_BACKUP_RETRY; retry++ ) {
		IOWrite( pIO, &Req, sizeof(Req) );
		if( pMsg ) {
			for( i = 0; i < pMsg->m_N; i++ ) {
				pVec	= &pMsg->m_aVec[i];
				if( !(pVec->v_Flags & VEC_DISABLED) ) {
					IOWrite( pIO, pVec->v_pStart, pVec->v_Len );
				}
			}
		}

		error = IORead( pIO, &Rpl, sizeof(Rpl) );
		if ( error == sizeof(Rpl) && Rpl.h_Error == 0 ) {
			Ret = 0;
			break;
		}

		LOG( PaxosAcceptorGetLog( pAcceptor ), LogERR,
			"retry=%d, error=%d, h_Error=%d", retry, error, Rpl.h_Error );
	}

	if ( pMsg ) {
		MsgDestroy( pMsg );
	}

	return( Ret );
}

int
PaxosSessionTransfer( PaxosAcceptor_t *pAcceptor, IOReadWrite_t *pIO )
{
	PaxosSessionHead_t	Req;
	int	Ret;

	if( pAcceptor->c_IF.if_fTransferRecv == NULL )	return( 0 );

	SESSION_MSGINIT( &Req, PAXOS_SESSION_TRANSFER, 0, 0, sizeof(Req) );

	IOWrite( pIO, &Req, sizeof(Req) );

	Ret = (pAcceptor->c_IF.if_fTransferRecv)( pIO );

	return( Ret );
}

int
PaxosSnapshotAbort( TimerEvent_t *pE )
{
	ABORT();
}

Timer_t	AutonomicTimer;

static	int
PaxosSessionGetSnapshot( PaxosAcceptor_t *pAcceptor,
						struct Session *pSession, int j, uint32_t *pNextDecide )
{
	IOSocket_t	IO;
	IOReadWrite_t	*pIO = (IOReadWrite_t*)&IO;
	void	*pH;
	PaxosSessionHead_t	Req;
	int	Ret;
	int	RetAgain;
	Msg_t	*pMsgBackup = NULL;
	struct Log*	pLog = PaxosAcceptorGetLog( pAcceptor);
	int		AgainCount = 0;
	int		Size = BACKUP_SIZE_ONCE;

	TimerInit( &AutonomicTimer, 0,
				0, 0, 0,	// Heap
				pAcceptor->c_IF.if_fMalloc, 
				pAcceptor->c_IF.if_fFree,	// Memory
				0, 			
				NULL );
	TimerEnable( &AutonomicTimer );
	TimerStart( &AutonomicTimer );
AgainIncremental:
	/*
	 *	$B5l1JB3>uBV$N>pJs(BMsg$B:n@.(B
	 */
	if( pAcceptor->c_IF.if_fBackupPrepare ) {
		LOG( pLog, LogINF, "BackupPrepare Count=%d", AgainCount);

		pMsgBackup = (pAcceptor->c_IF.if_fBackupPrepare)( AgainCount++ );
	}

	/*
	 *	$B%?!<%2%C%H$NE`7k(B
	 */
	LOG( pLog, LogINF, "PaxosSessionLock, j=%d", j);
	time_t t1,t2;
	t1 = time(NULL);
	Ret = PaxosSessionLock( pSession, j, &pH );
	if( Ret ) {
		LOG(pLog, LogERR, "PaxosSessionLock Ret=%d", Ret );
		goto err;
	}

	/*
	 *	$BDL?.2sO)(B
	 */
	LOG( pLog, LogINF, "IOSocketBind");
	IOSocketBind( &IO, PNT_INT32(pH) );

	TimerEvent_t	*pE;

	pE = TimerSet( &AutonomicTimer, TIMEOUT_SNAP,
				PaxosSnapshotAbort, (void*)pAcceptor );
	/*
	 *	PAXOS_SESSION_BACKUP$B%3%^%s%I$NAw?.(B
	 *		$B%?!<%2%C%H$O:9J,$r:n@.$70l;~J]B8(B
	 */
	LOG(pLog, LogINF, "PaxosSessionBackup Size=%d", Size);
	Ret = PaxosSessionBackup( pAcceptor, pIO, pMsgBackup, Size );
	if (Ret < 0) {
		LOG(pLog, LogERR, "PaxosSessionBackup Ret=%d", Ret );
		goto err2;
	}

	/*
	 *	PAXOS_SESSION_TRANSFER$B%3%^%s%I$NAw?.$H<u?.(B
	 *		0	OK
	 *		>=1	again
	 *		-1	error
	 */
	LOG(pLog, LogINF, "PaxosSessionTransfer");
	RetAgain = PaxosSessionTransfer( pAcceptor, pIO );
	if (RetAgain < 0) {
		LOG(pLog, LogERR, "PaxosSessionTransfer RetAgain=%d", RetAgain );
		goto err2;
	}

	TimerCancel( &AutonomicTimer, pE );

	if( pAcceptor->c_IF.if_fRestore ) {
		LOG(pLog, LogINF, "RestoreOB RetAgain=%d", RetAgain );
		Ret = (pAcceptor->c_IF.if_fRestore)();
		if( Ret ) {
			LOG(pLog, LogERR, "RestoreOB Ret=%d", Ret );
			goto err1;
		}
	}
	if( RetAgain >= 1 ) {
		LOG(pLog, LogINF, "SessionUnlock RetAgain=%d", RetAgain );
		PaxosSessionUnlock( pSession, pH );
		t2 = time(NULL);
		time_t te = t2 - t1;
		if ( te < 1 ) {
			te = 1;
		}
		else if ( te > 30 ) {
			te = 30;
		}
		if ( te < 15 ) {
			if (Size < (INT_MAX / 1.2)) {
				Size *= 1.2;
			}
		}
		else {
			if (Size > BACKUP_SIZE_ONCE) {
				Size *= 0.8;
			}
		}
		LogDump( pLog );
		LOG(pLog, LogINF, "usleep(%d)", te * 1000000 / 10 );
		usleep(te * 1000000 / 10);
		goto AgainIncremental;
	}

	pE = TimerSet( &AutonomicTimer, TIMEOUT_SNAP,
				PaxosSnapshotAbort, (void*)pAcceptor );

	/* order marshal */
	SESSION_MSGINIT( &Req, PAXOS_SESSION_MARSHAL, 0, 0, sizeof(Req) );

	LOG(pLog, LogINF, "IOWrite");
	IOWrite( pIO, &Req, sizeof(Req) );

	/* unmarshal */
	LOG(pLog, LogINF, "PaxosUnmarshal");
	PaxosUnmarshal( PaxosAcceptorGetPaxos(pAcceptor), pIO );
	*pNextDecide = PaxosGetNextDecide( pAcceptor->c_pPaxos );

	if( pAcceptor->c_IF.if_fGlobalUnmarshal )
		(pAcceptor->c_IF.if_fGlobalUnmarshal)( pIO );

	PaxosAcceptorUnmarshal( pIO, pAcceptor );

	/* order autonomic */
	SESSION_MSGINIT( &Req, PAXOS_SESSION_AUTONOMIC, 0, 0, sizeof(Req) );
	Req.h_From	= PaxosMyId( PaxosAcceptorGetPaxos(pAcceptor) );

	LOG(pLog, LogINF, "AUTONOMIC");
	IOWrite( pIO, &Req, sizeof(Req) );

	TimerCancel( &AutonomicTimer, pE );

	TimerStop( &AutonomicTimer );
	TimerDestroy( &AutonomicTimer );

	LOG( pLog, LogINF, "PaxosSessionUnlock");
	PaxosSessionUnlock( pSession, pH );

	LOG(pLog, LogINF, "PaxosSnapshotValidate");
	if( PaxosSnapshotValidate( PaxosAcceptorGetPaxos(pAcceptor) ) )	goto err;
	return( 0 );
err2:
	TimerCancel( &AutonomicTimer, pE );
err1:
	TimerStop( &AutonomicTimer );
	TimerDestroy( &AutonomicTimer );
	LOG( pLog, LogINF, "PaxosSessionUnlock");
	PaxosSessionUnlock( pSession, pH );
err:
	LOG( pLog, LogINF, "PaxosSessionUnlock");
	LogDump( pLog );
	return( -1 );	
}

int
PaxosSessionExport( PaxosAcceptor_t *pAcceptor,
				struct Session *pSession, int j, IOReadWrite_t *pW )
{
	IOSocket_t	IO;
	IOReadWrite_t	*pIO = (IOReadWrite_t*)&IO;
	void	*pH;
	int	Ret;
	char	Buff[1024*4];
	int		n, Len;
	PaxosSessionHead_t	Req;

	Ret = PaxosSessionLock( pSession, j, &pH );
	if( Ret )	goto err;

	IOSocketBind( &IO, PNT_INT32(pH) );

	PaxosSessionBackup( pAcceptor, pIO, NULL, BACKUP_SIZE_ONCE );

	/* order marshal */
	SESSION_MSGINIT( &Req, PAXOS_SESSION_MARSHAL, 0, 0, sizeof(Req) );
	IOWrite( pIO, &Req, sizeof(Req) );

	while( IORead(pIO, &Len, sizeof(Len)) > 0 ) {
		Ret = IOWrite( pW, &Len, sizeof(Len) );	
		if( Len <= 0 )	break;
		while( Len > 0 ) {
			n	= Len > sizeof(Buff) ? sizeof(Buff) : Len;
			n = IORead( pIO, Buff, n );
			IOWrite( pW, Buff, n );
			Len	-= n;
		}
	}
	PaxosSessionTransfer( pAcceptor, pIO );

	PaxosSessionUnlock( pSession, pH );

	return( 0 );
err:
	return( -1 );
}

int
PaxosSessionImport( PaxosAcceptor_t *pAcceptor, IOReadWrite_t* pIO )
{
	int	Ret;

	PaxosUnmarshal( pAcceptor->c_pPaxos, pIO );

	if( pAcceptor->c_IF.if_fGlobalUnmarshal )
		(pAcceptor->c_IF.if_fGlobalUnmarshal)( pIO );

	PaxosAcceptorUnmarshal( pIO, pAcceptor );

	if( pAcceptor->c_IF.if_fRestore ) {
		Ret = (pAcceptor->c_IF.if_fRestore)();
		if( Ret )	goto err;
	}
	if( PaxosSnapshotValidate( pAcceptor->c_pPaxos ) ) {
		DBG_PRT("PaxosSnapshotValidate ERROR\n");
		goto err;
	}
	return( 0 );
err:
	return( -1 );
}

struct PaxosAcceptor*
PaxosAcceptGetAcceptor( PaxosAccept_t *pAccept )
{
	return( pAccept->a_pAcceptor );
}

int
PaxosAcceptorFdEventAdd( PaxosAcceptor_t *pAcceptor, 
						uint64_t Key64, FdEvent_t *pEv, int fd )
{
	PaxosAcceptorFdEvent_t	*pEv1;
	int	Ret;

	pEv1 = (PaxosAcceptorFdEvent_t*)Malloc( sizeof(*pEv1) );
	memset( pEv1, 0, sizeof(*pEv1) );

	pEv1->e_FdEvent	= *pEv;
	pEv1->e_FdEvent.fd_fHandler	= AcceptorRecvHandler;

	pEv1->e_FdEvent.fd_Fd	= fd;
	IOSocketBind( &pEv1->e_IOSock, fd );


	Ret = FdEventAdd( &pAcceptor->c_FdEvent.f_Ctrl, Key64, &pEv1->e_FdEvent );
	LOG( PaxosAcceptorGetLog( pAcceptor ), LogINF, "Ret=%d, fd=%d, pEv=%p", Ret, pEv1->e_FdEvent.fd_Fd, &pEv1->e_FdEvent );
	return( Ret );
/*
 *	pEV1$B$O!"(Bdisconnect$B$G(BFree$B$5$l$F$$$k$N$GCm0U(B
 */
}

int
PaxosAcceptorFdEventDel( PaxosAcceptor_t *pAcceptor, FdEvent_t *pEv )
{
	int	Ret;

	Ret = FdEventDel( pEv );
	LOG( PaxosAcceptorGetLog( pAcceptor ), LogINF, "Ret=%d, fd=%d, pEv=%p", Ret, pEv->fd_Fd, pEv );
	return( Ret );
}

int
PaxosAcceptorFreeze( PaxosAcceptor_t *pAcceptor, PaxosAcceptorFdEvent_t *pEv )
{
	SLOG( pAcceptor, LogSYS, "ENT" );
againlock:
	SLOG( pAcceptor, LogDBG, "LOCK" );
	PaxosFreeze(PaxosAcceptorGetPaxos(pAcceptor));	// Paxos
	FdEventSuspend( &pAcceptor->c_FdEvent.f_Ctrl );
	if( MtxTrylock( &pAcceptor->c_Mtx ) ) {			// Session
		FdEventResume( &pAcceptor->c_FdEvent.f_Ctrl, NULL );
		PaxosDefreeze(PaxosAcceptorGetPaxos(pAcceptor));
		usleep( 20*1000 );
		goto againlock;
	}
	FdEventResume( &pAcceptor->c_FdEvent.f_Ctrl, &pEv->e_FdEvent );
	SLOG( pAcceptor, LogSYS, "EXT" );
	return( 0 );
}

int
PaxosAcceptorDefreeze( PaxosAcceptor_t *pAcceptor )
{
	SLOG( pAcceptor, LogSYS, "ENT" );
	MtxUnlock( &pAcceptor->c_Mtx );
	FdEventResume( &pAcceptor->c_FdEvent.f_Ctrl, NULL );
	PaxosDefreeze(PaxosAcceptorGetPaxos(pAcceptor));	// Paxos
	SLOG( pAcceptor, LogSYS, "EXT" );
	return( 0 );
}

int
AcceptorRecvHandler( FdEvent_t *pE, FdEvMode_t Mode )
{
	PaxosAcceptorFdEvent_t	*pEv = container_of( pE, 
										PaxosAcceptorFdEvent_t, e_FdEvent );
	PaxosAcceptor_t	*pAcceptor = (PaxosAcceptor_t*)pEv->e_FdEvent.fd_pArg;
	PaxosSessionHead_t	*pH;
	Msg_t		*pMsg;
	MsgVec_t	Vec;
	int		Ret = 0;
	int	No;

	DBG_ENT();

	if( Mode != EV_LOOP )	return( 0 );

	No	= pEv->e_IOSock.s_Fd % pAcceptor->c_Workers;
	pH = PaxosAcceptorRecvMsg( pAcceptor, pEv->e_IOSock.s_Fd );

	DBG_TRC("pH=%p fd_errno=%d\n", pH, pE->fd_errno );

	pMsg = MsgCreate( 1, pAcceptor->c_IF.if_fMalloc, 
						pAcceptor->c_IF.if_fFree );
	pMsg->m_pTag0	= pEv;

	if( !pH ||  pE->fd_errno ) {

		SLOG( pAcceptor, LogERR, "DEL:pEv=%p fd_Fd=%d", 
			pEv, pEv->e_FdEvent.fd_Fd );
		/* remove from epoll_wait */
		PaxosAcceptorFdEventDel( pAcceptor, &pEv->e_FdEvent );

		QueuePostEntry( &pAcceptor->c_aMsg[No], pMsg, m_Lnk );
	} else {

		DBG_TRC("pH=%p h_Cmd=0x%x h_Len=%d\n", pH, pH->h_Cmd, pH->h_Len );

		MsgVecSet( &Vec, VEC_MINE, pH, pH->h_Len, pH->h_Len );
		MsgAdd( pMsg, &Vec );

		ASSERT( pMsg );
		switch( pH->h_Cmd ) {

			case PAXOS_SESSION_OB_PUSH:
				pMsg->m_aVec[0].v_Flags &= ~VEC_MINE;
				MsgDestroy( pMsg );

				PaxosAcceptorOutOfBandPush( pAcceptor, pH );
				break;

			case PAXOS_SESSION_OB_PULL:
				PaxosAcceptorOutOfBandPull( pAcceptor, pH, pEv->e_IOSock.s_Fd );
				MsgDestroy( pMsg );
				break;

			default:
				QueuePostEntry( &pAcceptor->c_aMsg[No], pMsg, m_Lnk );
				break;
		}
	}
	DBG_EXT( Ret );
	return( Ret );
}

int
AcceptorAcceptHandler( FdEvent_t *pEv, FdEvMode_t Mode )
{
	int ListenFd = pEv->fd_Fd;
	PaxosAcceptor_t	*pAcceptor = (PaxosAcceptor_t*)pEv->fd_pArg;
	int	fd;
	struct linger	Linger = { 1/*On*/, 10/*sec*/};
	struct sockaddr_in	Addr;
	socklen_t	len;
	uint64_t	U64;

	DBG_ENT();

	if( Mode != EV_LOOP )	return( 0 );

	memset( &Addr, 0, sizeof(Addr) );
	len	= sizeof(Addr);

	fd	= accept( ListenFd, (struct sockaddr*)&Addr, &len );
	if( fd < 0 )	goto err;

	U64	= fd;

	/* KEEP_ALIVE$B$O@_Dj$5$l$F$$$k(B */
	/* Set Linger */
	setsockopt( fd, SOL_SOCKET, SO_LINGER, &Linger, sizeof(Linger) );
#ifdef	ZZZ
#else
	int	flag = 1;
	setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
#endif
	
	PaxosAcceptorFdEventAdd( pAcceptor, U64, pEv, fd );
	SLOG( pAcceptor, LogINF, "accept INET=%s", inet_ntoa(Addr.sin_addr) );

	DBG_EXT( 0 );
	return( 0 );
err:
	DBG_EXT( -1 );
	return( -1 );
}

PaxosSessionCell_t*
PaxosAcceptorGetCell( PaxosAcceptor_t *pAcceptor )
{
	return( pAcceptor->c_pSessionCell );
}

int
PaxosAcceptorSync( PaxosAcceptor_t *pAcceptor, int target,uint32_t *pNextDecide )
{
	struct	Session *pSession;
	int		Ret;

	pSession	= PaxosSessionAlloc( Connect, Shutdown, CloseFd, GetMyAddr,
					Send, Recv, Malloc, Free, 
					pAcceptor->c_pSessionCell->c_Name );
	if( !pSession ) {
			errno = ENOMEM;
			goto err;
	}
	PaxosSessionSetLog( pSession, PaxosAcceptorGetLog(pAcceptor) );

	Ret = PaxosSessionGetSnapshot( pAcceptor, pSession, target, pNextDecide );
	if( Ret )	goto err1;

	PaxosSessionFree( pSession );

	return( 0 );
err1:
	PaxosSessionFree( pSession );
err:
	return( -1 );
}

int
PaxosAcceptorStartSync( PaxosAcceptor_t* pAcceptor,
	PaxosStartMode_t StartMode, uint32_t nextDecide,
	int(*fInitSync)(int target) )
{
	int	Ret;
	PaxosActive_t	Summary;

	Ret = PaxosIsActiveSummary( pAcceptor->c_pSessionCell->c_Name, 
					pAcceptor->c_MyId, &Summary );

	if( Ret >= 0 ) {	// synchronize

		SLOG( pAcceptor, LogINF, "Target=%d NextDecide=%u", 
			Summary.a_Target, Summary.a_NextDecide );

		if( StartMode == PAXOS_SEQ ) {
			if( CMP( nextDecide, Summary.a_NextDecide ) < 0 ) {

				if( fInitSync ) {
					Ret = (*fInitSync)( Summary.a_Target );
					if( Ret < 0 )	goto err;
				}
				Ret = PaxosAcceptorSync( pAcceptor, Summary.a_Target, &nextDecide );
				if( Ret < 0 )	goto err;
			}
		} else {

			if( fInitSync ) {
				Ret = (*fInitSync)( Summary.a_Target );
				if( Ret < 0 )	goto err;
			}
			Ret = PaxosAcceptorSync( pAcceptor, Summary.a_Target, &nextDecide );
			if( Ret < 0 )	goto err;
		}
	} else {
		if( StartMode == PAXOS_WAIT || StartMode == PAXOS_NO_SEQ ) {
			errno	= ENOENT;
			SLOG( pAcceptor, LogERR, "No Target StartMode=%d", StartMode ); 
			goto err;
		}
	}
	Ret = PaxosAcceptorStart( pAcceptor, StartMode, nextDecide );
	if (Ret < 0) {
		SLOG( pAcceptor, LogERR, "Ret=%d", Ret ); 
	}

	return( Ret );
err:
	SLOG( pAcceptor, LogERR, "Ret=%d", Ret ); 
	return( -1 );
}
