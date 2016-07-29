/******************************************************************************
*
*  TestClient.c 	--- client test program of Paxos-Session Library
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

/**
 * $B;n83(B
 *	$B4D6-(B
 *	 PAXOS_SERVER_MAX=5
 *	 PAXOS_MAX=5
 *	 rmperm.sh	$B1JB32=%G!<%?$r:o=|(B
 *	 TestServer TestClient	$B%F%9%H%W%m%0%i%`(B
 *
 *	1.server$B$J$7(B
 *		1)TestClient
 *		[$B7k2L(B]
 *		  $BDd;_$9$k(B
 *	2.$B#1$D$G$b@8$-$F$$$l$P:F;n9T(B
 *		1)TestServer 0
 *		2)TestClient
 *		[$B7k2L(B]
 *		  $B1J1s$K:F;n9T$9$k(B
 *	3.$B:F;n9T8e!"(Bmaster$BA*DjD>8e<B9T(B
 *		1)TestServer 0
 *		2)TestClient
 *		3)TestServer 1,2
 *		[$B7k2L(B]
 *		  $B:F;n9T(B
 *		  master$BA*DjD>8e<B9T(B
 *	4.master0$B$,;`$L(B(1$B$OMW5a<uM}:Q$_!K(B
 *		1)TestServer 0,1,2,3
 *		2)TestClient
 *		3)$B$9$0$K(B0$B$r;&$9(B
 *		[$B7k2L(B]
 *		  0$B$KMW5a(B
 *		  0$B@ZCG8!CN8e(Bmaster$BC5$7$G(B1$B$K@\B3(B
 *		  1$B$O(Bmaster0$B$rJV$9(B
 *		  ELECTION WAIT$B$GBT$D(B
 *		  1$B$O(Bmaster1$B$rDLCN(B
 *		  $B:FMW5a$G1~Ez$"$j(B
 *	5.master0$B$,;`$L(B(1$B$OL$<uM}(B)
 *		1)TestServer 0,1,2,3
 *		2)admin 1 recvstop 14 1(SUCCESS)
 *		3)admin 1 catchupstop 1(CATCHUP$BMW5a(B)
 *		4)TestClient
 *		5)$B$9$0$K(B0$B$r;&$9(B
 *		[$B7k2L(B]
 *		  0$B$KMW5a(B
 *		  0$B@ZCG8!CN8e(Bmaster$BC5$7$G(B1$B$K@\B3(B
 *		  1$B$O(Bmaster0$B$rJV$9(B
 *		  ELECTION WAIT$B$GBT$D(B
 *		  1$B$O(Bmaster1$B$rDLCN(B
 *		  $B:FMW5a$G1~Ez$"$j(B
 *	6.master1$B$,;`$L(B(2$B$OMW5a<uM}:Q$_!K(B
 *		1)TestServer 1,2,3,0
 *		2)TestClient
 *		3)$B$9$0$K(B1$B$r;&$9(B
 *		[$B7k2L(B]
 *		  1$B$KMW5a(B
 *		  1$B@ZCG8!CN8e(Bmaster$BC5$7$G(B2$B$K@\B3(B
 *		  2$B$O(Bmaster1$B$rJV$9(B
 *		  ELECTION WAIT$B$GBT$D(B
 *		  0$B$"$k$$$O(B2$B$O(Bmaster2$B$rDLCN(B
 *		  $B:FMW5a$G1~Ez$"$j(B
 *	7.master1$B$,;`$L(B(1$B$OL$<uM}(B)
 *		1)TestServer 1,2,3,0
 *		2)admin 2 recvstop 14 1(SUCCESS)
 *		3)admin 2 catchupstop 1(CATCHUP$BMW5a(B)
 *		4)TestClient
 *		5)$B$9$0$K(B2$B$r;&$9(B
 *		[$B7k2L(B]
 *		  1$B$KMW5a(B
 *		  1$B@ZCG8!CN8e(Bmaster$BC5$7$G(B2$B$K@\B3(B
 *		  2$B$O(Bmaster1$B$rJV$9(B
 *		  ELECTION WAIT$B$GBT$D(B
 *		  0$B$"$k$$$O(B2$B$O(Bmaster2$B$rDLCN(B
 *		  $B:FMW5a$G1~Ez$"$j(B
 */

//#define	_DEBUG_

#define	_SERVER_
//#define	_LOCK_

#include	"PaxosSession.h"

int
main( int ac, char* av[] )
{
	int	Ret;

	if( ac < 3 ) {
		printf("TestClient cell log_size [sleep]\n");
		exit( -1 );
	}
	struct Session*	pSession;

	pSession = PaxosSessionAlloc( Connect, Shutdown, CloseFd, GetMyAddr,
									Send, Recv, Malloc, Free, av[1] );

	PaxosSessionLogCreate( pSession, atoi(av[2]), 0, stderr, LogDBG );

	Ret = PaxosSessionOpen( pSession, 0, TRUE );
	if( Ret ) {
		printf("Open ERROR\n");
		goto ret;
	}
	if( ac >= 4 ) {
		printf("sleep(%s)\n", av[3] );
		sleep( atoi(av[3]) );
	}
	Ret = PaxosSessionClose( pSession );
	printf("PaxosSessionClose:Ret=%d\n", Ret );
ret:
	LogDump( PaxosSessionGetLog( pSession ) );

	Free( pSession );
	return( 0 );
}

