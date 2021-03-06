/******************************************************************************
*
*  PFSRasActive.c 	--- Active program
*
*  Copyright (C) 2011-2016 triTech Inc. All Rights Reserved.
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

/*
 *	$B:n@.(B			$BEOJUE59'(B
 *	$B;n83(B
 *	$BFC5v!"Cx:n8"(B	$B%H%i%$%F%C%/(B
 */

//#define	_DEBUG_


#include	"PFS.h"
#include	<stdio.h>
#include	<semaphore.h>

int
usage()
{
	printf("PFSRasActive ras_cell path file\n");
	return( 0 );
}

int
main( int ac, char* av[] )
{
	int	Ret;

	if( ac < 4 ) {
		usage();
		exit( -1 );
	}
	struct Session*	pSession;

	pSession = PaxosSessionAlloc( Connect, Shutdown, CloseFd, GetMyAddr,
			Send, Recv, NULL, NULL, av[1] );
	if( !pSession ) {
		printf("Can't get cell[%s] addr.\n", av[1] );
		exit( -1 );
	}

	Ret	= PaxosSessionOpen( pSession, PFS_SESSION_NORMAL, FALSE );
	if( Ret ) goto err;

	Ret = PFSRasRegister( pSession, av[2], av[3] );
	if( Ret ) goto err1;

	sem_t	Sem;
	sem_init( &Sem, 0, 0 );
	sem_wait( &Sem );

	PaxosSessionClose( pSession );
	PaxosSessionFree( pSession );
	return( 0 );
err1:
	PaxosSessionClose( pSession );
	PaxosSessionFree( pSession );
err:
	return( -1 );
}
