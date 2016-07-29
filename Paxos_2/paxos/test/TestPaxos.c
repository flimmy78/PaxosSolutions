/******************************************************************************
*
*  TestPaxos.c 	--- test program of Paxos Library
*
*  Copyright (C) 2010-2014,2016 triTech Inc. All Rights Reserved.
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

/**
 * $B;n83(B
 *	$B4D6-(B
 *	 PAXOS_SERVER_MAX=5
 *	 PAXOS_MAX=5
 *	 down.sh	$BA4$F$N%5!<%P$r(Bshutdown
 *	 rmperm.sh	$B1JB32=%G!<%?$r:o=|(B($B:G=i$K<B9T(B)
 *	 TestPaxos	$B%F%9%H%W%m%0%i%`(B
 *	 admin		$B4IM}%3%^%s%I(B
 *
 *	1.$BJRJ}8~$NDL?.CG$N8!CN(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)master$BA*Br8e(B
 *			admin 0 stop 1$B$G(B0$B$+$i(B1$B$NAw?.CG(B
 *		[$B7k2L(B]
 *		  master$B$J$7$H$J$k(B
 *	2.$BDL?.2sI|(B
 *		1)admin 0 restart 1$B$G(B0$B$+$i(B1$B$NAw?.2sI|(B
 *		[$B7k2L(B]
 *		  master$B$r9_$j$?;~CB@8$r99?7$9$k$N$G(Bmaster$B$O(B1
 *		  $BDL?.2sI|;~$K(Bmaster$BA*Br$rMp$5$J$$(B
 *	3.round$B3MF@(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		[$B7k2L(B]
 *		 $B0lDj;~4V8e!"(B0$B$,(Bround$B$r3MF@$9$k(B
 *		4)admin 1 newround
 *		[$B7k2L(B]
 *		 1$B$,(Broud$B$r3MF@$9$k$,!"0lDj;~4V8e!"(B0$B$,(Bround$B$r3MF@$9$k(B
 *	4.round$B3MF@ITG=(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)admin 1 recvstop 9 1(COLLECT$B$r<u$1IU$1$J$$(B)
 *		5)admin 0 newround
 *		[$B7k2L(B]
 *			admin 0 paxos$B$G(Bround$B$,3MF@$G$-$J$$$N$r3NG'(B
 *		6)admin 1 newround
 *		[$B7k2L(B]
 *			admin 1 paxos$B$G(Bround$B3MF@(B
 *		7)admin 1 recvstop 9 0(COLLECT$B2r=|(B)
 *		[$B7k2L(B]
 *		 $B<BAu>e$O!"(Bmaster$B$N$_$,(Bround$B$r3MF@$9$k$N$G!"(B
 *		 $B0lDj;~4V8e!"(B0$B$,(Bround$B$r3MF@$9$k(B
 *	4.$B=i4|%G!<%?EjF~%5!<%P$G(Bnewround
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		 0$B$N(Bround$B3MF@$rBT$D(B
 *		4)admin 0 command AAA 0
 *		5)admin 0 command CCC 2
 *		6)admin 0 newround
 *		[$B7k2L(B]
 *			admin 0/1/2 paxos$B$G7hDj$r3NG'(B
 *			CCC$B$O<B9TJ]N1(B
 *		7)admin 0 command BBB 2
 *		8)admin 0 newround
 *		[$B7k2L(B]
 *		 BBB,CCC$B$N=g$K<B9T(B
 *	5.$B@h$K(Bround$B$KEjF~$5$l$?%G!<%?$,M-8z(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)admin 0 recvstop 6 1(ALIVE$B$r<u$1IU$1$J$$(B)
 *		5)admin 1 recvstop 6 1(ALIVE$B$r<u$1IU$1$J$$(B)
 *		6)admin 2 recvstop 6 1(ALIVE$B$r<u$1IU$1$J$$(B)
 *		7)admin 0 recvstop 9 1(COLLECT$B$r<u$1IU$1$J$$(B)
 *		8)admin 1 command AAA 0
 *		9)admin 1 newround(round$B$K:\$;$k(B)
 *		10)admin 0 command BBB 0
 *		11)admin 0 newround
 *		[$B7k2L(B]
 *			AAA$B$O<B9T(B
 *			BBB$B$O5qH](B
 *	6.$B7hDj$N0z$-7Q$.(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		 master$BA*Br8e(B
 *		4)admin 1 recvstop 6 1(ALIVE$B$r<u$1IU$1$J$$(B)
 *		5)admin 1 recvstop 14 1(SUCCESS$B$r<u$1IU$1$J$$(B)
 *		6)admin 2 recvstop 6 1(ALIVE$B$r<u$1IU$1$J$$(B)
 *		7)admin 2 recvstop 14 1(SUCCESS$B$r<u$1IU$1$J$$(B)
 *		8)admin 0 command AAA 0
 *		9)admin 0 newround
 *		[$B7k2L(B]
 *			0$B$G(BAAA$B$O<B9T(B
 *		10)admin 0 down
 *		11)TestPaxos 3
 *		12)admin 3 command BBBB 0
 *		13)admin 3 newround
 *		14)admin 3 newround
 *		[$B7k2L(B]
 *			3$B$G(BAAA$B$O<B9T(B
 *			BBB$B$O5qH](B
 *		15)admin 1 newround
 *		[$B7k2L(B]
 *			1$B$G(BAAA$B$O<B9T(B
 *		16)admin 2 newround
 *		[$B7k2L(B]
 *			2$B$G(BAAA$B$O<B9T(B
 *	7.accept$B8e$N7hDj$O0z$-7Q$,$l$k(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		 master$BA*Br8e(B
 *		4)admin 1 recvstop 14 1(SUCCESS$B$r<u$1IU$1$J$$(B)
 *		5)admin 1 recvstop 8 1(CATCHUP$B1~Ez$r<u$1IU$1$J$$(B)
 *		6)admin 2 recvstop 14 1(SUCCESS$B$r<u$1IU$1$J$$(B)
 *		7)admin 2 recvstop 8 1(CATCHUP$B1~Ez$r<u$1IU$1$J$$(B)
 *		8)0$B$+$i(BAAAA$B$rEjF~(B
 *		[$B7k2L(B]
 *			0$B$G(BAAA$B$O<B9T(B
 *		9)admin 0 down
 *		10)admin 2 command BBBB 0
 *		11)admin 2 recvstop 14 0(SUCCESS$B$r2r=|(B)
 *		12)TestPaxos 3
 *		[$B7k2L(B]
 *		 $B:F5/F08e$N(B2$B$G(BBBBB$B$,5qH]$5$l$k(B
 *		 1,2,3$B$G(BAAAA$B$,<B9T$5$l$k(B
 *	8.collect$B$r<u$1IU$1$J$$(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)admin 1 recvstop 9 1(COLLECT$B$r<u$1IU$1$J$$(B)
 *		4)TestPaxos 2
 *		5)master$B$,A*Br$5$l$kA0$K(B
 *			admin 2 recvstop 9 1(COLLECT$B$r<u$1IU$1$J$$(B)
 *		6)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		[$B7k2L(B]
 *		 timeout$B$G(BPAXOS_REJECTED$B$,JV$5$l$k(B
 *	9.last$B$r<u$1IU$1$J$$(B
 *		1)TestPaxos 0
 *		2)admin 0 recvstop 10 1(LAST$B$r<u$1IU$1$J$$(B)
 *		3)TestPaxos 1
 *		4)TestPaxos 2
 *		5)master$B$,A*Br$5$l$kA0$K(B
 *		6)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		[$B7k2L(B]
 *		 timeout$B$G(BPAXOS_REJECTED$B$,JV$5$l$k(B
 *	10.begin$B$r<u$1IU$1$J$$(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)admin 1 recvstop 12 1(BEGIN$B$r<u$1IU$1$J$$(B)
 *		5)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		[$B7k2L(B]
 *		 master$B$G$"$k8B$j1J1s$K:F;n9T(B 
 *	11.accept$B$r<u$1IU$1$J$$(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)admin 0 recvstop 13 1(ACCEPT$B$r<u$1IU$1$J$$(B)
 *		5)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		[$B7k2L(B]
 *		 master$B$G$"$k8B$j1J1s$K:F;n9T(B 
 *		 admin 0 paxos 1$B$G(Bround$B$,99?7$5$l$F$$$k(B
 *	12.success$B$r<u$1IU$1$J$$(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)admin 1 recvstop 14 1(SUCCESS$B$r<u$1IU$1$J$$(B)
 *		5)admin 1 recvstop 8 1(CATCHUP$B1~Ez$r<u$1IU$1$J$$(B)
 *		6)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		[$B7k2L(B]
 *		 0,2$B$G%3%^%s%I$,<B9T$5$l$k(B
 *		7)admin 1 recvstop 8 0(CATCHUP$B1~Ez(B)
 *		[$B7k2L(B]
 *		 1$B$G%3%^%s%I$,<B9T$5$l$k(B
 *	13.$BM^;_(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)admin 0 command AAAA 0
 *		5)admin 0 suppress 100
 *		6)admin 0 begin 0
 *		[$B7k2L(B]
 *		 master$B$G(Baccept$B$5$l$J$$$H<B9T$5$l$J$$(B
 *		 suppress$B$,2r=|$5$l(Bround$B%A%'%C%/8e!"<B9T$5$l$k(B
 *	14.$BM^;_(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)TestPaxos 3
 *		5)admin 0 command AAAA 0
 *		6)admin 1 suppress 100
 *		7)admin 0 begin 0
 *		[$B7k2L(B]
 *		 $BA4$F$G<B9T$5$l$k(B
 *		 1$B$O(Baccept$B$rJV$5$J$$$,(Bsuccess$B$r<u$1F~$l$k(B
 *		 $BM^;_Cf$G$b%Z!<%899?79T$o$l$k$N$G%Z!<%8:o=|$bM^;_$9$k(B
 *	15.catchup
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		5)admin 0 down
 *		6)TestPaxos 0
 *		[$B7k2L(B]
 *		 0$B$G%3%^%s%I$,:F<B9T$5$l$k(B
 *	16.catchup$BITG=(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		5)0$B$+$i(B1$B%Z!<%8J,EjF~$7!";C$/BT$D(B
 *		6)0$B$+$i(B1$B%Z!<%8J,EjF~(B
 *		7)admin 0 paxos $B$G%-%c%7%e%Z!<%83NG'(B
 *		8)TestPaxos 3
 *		[$B7k2L(B]
 *		 3$B$O=*N;$9$k(B
 *	16.catchup
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)0$B$+$i(B1$B%Z!<%8J,EjF~(B
 *		5)admin 0 removepage
 *		6)admin 1 catchupstop 1
 *		7)admin 2 catchupstop 1
 *		8)TestPaxos 3
 *		[$B7k2L(B]
 *		 $B;C$/BT$C$F$b(B3$B$O(Bcatchup$B$7$J$$(B		 
 *		9)admin 2 catchupstop 0
 *		[$B7k2L(B]
 *		 $B$O(Bcatchup
 *	16.catchup
 *	20.$B<B9T$,(Breplica$B$G<:GT$9$k(B
 *		1)TestPaxos 0
 *		2)TestPaxos 1 1(Seq1$B$G%"%\!<%H(B)
 *		3)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		4)TestPaxos 1
 *		[$B7k2L(B]
 *		 0$B$G%3%^%s%I$,<B9T$5$l$k(B
 *		 1$B$G%3%^%s%I$,:F<B9T$5$l$k(B
 *	21.$B<B9T$,(Bmaster$B$G<:GT$9$k(B
 *		1)TestPaxos 0 1(Seq1$B$G%"%\!<%H(B)
 *		2)TestPaxos 1
 *		3)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		4)TestPaxos 0
 *		[$B7k2L(B]
 *		 1$B$G%3%^%s%I$,<B9T$5$l$k(B
 *		 0$B$G%3%^%s%I$,:F<B9T$5$l$k(B
 *	22.REDIRECT
 *		1)TestPaxos 0
 *		2)admin 0 recvstop 6 1(ALIVE)
 *		3)admin 0 command AAAA 0
 *		4)admin 0 newround
 *		5)TestPaxos 1
 *		6)admin 1 newround
 *		7)admin 1 newround
 *		[$B7k2L(B]
 *		 redirect$B$,H/@8$9$k(B
 *	23.autonomic
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		5)TestPaxos 3 autonomic 0
 *		6)admin 0 paxos
 *		7)admin 3 paxos
 *		[$B7k2L(B]
 *		 $BI|5l$r3NG'(B
 *	24.autonomic
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		5)admin 0 command Vaaa 2	V$B$OBS0h30(B
 *		6)admin 0 newround
 *		7)TestPaxos 3 autonomic 0
 *		8)admin 0 paxos
 *		9)admin 3 paxos
 *		[$B7k2L(B]
 *		 Gap,$BBS0h30%G!<%?$b4^$a$FI|5l$r3NG'(B
 *	24.export/import
 *		1)TestPaxos 0
 *		2)TestPaxos 1
 *		3)TestPaxos 2
 *		4)0$B$+$i%3%^%s%IEjF~(B	0$B$,(Bmaster
 *		5)admin 0 command Vaaaa 2
 *		6)admin 0 newround
 *		7)TestPaxos 0 export EXPORT_0
 *		8)TestPaxos 3 import EXPORT_0
 *		9)admin 0 paxos
 *		10)admin 3 paxos
 *		[$B7k2L(B]
 *		 Gap,$BBS0h30%G!<%?$b4^$a$FI|5l$r3NG'(B
 */

//#define	_DEBUG_

#include	"Paxos.h"

#include	<ctype.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<fcntl.h>

typedef struct Command {
	PaxosHead_t	c_Head;
	char		c_Command[128];
	int			c_Id;
} Command_t;

#define	PRINTF(fmt,...)	printf("[%u] " fmt, (uint32_t)time(0), ##__VA_ARGS__ )

Queue_t	WaitQueue;
int	MyId;

uint32_t	AbortSeq = -1;
/*
 *	Outbound marshal/unmarshal
 */
int
OutboundMarshal( struct Paxos *pPaxos, void *pVoid, IOReadWrite_t *pW )
{
	Command_t	*pCmd = (Command_t*)pVoid;
	int	l;

	l = strlen(pCmd->c_Command);
	IOMarshal( pW, &l, sizeof(l) );
	IOMarshal( pW, pCmd->c_Command, l );

	printf("l=%d [%s]\n", l, pCmd->c_Command );
	return( 0 );
}

int
OutboundUnmarshal( struct Paxos *pPaxos, void *pVoid, IOReadWrite_t *pR )
{
	int	l;
	char Buff[256];

	memset( Buff, 0, sizeof(Buff) );

	IOUnmarshal( pR, &l, sizeof(l) );
	IOUnmarshal( pR, Buff, l );

	printf("l=%d [%s]\n", l, Buff );
	return( 0 );
}

/*
 *	Critical section
 */
int
Election( struct Paxos* pPaxos, PaxosNotify_t Notify, 
			int32_t server, uint32_t epoch, int32_t from )
{
	switch( (int)Notify ) {
	case PAXOS_Im_NOT_MASTER:
		PRINTF("===I'm not Master(%d epoch=%u from=%d)===\n",
								server, epoch, from );
		break;
	case PAXOS_MASTER_ELECTED:
		PRINTF("===Master(%d epoch=%u from=%d) is elected===\n", 
								server, epoch, from );
		break;
	case PAXOS_MASTER_CANCELED:
		PRINTF("===Master(%d epoch=%u from=%d) is canceled===\n", 
								server, epoch, from );
		break;
	case PAXOS_Im_MASTER:
		PRINTF("===I'm Master(%d epoch=%u from=%d)===\n",
								server, epoch, from );
		break;
	}
	return( 0 );
}

int
Notify( struct Paxos* pPaxos, PaxosMode_t Mode, PaxosNotify_t Notify, 
	uint32_t seq, int server, uint32_t epoch, int32_t len, void *pC, int from )
{
	Command_t*	pCommand = (Command_t*)pC;

	DBG_ENT();

	switch( Notify ) {
		case PAXOS_NOOP:
			PRINTF("=== NOOP[%d] ===\n", seq );
			break;
		case PAXOS_REJECTED:
			PRINTF("=== Command[%d %u %d %d %s] === ", 
				Notify, seq, server, pCommand->c_Id, pCommand->c_Command );
			printf("REJECTED\n");
			break;

		case PAXOS_RECOVER:
			PRINTF("=== RECOVER SNAPSHOT(%u) ===\n", seq );
			return( 0 );
		case PAXOS_NOT_GET_PAGE:
			PRINTF("=== NOT_GET_PAGE(%d) ===\n",  seq );
			break;
		case PAXOS_VALIDATE:
			PRINTF("=== VALIDATE(%d %d) ===\n",  Mode, seq );
			break;
		default:
			ABORT();
			break;
	}
	DBG_EXT(0);
	return( 0 );
}

void*
ThreadExec( void* pArg )
{
	struct Paxos* pPaxos = (struct Paxos*)pArg;
	Msg_t	*pMsg;
	PaxosExecute_t*	pPH;
	Command_t* pC;

	while( (pMsg = PaxosRecvFromPaxos( pPaxos )) ) {
		pPH	= (PaxosExecute_t*)MsgFirst( pMsg );
		pC	= (Command_t*)MsgNext( pMsg );
		switch( (int)pPH->e_Notify ) {

			case PAXOS_ACCEPTED:
//				pC = (Command_t*)(pPH+1);	
				PRINTF("=== Seq=%u Command[%s] ===\n", 
					pPH->e_Seq, pC->c_Command );

				if( pPH->e_Seq == AbortSeq )	kill(0, SIGINT );

				if( PaxosIsIamMaster( pPaxos ) ) {
					list_init( &pC->c_Head.p_Lnk );
					QueuePostEntry( &WaitQueue, pC, c_Head.p_Lnk );
					printf("REPLY\n");
				} else {
					printf("NOT REPLY\n");
				}
		}
		MsgDestroy( pMsg );
	}
	return( NULL );
}

void*
ThreadWait( void* pArg )
{
	struct Paxos* pPaxos = (struct Paxos*)pArg;
	Command_t* pC;

	while( 1 ) {
		QueueTimedWaitEntryWithMs( &WaitQueue, pC, c_Head.p_Lnk, 
					PaxosDecidedTimeout(pPaxos)*1000 );
		if( !pC )	printf("TIMEOUT =>");
		else {
			Free( pC );
			printf("=>");
		}
	}
	return( NULL );
}

int	id;
struct Paxos* pPaxos;
PaxosCell_t	*pPaxosCell;

int		AutonomicId	= -1;

void*
ThreadAutonomic( void* pArg )
{
	struct Paxos	*pPaxos = (struct Paxos*)pArg;
	struct sockaddr_in	ListenAddr, From;
	int ListenFd;
	int	len;
	int	Fd;
	IOSocket_t	IO;

	if( !pPaxosCell->c_aPeerAddr[id].a_Active ) {
		DBG_PRT("id=%d is not active.\n", id );
		exit( -1 );
	}
	ListenAddr	= pPaxosCell->c_aPeerAddr[id].a_Addr;
//	ListenAddr.sin_port = htons( id + PAXOS_UDP_PORT_BASE + 10 );

	if( ( ListenFd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
		printf("socket open error.\n");
		exit( -1 );
	}
	if( bind( ListenFd, (struct sockaddr*)&ListenAddr, 
								sizeof(ListenAddr) ) < 0 ) {
		printf("bind error.\n");
		exit( -1 );
	}
	listen( ListenFd, 5 );

	len	= sizeof(From);
loop:
	Fd	= accept( ListenFd, (struct sockaddr*)&From, (socklen_t*)&len );
	if( Fd < 0 ) {
		printf("accept error.\n");
		exit( -1 );
	}
	IOSocketBind( &IO, Fd );
	PaxosFreeze( pPaxos );
	PaxosMarshal( pPaxos, (IOReadWrite_t*)&IO );
	PaxosDefreeze( pPaxos );
	close( Fd );
goto loop;
	return( NULL );
}

int
GetAutonomic( struct Paxos *pPaxos, int j )
{
	int	Fd;
	struct sockaddr_in	ListenAddr;
	IOSocket_t	IO;

	if( ( Fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
		printf("socket open error.\n");
		exit( -1 );
	}
	if( !pPaxosCell->c_aPeerAddr[j].a_Active ) {
		DBG_PRT("j=%d is not active.\n", j );
		exit( -1 );
	}
	ListenAddr	= pPaxosCell->c_aPeerAddr[j].a_Addr;
//	ListenAddr.sin_port = htons( j + PAXOS_UDP_PORT_BASE + 10 );
	if( connect( Fd, (struct sockaddr*)&ListenAddr,
							sizeof(struct sockaddr_in)) < 0 ) {
		close( Fd );
		return( -1 );
	}
	IOSocketBind( &IO, Fd );
	PaxosUnmarshal( pPaxos, (IOReadWrite_t*)&IO );
	PaxosSnapshotValidate( pPaxos );
	close( Fd );
	return( 0 );
}

int
ExportToFile( int j, char *pFile )
{
	IOFile_t	*pIOFile;
	int	Fd;
	struct sockaddr_in	ListenAddr;
	IOSocket_t	IO;
	int32_t	Len, n;
	char	Buff[1024*4];
	IOReadWrite_t	*pR, *pW;
	int	Ret;

	if( ( Fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
		printf("socket open error.\n");
		exit( -1 );
	}
	if( !pPaxosCell->c_aPeerAddr[j].a_Active ) {
		DBG_PRT("j=%d is not active.\n", j );
		exit( -1 );
	}
	ListenAddr	= pPaxosCell->c_aPeerAddr[j].a_Addr;
//	ListenAddr.sin_port = htons( j + PAXOS_UDP_PORT_BASE + 10 );
	if( connect( Fd, (struct sockaddr*)&ListenAddr,
							sizeof(struct sockaddr_in)) < 0 ) {
		close( Fd );
		return( -1 );
	}
	IOSocketBind( &IO, Fd );
	pR	= (IOReadWrite_t*)&IO;

	pIOFile	= IOFileCreate( pFile, O_WRONLY|O_TRUNC, 0, Malloc, Free );
	pW	= (IOReadWrite_t*)pIOFile;

	while( (pR->IO_Read)( pR, &Len, sizeof(Len)) > 0 ) {
DBG_PRT("Len=%d\n", Len );
		Ret = (pW->IO_Write)( pW, &Len, sizeof(Len) );	
		while( Len > 0 ) {
			n	= (pR->IO_Read)( pR, Buff, 
						(Len > sizeof(Buff) ? sizeof(Buff) : Len ) );
			Ret = (pW->IO_Write)( pW, Buff, n );	
			Len -= Ret;
		}
	}

	IOFileDestroy( pIOFile, FALSE );
	close( Fd );
	return( 0 );
}

int
ImportFromFile( struct Paxos *pPaxos, char *pFile )
{
	IOFile_t	*pIOFile;

	pIOFile	= IOFileCreate( pFile, O_RDONLY, 0, Malloc, Free );
	if( !pIOFile ) {
		printf("No file[%s].\n", pFile );
		exit( -1 );
	}
	PaxosUnmarshal( pPaxos, (IOReadWrite_t*)pIOFile );
	PaxosSnapshotValidate( pPaxos );

	IOFileDestroy( pIOFile, FALSE );
	return( 0 );
}

void
usage()
{
	printf("TestPaxos cell id {FALSE|TRUE [seq]}\n");
	printf("TestPaxos cell id abortseq seq\n");
	printf("TestPaxos cell id autonomic id\n");
	printf("TestPaxos cell id export file\n");
	printf("TestPaxos cell id import file\n");
	exit( -1 );
}

#define	SERVER_MAX		3
#define	SERVER_MAXIMUM	4
int
main( int ac, char* av[] )
{
	int	j;
	int	Ret;
	bool_t	bExport = FALSE, bImport = FALSE;
	short	Port;
	PaxosStartMode_t	StartMode = PAXOS_NO_SEQ;
	uint32_t	seq;
	struct in_addr	pin_addr;
//	int	AdminFd;

	if( ac < 3 ) {
		usage();
	}
	id	= atoi(av[2]);
	if( ac >= 4 ) {
		if( !strcmp( av[3], "TRUE" ) ) {
			if( ac >= 5 )	seq	= atoi(av[4]);
			else			seq = 0;
			StartMode	= PAXOS_SEQ;
		} else if( !strcmp( av[3], "FALSE" ) ) {
			StartMode	= PAXOS_WAIT;
		} else if( !strcmp( av[3], "abortseq" ) ) {
			if( ac < 5 )	usage();
			AbortSeq 	= atoi(av[4]);
		} else if( !strcmp( av[3], "autonomic" ) ) {
			if( ac < 5 )	usage();
			AutonomicId 	= atoi(av[4]);
		} else if( !strcmp( av[3], "export" ) ) {
			if( ac < 5 )	usage();
			bExport	= TRUE;
		} else if( !strcmp( av[3], "import" ) ) {
			if( ac < 5 )	usage();
			bImport	= TRUE;
		} else {
			usage();
		}
	}
	pPaxosCell	= PaxosGetCell( av[1] );
	for( j = 0; j < pPaxosCell->c_Maximum; j++ ) {
        pin_addr	= pPaxosCell->c_aPeerAddr[j].a_Addr.sin_addr;
        Port	= pPaxosCell->c_aPeerAddr[j].a_Addr.sin_port;
		Port	= ntohs( Port );

		printf("name[%s] id=%d ip[%s:%d]\n",
				pPaxosCell->c_aPeerAddr[j].a_Host, j, 
				inet_ntoa(pin_addr), Port );
	}
/*
 *	Export
 */
	if( bExport ) {
		ExportToFile( id, av[3] );
		return( 0 );
	}
/*
 *	Initialization & Recover
 */
	QueueInit( &WaitQueue );

	pPaxos = (struct Paxos*)Malloc( PaxosGetSize() );

	PaxosInitUsec( pPaxos, id,
		SERVER_MAX, SERVER_MAXIMUM-SERVER_MAX, pPaxosCell, 
		10*1000*1000, TRUE,
	 	Election, Notify, 
		OutboundMarshal, OutboundUnmarshal,
		NULL, NULL, NULL );

	PaxosLogCreate( pPaxos, 0/*1024*/, 0/*LOG_FLAG_BINARY*/, stderr, LogDBG );

	MyId	= PaxosMyId( pPaxos );

	if( AutonomicId >= 0 )	{
		GetAutonomic( pPaxos, AutonomicId );
	}
	if( bImport ) {
		ImportFromFile( pPaxos, av[3] );
	}
	LogDump( PaxosGetLog( pPaxos ) );
/*
 * 	Start
 */
	char	buff[128];
	Command_t*	pC;
	pthread_t	th;

	Ret = pthread_create( &th, NULL, ThreadExec, (void*)pPaxos );
	Ret = pthread_create( &th, NULL, ThreadWait, (void*)pPaxos );
	Ret = pthread_create( &th, NULL, ThreadAutonomic, (void*)pPaxos );
	if( PaxosStart( pPaxos, StartMode, seq ) < 0 ) {
		perror("PaxosStart error");
		exit( -1 );
	}

	printf("=>");
	while( gets( buff ) ) {
		pC	= (Command_t*)Malloc( sizeof(*pC) );
		dlInit( &pC->c_Head.p_Lnk );
		pC->c_Id	= MyId;
		strcpy( pC->c_Command, buff );

		Ret = PaxosRequest( pPaxos, sizeof(*pC), pC, FALSE );
		printf("===Ret[%d]===\n", Ret );
		if( Ret != 0 )	continue;

	}
	return( 0 );	
}

