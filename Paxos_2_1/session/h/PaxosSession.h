/******************************************************************************
*
*  PaxosSession.h 	--- header of Paxos-Session Library
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
 *	$B;n83(B			$B1v?,>o=U(B,$B5WJ]@56H(B
 *	$BFC5v!"Cx:n8"(B	$B%H%i%$%F%C%/(B
 *					$BFC4j(B2010-538664(PCT/JP2010/066906)
 *
 *
 *	$B35MW(B
 *	 Paxos$B$KDL?.$H(Bsession/event$B4IM}$rIUM?$7$?(B
 *	$B$^$?!";2>H$NJ,;65!G=$bAuHw$7$F$$$k(B
 *	Timer$B5!G=$bM-$7$F$$$k(B
 *	 
 *	$BDL?.$H(Bmaster$B@\B3(B
 *	 $BDL?.@\B38e!"%/%i%$%"%s%H(BId$B$K$h$kDL?.C<E@:n@.$r%5!<%P$K;X<($9$k(B
 *	$B$I$N%5!<%P$H$b@\B3$,$G$-$J$$>l9g$OCf;_$9$k(B
 *	$B0l$D$G$b@\B3$,$"$l$P(Bmaster$B$,8+$D$+$k$^$G1J1s$K:F;n9T$9$k(B
 *	master$B$N@Z$jBX$(;~$bF1MM$G$"$k(B
 *	 AP$B$,B>$N=hM}Cf$K(Bmaster$B@Z$jBX$($,H/@8$9$k(B
 *	$B%5!<%P$O%/%i%$%"%s%H$K%$%Y%s%H$rH/9T$9$k(B
 *	$B%/%i%$%"%s%H$OD>$A$K?7(Bmaster$B$K@\B3$7!"C<E@$,4"$j<h$i$l$J$$$h$&$K$9$k(B
 *	$B=>$C$F%/%i%$%"%s%H$O(Bmaster$B$H>o$K@\B3$7$F$$$k(B
 *
 *	master$BITDj>uBV(B(Jeopard)
 *	 master$B8rBX;~$K$O!"ITL@$"$k$$$O?7$,5l$r!"5l$,?7$rDLCN$9$k>uBV$,$"$k(B
 *	$B%/%i%$%s%H$O$3$N$h$&$J>uBV$r8!CN$7$?$i%;%k$+$i$NDLCN$rBT$D(B
 *	$B0lDj;~4V7P$C$F$bDLCN$,$J$$>l9g$O!"(Bmaster$BC5$7$r9T$&(B
 *
 *	$B%;%C%7%g%s4IM}(B
 *	 master$B@\B38e(Bsession$B3+;O$rDLCN$9$k(B
 *	$BK\DLCN$O(BPaxos$B$G(Bagent$B$KEA$($i$l(Bagent$B$O(Bsession$B>uBV$r;}$D(B
 *	agent$B$O%/%i%$%"%s%H(BId$B$r%-!<$H$7$F%;%C%7%g%s$r:n@.$9$k(B
 *	$B$I$N(Breplica$B$N(Bsession$B$b1~Ez$r4^$a$?F10l$N>uBV$r;}$D(B
 *	TCP$B$K$h$k(BVC$B$G(Bserver,client$B$G@\B34F;k$r9T$&(B
 *	master$B$G$O@ZCG;~$K$O(Bsession$BCG$HH=CG$9$k(B
 *	client$B$G$O(Bmaster$BCG$HH=CG$70lDj;~4V8e@\B3$9$k(B
 *	$B?7(Bmaster$B$H$J$C$?%5!<%P$O0lDj;~4V8e(B
 *	session$B>uBV$N4"$j<h$j$r9T$&(B
 *
 *	$B%/%i%$%"%s%H(BId
 *	 (IN$B%"%I%l%9!"%W%m%;%9(BId$B!"%f!<%6IUM?HV9f!"DLHV!"D>6a99?7DLHV(B)$B$H$7$F$$$k(B
 *	$BDL?.C<E@!"%;%C%7%g%s$O(B(IN$B!"%W%m%;%9(BId, $B%f!<%6IUM?HV9f(B)$B$G<1JL$9$k(B
 *	$B%H%i%s%6%/%7%g%s$ODLHV!"D>6a99?7DLHV$r4^$`(B
 *
 *	$B=g=x4IM}$H=EJ#(B
 *	 $B%;%C%7%g%sKh$K=g=x4IM}$r9T$&(B
 *	$B%5!<%P$KBP$9$k=EJ#MW5a$O(Bagent$B$G9T$&(B
 *	Paxos$B$K$h$j$I$N%5!<%P$G$b(BPaxos$B=g=x$,J]>Z$5$l$k(B
 *	$B=>$C$F!"(Bmaster$B@Z$jBX$($G$bDI$$1[$7$,H/@8$7$J$$(B
 *	$BMW5aDLHV$,(Bsession$B$N1~EzDLHV$G$"$l$P1~Ez$rJV$9(B
 *	$BMW5aDLHV$,(Bsession$B$N<uM}DLHV$G$"$l$P=hM}Cf$J$N$G2?$b$7$J$$(B
 *	$B<uM};~!"(Bsession$B$N=hM}:Q$_DLHV$,D>6a99?7DLHV$h$j>.$5$$$H$-$OBT$D(B
 *
 * VC$B$NBT$A4IM}$O!"F0E*@\B3@ZCG$,$"$k$N$G%9%l%C%I$H$7$F$$$k(B
 */

#ifndef	_PAXOS_SESSION_
#define	_PAXOS_SESSION_

#include	"Paxos.h"

#include	<netdb.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<fcntl.h>

/*
 *	Default values
 */
#define	DEFAULT_LOG_LEVEL	LogINF

#define	DEFAULT_CORE		3
#define	DEFAULT_EXTENSION	1
#define	DEFAULT_CKSUM_ON	1
#define	DEFAULT_FD_MAX		20
#define	DEFAULT_BLOCK_MAX	2000
#define	DEFAULT_SEG_SIZE	(4*1024)
#define	DEFAULT_SEG_NUM		(16)
#define	DEFAULT_WB_USEC		(1000*100)	// 100 ms

#define	DEFAULT_PAXOS_USEC	(3*1000*1000LL)	// 3 sec
#define	DEFAULT_OUT_MEM_MAX	(1024*1024*100)	// 100 MB

#define	DEFAULT_WORKERS	5

#define	PAXOS_SESSION_ADMIN_PORT_FMT	"/tmp/PAXOS_SESSION_ADMIN_PORT_%s_%d"
#define	EVENT_MAX		200

#define	TIMEOUT_SNAP	30*60*1000	// ms
#define	RETRY_MAX		20

#define	SESSION_FAMILY	0x0

/*
 *	Error
 */
#define	ERR_SESSION_RETRY_MAX	(SESSION_FAMILY|1)
#define	ERR_SESSION_NOT_FOUND	(SESSION_FAMILY|2)
#define	ERR_SESSION_MINORITY	(SESSION_FAMILY|3)

/*
 *	API
 */
typedef PaxosCell_t	PaxosSessionCell_t;

#define	PAXOS_SESSION_FAMILY_MASK	0xffff0000

typedef enum PaxosCmd {
	PAXOS_SESSION_BIND,
	PAXOS_SESSION_ELECTION,
	PAXOS_SESSION_OPEN,
	PAXOS_SESSION_CLOSE,

	PAXOS_SESSION_BACKUP,
	PAXOS_SESSION_TRANSFER,
	PAXOS_SESSION_MARSHAL,

	PAXOS_SESSION_ABORT,
	PAXOS_SESSION_EVENT,

	PAXOS_SESSION_OB_PUSH,
	PAXOS_SESSION_OB_PULL,

	PAXOS_SESSION_LOCK,
	PAXOS_SESSION_ANY,
	PAXOS_SESSION_UNLOCK,

	PAXOS_SESSION_CLIENT_ABORT,

	PAXOS_SESSION_SHUTDOWN,

#define	DT_SESSION_HEALTH		10	// sec
#define	DT_SESSION_HEALTH_MS	(DT_SESSION_HEALTH*1000)	//msec

	PAXOS_SESSION_HEALTH,
	PAXOS_SESSION_ACCEPTOR,
	PAXOS_SESSION_DUMP,
	PAXOS_SESSION_AUTONOMIC,
	PAXOS_SESSION_DATA,

	PAXOS_SESSION_CHANGE_MEMBER,
	PAXOS_SESSION_GET_MEMBER

} PaxosCmd_t;

typedef struct PaxosClientId {
	struct sockaddr_in	i_Addr;
	pid_t				i_Pid;
	int					i_No;
	time_t				i_Time;
	uint32_t			i_Reuse;
	uint32_t			i_Seq;
	uint32_t			i_Try;
	uint32_t			i_SeqWait;
} PaxosClientId_t;

typedef struct PaxosSessionHead {
	uint64_t		h_Cksum64;
	PaxosCmd_t		h_Cmd;
	int				h_Len;
	PaxosClientId_t	h_Id;
	int				h_Error;
	PaxosNotify_t	h_Code;
	int				h_From;
	int				h_Master;
	uint32_t		h_Epoch;
	int64_t			h_TimeoutUsec;
	int64_t			h_UnitUsec;
	int				h_EventCnt;
	int				h_EventOmitted;
	int				h_Reader;
	uint32_t		h_Ver;
} PaxosSessionHead_t;

#define	SESSION_MSGINIT( pF, cmd, code, error, len ) \
	({PaxosSessionHead_t*p=(PaxosSessionHead_t*)(pF); \
		memset(p,0,sizeof(PaxosSessionHead_t)); \
		/*list_init(&p->h_Paxos.p_Lnk);*/p->h_Cmd=cmd;p->h_Code=code; \
		p->h_Error=error;p->h_Len=len;p->h_Master=-1;})

#define	COMMENT_SIZE	1024
typedef struct PaxosSessionShutdownReq {
	PaxosSessionHead_t	s_Head;
	char				s_Comment[COMMENT_SIZE];
} PaxosSessionShutdownReq_t;

typedef	struct PaxosSessionChangeMemberReq {
	PaxosSessionHead_t	c_Head;
	int					c_Id;
	struct sockaddr_in	c_Addr;
} PaxosSessionChangeMemberReq_t;

typedef	struct PaxosSessionChangeMemberRpl {
	PaxosSessionHead_t	c_Head;
} PaxosSessionChangeMemberRpl_t;

typedef	struct PaxosSessionGetMemberReq {
	PaxosSessionHead_t	m_Head;
} PaxosSessionGetMemberReq_t;

typedef	struct PaxosSessionGetMemberRpl {
	PaxosSessionHead_t	m_Head;
	PaxosSessionCell_t	m_Cell;
} PaxosSessionGetMemberRpl_t;

typedef	struct PaxosSessionDumpReq {
	PaxosSessionHead_t	d_Head;
	void*			d_pStart;
	int				d_Size;
} PaxosSessionDumpReq_t;

typedef	struct PaxosSessionDumpRpl {
	PaxosSessionHead_t	d_Head;
	char			d_Data[0];
} PaxosSessionDumpRpl_t;

/*
 *	Client API
 */
/*
 * $B;HMQJ}K!(B	
 *	PaxosSessionSize		Session$B9=B$BN$N%5%$%:$N<hF@(B
 *	PaxosSessionInit		Session$B9=B$BN$N=i4|2=(B
 *		fConnect				$B@\B34X?t(B
 *			Id						$B%5!<%P(BId(0 <= Id< PAXOS_SERVER_MAX)
 *		fShutdown				$B@ZCG4X?t(B
 *			pHandle					$B%O%s%I%k(B
 *			How						$BJ}K!(B
 *		fClose					close$B4X?t(B
 *			pHandle					$B%O%s%I%k(B
 *		fGetMyAddr				$B<+%"%I%l%9<hF@4X?t(B
 *			pHandle					$B%O%s%I%k(B
 *			pAddr					$B%"%I%l%9(B
 *		fSendTo					$BAw?.4X?t(B
 *			pHandle					$B%O%s%I%k(B
 *			pBuff					$B%P%C%U%!(B
 *			Len						$BD9$5(B
 *			Flags					$B%U%i%0(B	
 *		fRecvFrom				$B<u?.4X?t(B
 *			pHandle					$B%O%s%I%k(B
 *			pBuff					$B%P%C%U%!(B
 *			Len						$BD9$5(B
 *			Flags					$B%U%i%0(B	
 *		fMalloc					$B%a%b%j<hF@4X?t(B
 *			Size					$BD9$5(B
 *		fFree					$B%a%b%jJV5Q4X?t(B
 *			pAddr					$B%"%I%l%9(B
 *		pSessionCell				$B%;%k%"%I%l%9(B
 *	PaxosSessionDestroy		Session$B9=B$BN$NGK2u(B
 *	PaxosSessionGetCell		$B%;%k%"%I%l%9$N<hF@(B
 *	PaxosSessionOpen		Session$B$N3+;O(B
 *		No						$B%f!<%6IUM?HV9f(B
 *		Sync					$BF14|%b!<%I(B($B;2>H$H99?7$NF14|(B)
 *	PaxosSessionClose		Session$B$N=*N;(B
 *	PaxosSessionSendToCell	Cell$B$X$N%G!<%?Aw?.(B
 *		N						$B%Y%/%?$N?t(B
 *		aVec					$B%Y%/%?G[Ns(B
 *	PaxosSessionReqAndRplByMsg	$B%H%i%s%6%/%7%g%s$NAw<u?.(B
 *		pReq					$BMW5a(B
 *		ppRpl					$B1~Ez(B
 *		Update					$B99?7(B
 *		Forever					$B1J5W$KBT$D(B
 *	PaxosSessionGetMaster	$B%^%9%?!<$N;2>H(B
 *	PaxosSessionGetReader	$B:G>.Ii2Y$N%l%W%j%+(B
 *	PaxosSessionLock		$B%5!<%P!<$N(Block	
 *		Id						$B%5!<%P!<(BId
 *		ppHandle				$BDL?.%O%s%I%k(B
 *	PaxosSessionUnlock		$B%5!<%P!<$N(Bunlock	
 *		pHandle					$BDL?.%O%s%I%k(B
 *	PaxosSessionAny			$BFC<l%a%C%;!<%8$NAw<u?.(B
 *		pHandle					$BDL?.%O%s%I%k(B
 *		pM						$BFC<l%a%C%;!<%8(B
 *	PaxosSessionLogCreate	logger$B$N@8@.(B
 *		Size					$B%P%C%U%!%5%$%:(B
 *		Flags					$B%U%i%0(B
 *		pFile					$B%U%!%$%k9=B$BN(B
 *	PaxosSessionLogDestroy	logger$B$NGK2u(B
 *	PaxosSessionGetLog		logger$B$N;2>H(B
 *	PaxosSessionSetLog		logger$B$N@_Dj(B
 *		pLog					logger
 *	PaxosSessionDumpMem		$B%a%b%j%G!<%?$N<hF@(B	
 *		pStart
 *		Size
 *
 */

#ifdef	_PAXOS_SESSION_CLIENT_
typedef void*	pHandle;

typedef struct Server {
	bool_t				s_Binded;
	time_t				s_CheckLast;
	struct sockaddr_in	s_PeerAddr;
	struct sockaddr_in	s_MyAddr;
	FdEvent_t			*s_pEvent;
} Server_t;

#define	SB_INIT		0x1
#define	SB_ACTIVE	0x2
#define	SB_ISSUED	0x4
#define	SB_CLOSED	0x8
#define	SB_WAIT		0x10

typedef struct Session {
	list_t			s_Lnk;
	Mtx_t			s_Serialize;
	int				s_Err;
	int				s_Status;
	int				s_Master;
	int				s_NewMaster;
	int				s_Reader;
	PaxosClientId_t	s_Id;
	uint32_t		s_Update;
	int64_t			s_MasterElectionUsec;
	int64_t			s_UnitUsec;
	Mtx_t			s_Mtx;
	Cnd_t			s_CndMaster;
	int				s_Max;
	int				s_Maximum;
	Server_t		*s_pServer;		// [PAXOS_SERVER_MAXIMUM];
	int				*s_pJeopard;	// [PAXOS_SERVER_MAX+1];

#define	J_IND( j )	( (j) < 0 ? pSession->s_Max : (j) )
#define	J_SET( j )	pSession->s_pJeopard[J_IND(j)]++
#define	CLEAR_JEOPARD \
	/*LOG(pSession->s_pLog,"CLEAR_JEOPARD");*/ \
	{int ja;for( ja = 0; ja < pSession->s_Max + 1; ja++ ) { \
		if( ja < pSession->s_Max )	pSession->s_pServer[ja].s_CheckLast = 0; \
		pSession->s_pJeopard[ja] = 0; \
	}} 

	Queue_t			s_Post;
	Queue_t			s_Recv;
	pthread_t		s_RecvThread;
	Queue_t			s_Data;
	pHandle			(*s_fConnect)(struct Session*,int j);
	int				(*s_fGetMyAddr)(pHandle,struct sockaddr_in*);
	int				(*s_fShutdown)(pHandle,int);
	int				(*s_fClose)(pHandle);
	int				(*s_fSendTo)(pHandle,char*,size_t,int);
	int				(*s_fRecvFrom)(pHandle,char*,size_t,int );
	void*			(*s_fMalloc)(size_t);
	void			(*s_fFree)(void*);

	PaxosSessionCell_t			*s_pCell;

	struct Log*		s_pLog;
	bool_t			s_Sync;

	void*			s_pVoid;
	time_t			s_NoMaster;
} Session_t;
#else // _PAXOS_SESSION_CLIENT_
struct Session;
#endif // _PAXOS_SESSION_CLIENT_

extern	int PaxosSessionGetSize(void);
extern	struct Session* PaxosSessionAlloc(
			void*(*fConnect)(struct Session*,int Id),
			int(*fShutdown)(void*pHandle,int How),
			int(*fClose)(void*pHandle),
			int(*fGetMyAddr)(void*pHandle,struct sockaddr_in*pAddr),
			int(*fSendTo)(void*pHandle,char*pBuff,size_t Len,int Flags),
			int(*fRecvFrom)(void*pHandle,char*pBuff,size_t Len,int Flags),
			void*(*fMalloc)(size_t Size),
			void(*fFree)(void*pAddr),
			char*pSessionCellName );
extern	int	PaxosSessionFree( struct Session* pSession );

extern	PaxosSessionCell_t* PaxosSessionGetCell( struct Session* pSession );
extern	int PaxosSessionOpen( struct Session* pSession, int No, bool_t Sync );
extern	int PaxosSessionClose( struct Session* pSession );
extern	int PaxosSessionAbort( struct Session* pSession );
extern	struct Session *PaxosSessionFind( char *pCell, int No );

extern	int PaxosSessionSendToCellByMsg( struct Session* pSession, Msg_t*); 
extern	Msg_t* PaxosSessionReqAndRplByMsg( struct Session* pSession,
								Msg_t*, bool_t bUpdate, bool_t bForEver );
extern	Msg_t* PaxosSessionRecvDataByMsg( struct Session* pSession );
extern	int PaxosSessionGetErr( struct Session* pSession ); 

extern	int PaxosSessionGetMaster( struct Session* pSession );
extern	int PaxosSessionGetReader( struct Session* pSession );

extern	int	PaxosSessionLock( struct Session*, int Id, void** ppHandle );
extern	int	PaxosSessionUnlock( struct Session*, void* pHandle );
extern	PaxosSessionHead_t*	PaxosSessionAny( struct Session*, void* pHandle, 
				PaxosSessionHead_t* pM );

extern	int PaxosSessionLogCreate(struct Session*, int Size, int Flags, 
									FILE* pFile, int Level );
extern	void PaxosSessionLogDestroy(struct Session*);
extern	struct Log* PaxosSessionGetLog( struct Session* );
extern	int	PaxosSessionSetLog( struct Session*, struct Log* pLog );

extern	void*	(*PaxosSessionGetfMalloc(struct Session*))(size_t);
extern	void	(*PaxosSessionGetfFree(struct Session*))(void*);

extern	PaxosClientId_t* PaxosSessionGetClientId( struct Session* );
extern	int	PaxosSessionGetMyAddr(struct Session*,
						void*pHandle,struct sockaddr_in*pAddr);
extern	void*	PaxosSessionGetTag( struct Session* );
extern	void	PaxosSessionSetTag( struct Session*, void* );
extern	int		PaxosSessionChangeMember( struct Session *pSession, 
							int Id, struct sockaddr_in *pAddr );

extern	int	PaxosSessionGetMax( struct Session *pSession );
extern	int	PaxosSessionGetMaximum( struct Session *pSession );
extern	int		PaxosSessionDumpMem( struct Session *pSession, void *pHandle,
							void *pRemote, void *pLocal, size_t Length );
/*
 *	Server API
 */
/*
 * $B;HMQJ}K!(B	
 *	PaxosAcceptorSize	Acceptor$B9=B$BN$N%5%$%:$N<hF@(B
 *	PaxosAcceptorInit	Acceptor$B9=B$BN$N=i4|2=(B
 *		Id					$B%5!<%P(BId(0 <= Id< PAXOS_SERVER_MAX)
 *		pSessionCell		session$B%;%k%"%I%l%9(B
 *		pPaxosCell			Paxos$B%;%k%"%I%l%9(B
 *		UnitSec				Paxos$BC10L;~4V(B
 *		pIF					$B%$%s%?!<%U%'!<%94X?t(B
 *			if_fWritePaxos		Paxos$B%9%J%C%W%7%g%C%HMQ(Bwrite$B4X?t(B
 *			if_fReadPaxos		Paxos$B%9%J%C%W%7%g%C%HMQ(Bread$B4X?t(B
 *			if_fMalloc			$B%a%b%j<hF@4X?t(B
 *			if_fFree			$B%a%b%j2rJ|4X?t(B
 *			if_fSessionOpen		session$B3+@_4X?t(B
 *			if_fSessionClose	session$BJD:?4X?t(B
 *			if_fRequest			$BMW5a<uIU4X?t(B
 *			if_fValidate		$B%G!<%?3NG'4X?t(B
 *			if_fAccepted		$B9g0U<B9T4X?t(B
 *			if_fRejected		$BMW5a5qH]4X?t(B
 *			if_fAbort			$B%"%\!<%H4X?t(B
 *			if_fShutdown		shutdown$B4X?t(B
 *			if_fSnapAdaptorRead		$B%"%@%W%?!<(Bread$B4X?t(B
 *			if_fOutOfBandSave		$BBS0h30%G!<%?(Bsave$B4X?t(B
 *			if_fOutOfBandLoad		$BBS0h30%G!<%?(Bload$B4X?t(B
 *			if_fOutOfBandRemove		$BBS0h30%G!<%?:o=|4X?t(B
 *			if_fLock			session$B%m%C%/4X?t(B
 *			if_fUnlock			session$B%"%s%m%C%/4X?t(B
 *			if_fAny				$BFC<l%a%C%;!<%8=hM}4X?t(B
 *			if_fLoad			$BIi2Y7W;;4X?t(B
 *	PaxosAcceptorStart		Acceptor$B$N3+;O(B
 *	PaxosAcceptorSetTag		Adaptor$B$N@_Dj(B
 *		Family					$B%U%!%_%j(B
 *		pTag					Adaptor
 *	PaxosAcceptorGetTag		Adaptor$B$N;2>H(B
 *		Family					$B%U%!%_%j(B
 *	PaxosAcceptIsAccepted	$B<B9T$N%A%'%C%/(B
 *		pId						$B%/%i%$%"%s%H(BID
 *	PaxosAcceptReplyByMsg	$B1~Ez$NAw?.(B($B%^%9%?!<$G$"$l$PAw?.!K(B
 *		pRpl					$B1~Ez(B
 *	PaxosAcceptSend			$B1~Ez$ND>@\Aw?.(B
 *		pRpl					$B1~Ez(B
 *	JOINT					$B%$%Y%s%H$H$N@\B3(B
 *		pW						Adaptor$B$NBT$A%j%s%/(B
 *		pE						$B%$%Y%s%H$N%j%s%/(B
 *		pJ						$B%8%g%$%s%H(B
 *	PaxosAcceptEventRaise	$B%$%Y%s%H$NNe5/(B
 *		pEvent					$B%$%Y%s%H%a%C%;!<%8(B
 *	PaxosAcceptorGetLoad	$B<uIU(BFd$B?t(B($BIi2Y7W;;MQ!K(B
 *	PaxosAcceptorLogCreate	logger$B@8@.(B
 *	PaxosAcceptorLogDestroy	logger$BGK2u(B
 *	PaxosAcceptorGetLog		logger$B;2>H(B
 *	PaxosAcceptorSetLog		logger$B@_Dj(B
 */

#define	PATH_NAME_MAX	512

#define	PAXOS_SESSION_OB_PATH	"DATA"

struct PaxosAcceptor;
struct PaxosAccept;

typedef struct PaxosAcceptorIF {

	void*	(*if_fMalloc)(size_t);
	void	(*if_fFree)(void*);

	int		(*if_fSessionOpen)(struct PaxosAccept*,PaxosSessionHead_t*);
	int		(*if_fSessionClose)(struct PaxosAccept*,PaxosSessionHead_t*);

	int		(*if_fRequest)(struct PaxosAcceptor*,PaxosSessionHead_t*,int);
	int		(*if_fValidate)(struct PaxosAcceptor*,uint32_t,
											PaxosSessionHead_t*,int);
	int		(*if_fAccepted)(struct PaxosAccept*,PaxosSessionHead_t*);

	int		(*if_fRejected)(struct PaxosAcceptor*,PaxosSessionHead_t*);
	int		(*if_fAbort)(struct PaxosAcceptor*,uint32_t);
	int		(*if_fShutdown)(struct PaxosAcceptor*,
						PaxosSessionShutdownReq_t*, uint32_t nextDecide);

	Msg_t*	(*if_fBackupPrepare)(int);
	int		(*if_fBackup)(PaxosSessionHead_t*);
	int		(*if_fTransferSend)(IOReadWrite_t*);
	int		(*if_fTransferRecv)(IOReadWrite_t*);
	int		(*if_fRestore)(void);

	int		(*if_fGlobalMarshal)(IOReadWrite_t*);
	int		(*if_fGlobalUnmarshal)(IOReadWrite_t*);

	int		(*if_fAdaptorMarshal)(IOReadWrite_t*,struct PaxosAccept*);
	int		(*if_fAdaptorUnmarshal)(IOReadWrite_t*,struct PaxosAccept*);

	int		(*if_fLock)(void);
	int		(*if_fAny)(struct PaxosAcceptor*, PaxosSessionHead_t*,int);
	int		(*if_fUnlock)(void);

	int		(*if_fOutOfBandSave)(struct PaxosAcceptor*,PaxosSessionHead_t*);
	PaxosSessionHead_t*
			(*if_fOutOfBandLoad)(struct PaxosAcceptor*,PaxosClientId_t*);
	int		(*if_fOutOfBandRemove)(struct PaxosAcceptor*,PaxosClientId_t*);

	int		(*if_fLoad)(void*);

} PaxosAcceptorIF_t;

typedef	struct PaxosAcceptorFdEvent {
	FdEvent_t				e_FdEvent;
	IOSocket_t				e_IOSock;
	struct PaxosAccept		*e_pAccept;
} PaxosAcceptorFdEvent_t;

typedef struct Joint {
	list_t	j_Wait;
	list_t	j_Event;
} Joint_t;

#define	JOINT_INIT( pJ ) \
	({list_init(&(pJ)->j_Wait);list_init(&(pJ)->j_Event);})

#define	JOINT( pW, pE, pJ ) \
	({list_add(&(pJ)->j_Wait,pW);list_add(&(pJ)->j_Event,pE);})

#define	UNJOINT( pJ ) \
	({list_del_init(&(pJ)->j_Wait);list_del_init(&(pJ)->j_Event);})

extern	void*	(*PaxosAcceptorGetfMalloc(struct PaxosAcceptor*))(size_t);
extern	void	(*PaxosAcceptorGetfFree( struct PaxosAcceptor* ))(void*);
extern	void*	(*PaxosAcceptGetfMalloc( struct PaxosAccept* ))(size_t);
extern	void	(*PaxosAcceptGetfFree( struct PaxosAccept* ))(void*);

extern	int PaxosAcceptorGetSize(void);
extern	int PaxosAcceptorInitUsec( struct PaxosAcceptor *pAcceptor, int Id, 
		int	ServerCore, int ServerExtension, int64_t MemLimit, bool_t bCksum,
		PaxosSessionCell_t	*pSessionCell,
		PaxosCell_t			*pPaxosCell,
		int64_t 			UnitUsec,
		int					Workers,
		PaxosAcceptorIF_t* );

extern	int PaxosAcceptorStartSync( struct PaxosAcceptor* pAcceptor,
				PaxosStartMode_t StartMode, uint32_t nextDecide,
				int (*fInitSync)(int tartget) );
extern	struct Paxos* PaxosAcceptorGetPaxos( struct PaxosAcceptor* pAcceptor );
extern	PaxosSessionCell_t* PaxosAcceptorGetCell(struct PaxosAcceptor* );
extern	int PaxosAcceptorMyId( struct PaxosAcceptor *pAcceptor );
extern	void* PaxosAcceptGetTag( struct PaxosAccept* pAccept, int Family );
extern	void PaxosAcceptSetTag( struct PaxosAccept* pAccept, 
										int Family, void* pTag );
extern	struct PaxosAccept* PaxosAcceptGetToMine( 
								struct PaxosAcceptor* pAcceptor,
								PaxosClientId_t* pClientId,
								int fd );
extern	struct PaxosAccept* PaxosAcceptGet( struct PaxosAcceptor* pAcceptor,
					PaxosClientId_t* pId, bool_t bCreate );
extern	int PaxosAcceptPut( struct PaxosAccept* pAccept );
extern	PaxosClientId_t* PaxosAcceptorGetClientId( 
					struct PaxosAccept* pAccept );
extern	bool_t PaxosAcceptIsAccepted( struct PaxosAccept* pAccept, 
									PaxosSessionHead_t* pId );
extern	bool_t PaxosAcceptIsDupricate( struct PaxosAccept* pAccept, 
									PaxosSessionHead_t* pPSH );
extern	int PaxosAcceptSend( struct PaxosAccept* pAccept, 
									PaxosSessionHead_t* pRplHead );
extern	int PaxosAcceptSendDataByMsg( struct PaxosAccept* pAccept, Msg_t *pM);

extern	int PaxosAcceptReplyByMsg( struct PaxosAccept* pAccept, Msg_t* pMsg );
extern	int PaxosAcceptReplyReadByMsg( struct PaxosAccept* pAccept, 
									Msg_t* pMsg );

extern	TimerEvent_t* 
		PaxosAcceptorTimerSet( struct PaxosAcceptor* pAcceptor, int64_t Ms, 
					int(*fEvent)(TimerEvent_t*), void* pArg );
extern	PaxosSessionHead_t* 
		PaxosAcceptorRecvMsg( struct PaxosAcceptor *pAcceptor, int fd );
extern	void* PaxosAcceptorMain( void* pArg );
extern	struct PaxosAccept* PaxosAcceptOpen( struct PaxosAcceptor* pAcceptor, 
						PaxosSessionHead_t* pH );
extern	int PaxosAcceptOpenReply( struct PaxosAccept* pAccept,
				PaxosSessionHead_t* pH, int Error );
extern	int PaxosAcceptClose( struct PaxosAccept* pAccept,
					PaxosSessionHead_t* pH, int Error );
extern	int PaxosAcceptSnapshotReply( struct PaxosAccept* pAccept,
			PaxosSessionHead_t* pH, int Error );

extern	struct sockaddr_in*	PaxosAcceptorGetAddr( 
						struct PaxosAcceptor* pAcceptor, int j );

extern	int	PaxosAcceptOutOfBandLock(struct PaxosAccept*);
extern	int	PaxosAcceptOutOfBandUnlock(struct PaxosAccept*);

extern	PaxosSessionHead_t*	
			_PaxosAcceptOutOfBandGet(struct PaxosAccept*, PaxosClientId_t*);
extern	int PaxosAcceptorOutOfBandValidate( struct PaxosAcceptor*, uint32_t,
									PaxosSessionHead_t*, int );

extern	int PaxosAcceptedStart( struct PaxosAccept*, PaxosSessionHead_t* );

extern	int	PaxosAcceptHold( struct PaxosAccept* );
extern	int	PaxosAcceptRelease( struct PaxosAccept* );

extern	int PaxosAcceptorGetLoad( struct PaxosAcceptor* );

extern	int PaxosAcceptorLogCreate(struct PaxosAcceptor*, int Size, int Flags, 
								FILE*, int Level);
extern	void PaxosAcceptorLogDestroy(struct PaxosAcceptor*);
extern	struct Log* PaxosAcceptorGetLog( struct PaxosAcceptor* );
extern	int PaxosAcceptorSetLog(struct PaxosAcceptor*, struct Log*);

extern	struct Log* PaxosAcceptGetLog( struct PaxosAccept* );

extern	int	PaxosAcceptEventRaise( struct PaxosAccept *pAccept,
									PaxosSessionHead_t* pEvent );
extern	int	PaxosAcceptEventGet( struct PaxosAccept *pAccept,
									PaxosSessionHead_t* pH );
extern	int PaxosAcceptorUnmarshal( IOReadWrite_t *pR, 
									struct PaxosAcceptor* pAcceptor );
extern	int PaxosSessionExport( struct PaxosAcceptor *pAcceptor,
				struct Session *pSession, int j, IOReadWrite_t *pW );
extern	int PaxosSessionImport( struct PaxosAcceptor *pAcceptor, 
				IOReadWrite_t* pIO );

extern	struct PaxosAcceptor*	 PaxosAcceptGetAcceptor( struct PaxosAccept* ); 
extern	int	PaxosAcceptorFreeze( struct PaxosAcceptor *pAcceptor,
										PaxosAcceptorFdEvent_t *pEv );
extern	int PaxosAcceptorDefreeze( struct PaxosAcceptor *pAcceptor );

#ifdef	_PAXOS_SESSION_SERVER_

/*
 *	Session
 */
typedef struct PaxosAcceptor {
	Mtx_t				c_Mtx;
	int					c_MyId;
	PaxosSessionCell_t	*c_pSessionCell;
	struct Paxos*		c_pPaxos;
	bool_t				c_bCksum;
	GElmCtrl_t			c_Accept;

	struct {
		Mtx_t	o_Mtx;
		list_t	o_Lnk;
		int		o_Cnt;
		int64_t	o_MemLimit;
		int64_t	o_MemUsed;
		Hash_t	o_Hash;
	} 					c_OutOfBand;
	int					*c_pFds;
	Timer_t				c_Timer;
	bool_t				c_Enabled;
	PaxosAcceptorIF_t	c_IF;
	int					c_MasterWas;

	struct {
		FdEventCtrl_t	f_Ctrl;
	} 					c_FdEvent;

	PaxosAcceptorFdEvent_t *c_pFdFreeze;
	int				c_Workers;
	Queue_t			*c_aMsg;

	struct Log*		c_pLog;
} PaxosAcceptor_t;

typedef	struct ThreadAcceptorArg {
	PaxosAcceptor_t	*a_pAcceptor;
	int				a_No;
} ThreadAcceptorArg_t;

typedef enum PaxosAcceptOpen {
	ACCEPT_BINDED	= 0,
	ACCEPT_OPENING	= 1,
	ACCEPT_OPENED	= 2,
	ACCEPT_CLOSING	= 3,
} PaxosAcceptOpen_t;

typedef struct PaxosAccept {
	GElm_t				a_GE;
	Mtx_t				a_Mtx;
	PaxosAcceptorFdEvent_t *a_pFd;
	PaxosClientId_t		a_Id;
	PaxosAcceptOpen_t	a_Opened;
	struct {
		Cnd_t		a_Cnd;
		int32_t		a_Cnt;
		uint32_t	a_Start;	// $BMW5a$r<u$1IU$1$?(B
		uint32_t	a_End;		// $BMW5a$r=hM}$7$?(B
		uint32_t	a_Reply;	// $B1~Ez(B
		bool_t		a_Master;
		int			a_Cmd;
	}					a_Accepted;
	struct {
		int		r_Cnt;
		bool_t	r_Hold;
		list_t	r_Msg;
	}					a_Rpls;
	struct {
		int		e_Cnt;
		int		e_Omitted;
		bool_t	e_Send;
		Msg_t	*e_pRaised;
	} 					a_Events;
	PaxosAcceptor_t*	a_pAcceptor;
	Hash_t				a_Tag;
	time_t				a_Create;
	time_t				a_Access;
	time_t				a_Health;
} PaxosAccept_t;

typedef struct AcceptorOutOfBand {
	list_t				o_Lnk;
	PaxosClientId_t		o_Id;
	PaxosSessionHead_t*		o_pData;
	time_t				o_Last;
	time_t				o_Create;
	bool_t				o_bValidate;
	uint32_t			o_Seq;
	bool_t				o_bSaved;
} AcceptorOutOfBand_t;

#define	SESSION_MSGALLOC( cmd, code, error, len ) \
	({PaxosSessionHead_t* pF; \
		pF = (PaxosSessionHead_t*)(pAcceptor->c_IF.if_fMalloc)( len ); \
		SESSION_MSGINIT( pF, cmd, code, error, len ); pF;})

#define	SESSION_MSGFREE( pF )	(pAcceptor->c_IF.if_fFree)( pF );

#define	ACCEPT_REPLY_MSG( a, m, p, s, l ) \
	({Msg_t		*_pMsgRpl; \
	MsgVec_t	_Vec; \
	int	_ret; \
	_pMsgRpl = MsgCreate( 1,	PaxosAcceptGetfMalloc((a)), \
								PaxosAcceptGetfFree((a))); \
	MsgVecSet( &_Vec, VEC_MINE, (p), (s), (l) ); \
	MsgAdd( _pMsgRpl, &_Vec ); \
	if( m ) _ret = PaxosAcceptReplyByMsg( (a), _pMsgRpl ); \
	else	_ret = PaxosAcceptReplyReadByMsg( (a), _pMsgRpl ); \
	_ret;})

typedef	struct PaxosSessionAcceptorReq {
	PaxosSessionHead_t	a_Head;
} PaxosSessionAcceptorReq_t;

typedef	struct PaxosSessionAcceptorRpl {
	PaxosSessionHead_t	a_Head;
	PaxosAcceptor_t		a_Acceptor;
	PaxosAcceptor_t	*	a_pAcceptor;
} PaxosSessionAcceptorRpl_t;


#endif	// _PAXOS_SESSION_SERVER_

extern	struct in_addr PaxosGetAddrInfo( char *pNode );
extern	PaxosCell_t*	PaxosCellGetAddr( char *pCellName, 
										void*(*fMalloc)(size_t) );
extern	PaxosSessionCell_t*	PaxosSessionCellGetAddr( char *pCellName,
										void*(*fMalloc)(size_t) );

extern	void*	Connect( struct Session* pSession, int j );
extern	int		ConnectAdmPort( char *pName, int Id );
extern	int 	ReqAndRplByAny( int Fd, char* pReq, int ReqLen, 
										char* pRpl, int RplLen );
extern	void*	ConnectAdm( struct Session* pSession, int j );
extern	int		Shutdown( void* pVoid, int how );
extern	int		CloseFd( void* pVoid );
extern	int		GetMyAddr( void* pVoid, struct sockaddr_in* pMyAddr );
extern	int		Send( void* pVoid, char* pBuf, size_t Len, int Flags );
extern	int		Recv( void* pVoid, char* pBuf, size_t Len, int Flags );

#endif	// _PAXOS_SESSION_
