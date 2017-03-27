/******************************************************************************
*
*  PFS.h 	--- header of PFS Library
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

/*
 *	$B:n@.(B			$BEOJUE59'(B
 *	$B;n83(B			$B1v?,>o=U!"5WJ]@56H(B
 *	$BFC5v!"Cx:n8"(B	$B%H%i%$%F%C%/(B
 *
 *	$B35MW(B
 *	 $B%U%!%$%k$O(Bread/write$B$N$_$N(Bstateless$B$H$9$k(B
 *	 $B%U%!%$%k$N%,!<%I$O(Badvisary$B!!(Block($BGSB>$N$_(B)$B$K$h$k(B
 *	$B$3$l$K$h$j%U%!%$%k$N%/%i%$%"%s%H%-%c%C%7%e2=$N<BAu$,2DG=$H$J$k(B
 *	$B$3$N$?$a$N%m%C%/2r=|MW5a$O%;%C%7%g%s%b%G%e!<%k$N%$%Y%s%H4IM}$r;HMQ$9$k(B
 *	 $B%m%C%/$K$O(Bread/write$B$,$"$j!"%m%C%/$OL58BBT$A$G$"$k(B
 *	 $B%m%C%/$N2r=|$O%m%C%/J];}%/%i%$%"%s%H$K$h$k2r=|DLCN!"DL?.CG$K$h$k(B
 *	 $B0l;~%U%!%$%k!J%(%U%'%a%i%k!K$ODL?.CG$K$h$C$F$b:o=|$5$l$k(B
 *	 $B%G%#%l%/%H%j%$%Y%s%H$K$O!"?75,DI2C!"%U%!%$%k$N99?7!":o=|$,$"$k(B
 *	 $B%-%e!<$N(Bwait$B$O(Bpeek$B$r9T$$!"<hF@8e(Back$B$N(B3way$B$G%a%C%;!<%8$,3N<B$KEO$k(B
 *	peek$BCf$OA`:n$OJ]N1$5$l$k(B
 *
 *	snapshot$B$H2sI|(B
 *	 snapshot$B$O!"(Btar$B$G%U%!%$%k%7%9%F%`$r%P%C%/%"%C%W$7$F$$$k(B
 *	$B<B1?MQ$G$OJQ99J,$N%8%c!<%J%k2=$,I,MW$H$J$k(B
 *
 *	$BBgMFNL%G!<%?(B
 *	 write$B;~$NBgMFNL%G!<%?$O(BOut-Of-Band$B$rMxMQ(B
 *
 *	File$B9=B$BN(B
 *	 $B%/%i%$%"%s%H$K%*%U%;%C%HCM4IM}$N(BFile$B9=B$BN$r;}$D(B
 *	$B%5!<%P$G$O(BFILE$B$r%-%c%C%7%e2=$7$F4IM}$9$k(B
 *	$B$^$?!"(BFILE$B$J$N$G%G!<%?$O<+F0E*$K%-%c%C%7%e2=$5$l$k(B
 *
 */
/*
 *	$B;HMQJ}K!(B
 *	PFSLockW			$BGSB>@k8@(B(write)
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$BGSB>L>(B
 *	PFSLockR			$BGSB>@k8@(B(read)
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$BGSB>L>(B
 *	PFSUnlock			$BGSB>2r=|(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$BGSB>L>(B
 *	PFSCreate			$B%U%!%$%k$N:n@.(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%U%!%$%kL>(B
 *	PFSOpen				$B%U%!%$%k$N%*!<%W%s(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%U%!%$%kL>(B
 *	PFSClose			$B%U%!%$%k$N%/%m!<%:(B
 *		pF					$B%U%!%$%k%O%s%I%k(B
 *	PFSWrite			$B=q$-=P$7(B
 *		pF					$B%U%!%$%k%O%s%I%k(B
 *		pBuff				$B%P%C%U%!(B
 *		Len					$B%P%C%U%!D9(B
 *	PFSRead				$BFI$_<h$j(B
 *		pF					$B%U%!%$%k%O%s%I%k(B
 *		pBuff				$B%P%C%U%!(B
 *		Len					$B%P%C%U%!D9(B
 *	PFSLseek			$B%*%U%;%C%H@_Dj(B
 *		pF					$B%U%!%$%k%O%s%I%k(B
 *		Offset				$B%*%U%;%C%HCM(B
 *	PFSDelete			$B%U%!%$%k$N:o=|(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%U%!%$%kL>(B
 *	PFSStat				$B%U%!%$%k$N>uBV(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%U%!%$%kL>(B
 *		pStat				struct$B!!(Bstat$B9=B$BN(B
 *	PFSTruncate				$B%U%!%$%k$N>uBV(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%U%!%$%kL>(B
 *		Len					$B%U%!%$%kD9(B
 *	PFSMkdir			$B%G%#%l%/%H%j$N:n@.(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%G%#%l%/%H%jL>(B
 *	PFSRmdir			$B%G%#%l%/%H%j$N:o=|(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%G%#%l%/%H%jL>(B
 *	PFSReadDir			$B%G%#%l%/%H%j%(%s%H%j$NFI9~(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%G%#%l%/%H%jL>(B
 *		pEntry				$B%G%#%l%/%H%j%(%s%H%jG[Ns(B
 *		Index				$BFI9~3+;O0LCV(B
 *		pNumber				$BFI9~:GBg8D?t(B
 *	PFSEventDirSet		$B%$%Y%s%H<hF@%G%#%l%/%H%j$N<hF@3+;O(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%G%#%l%/%H%jL>(B
 *		pEvent				$B%$%Y%s%H<hF@%;%C%7%g%s(B
 *	PFSEventDirCancel	$B%$%Y%s%H<hF@%G%#%l%/%H%j$N<hF@=*N;(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%G%#%l%/%H%jL>(B
 *		pEvent				$B%$%Y%s%H<hF@%;%C%7%g%s(B
 *	PFSEventLockSet		$B%$%Y%s%H<hF@%m%C%/$N<hF@3+;O(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%m%C%/L>(B
 *		pEvent				$B%$%Y%s%H<hF@%;%C%7%g%s(B
 *	PFSEventLockCancel	$B%$%Y%s%H<hF@%m%C%/$N<hF@=*N;(B
 *		pSession			$B%;%C%7%g%s(B
 *		pName				$B%m%C%/L>(B
 *		pEvent				$B%$%Y%s%H<hF@%;%C%7%g%s(B
 *	PFSEventGetByCallback		$B%$%Y%s%H<uIU4X?t(B
 *		pSession			$B%;%C%7%g%s(B
 *		pCnt				$B%$%Y%s%H?t(B
 *		pOmitted			$BL5;k$5$l$?%$%Y%s%H?t(B
 *		fCallback			callback
 *	PFSPost				$B%-%e!<$K%a%C%;!<%8$rAw$k(B
 *		pSession			$B%;%C%7%g%s(B
 *		pQueue				$B%-%e!<L>(B
 *		pBuff				$B%P%C%U%!(B
 *		Len					$B%P%C%U%!D9(B
 *	PFSPeek				$B%-%e!<$+$i%a%C%;!<%8$r<u$1<h$k(B
 *		pSession			$B%;%C%7%g%s(B
 *		pQueue				$B%-%e!<L>(B
 *		pBuff				$B%P%C%U%!(B
 *		Len					$B%P%C%U%!D9(B
 *	PFSPeek				$B%-%e!<$+$i%a%C%;!<%8$r:o=|$9$k(B
 *		pSession			$B%;%C%7%g%s(B
 *		pQueue				$B%-%e!<L>(B
 *	PFSEventQueueSet	$B%$%Y%s%H<hF@%G%#%l%/%H%j$N<hF@3+;O(B
 *		pSession			$B%;%C%7%g%s(B
 *		pQueue				$B%-%e!<L>(B
 *		pEvent				$B%$%Y%s%H<hF@%;%C%7%g%s(B
 *	PFSEventEventCancel	$B%$%Y%s%H<hF@%G%#%l%/%H%j$N<hF@=*N;(B
 *		pSession			$B%;%C%7%g%s(B
 *		pQueue				$B%-%e!<L>(B
 *		pEvent				$B%$%Y%s%H<hF@%;%C%7%g%s(B
 *	PFSCopy				$B%U%!%$%k$N%3%T!<(B
 *		pSession			$B%;%C%7%g%s(B
 *		pFrom				$B85%U%!%$%k(B
 *		pTo					$B@h%U%!%$%k(B
 *	PFSConcat			$B%U%!%$%k$NO"7k(B
 *		pSession			$B%;%C%7%g%s(B
 *		pFrom				$B85%U%!%$%k(B
 *		pTo					$B@h%U%!%$%k(B
 */

#ifndef	__PFS__
#define	__PFS__

#include	"PaxosSession.h"
#include	"FileCache.h"

#include	<netdb.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<dirent.h>
#include	<libgen.h>

#define	FILE_NAME_MAX	256	//64
#define	SEND_BUFF_SIZE	(1024*4)

#define	FILE_SHUTDOWN	"SHUTDOWN"

#define	DATA_SIZE_WRITE	(PAXOS_COMMAND_SIZE - sizeof(PFSWriteReq_t))
#define	DATA_SIZE_POST	(PAXOS_COMMAND_SIZE - sizeof(PFSPostMsgReq_t))

#define	FILE_EPHEMERAL	0x1

/* Snapshot */
#define	PFS_SNAP_TAR		"SNAP_PFS_TAR"
#define	PFS_SNAP_UPD_LIST	"SNAP_PFS_UPD"
#define	PFS_SNAP_DEL_LIST	"SNAP_PFS_DEL"

#ifdef KKK
#define	TAR_SIZE_MAX	1000000000LL	/* 1GB */
#else
#define	TAR_SIZE_MAX	500000000LL	/* 500M */
#endif

/*
 *	Error
 */
#define	ERR_PFS	0x1000
#define	ERR_PFS_NO_MEM		(ERR_PFS|0x1)
#define	ERR_PFS_EXIST		(ERR_PFS|0x2)
#define	ERR_PFS_NOT_EXIST	(ERR_PFS|0x4)

#define	pLOG	PaxosSessionGetLog( pSession )

#ifdef	_PFS_SERVER_
#define	pALOG	PaxosAcceptGetLog( pAccept )
#endif

inline	static char*
RootOmitted( char *pName )
{
	char	*pC = pName;

	while( *pC ) {
		switch( *pC ) {
			case '/':
				if( *(pC+1) == '\0' ) {
					*pC	= '.';
					break;
				} else {
					pC++;
					continue;
				}
			case '.':
				if( *(pC+1) == '/' ) {
					pC++; pC++;
					continue;
				} else {
					break;
				}
		}
		break;
	}
	return( pC );
}
/*
 *	API
 */
#ifdef	_PFS_SERVER_

struct FileHead;

#define	PFS_ROOT		"./PFS"
#define	PFS_PFS			"PFS"
#define	PFS_PAXOS		"PAXOS"
#define	PFS_OSTYPE		"OSTYPE"


#define	PATH_NAME_MAX_REAL	(PATH_NAME_MAX+8)

typedef struct PFSControl {
	struct PaxosAcceptor*	c_pAcceptor;
	RwLock_t				c_RwLock;

	DHashList_t				c_Lock;
	DHashList_t				c_Path;
	DHashList_t				c_Queue;

	int						c_Max;
	int						c_Maximum;
	BlockCacheCtrl_t		c_BlockCache;
	bool_t					c_bCksum;
	bool_t					c_bRas;
} PFSControl_t;

struct PFSLock;
struct PFSQueue;

typedef	struct SessionAdaptor {
	list_t							a_LockWait;	 
	struct PFSLock*					a_pLockWait;
	bool_t							a_LockRead;
	list_t							a_LockHold;	 
	HashList_t						a_Ephemeral;
	struct PaxosAccept*	a_pAccept;
	list_t							a_Events;
	list_t							a_QueueWait;
	struct PFSQueue*				a_pQueueWait;
} SessionAdaptor_t;

typedef enum {
	JOINT_EVENT_PATH,
	JOINT_EVENT_LOCK,
	JOINT_EVENT_QUEUE,
	JOINT_LOCK_WRITE_HOLD,
	JOINT_LOCK_READ_HOLD,
	JOINT_EVENT_LOCK_RECALL,
} PFSJointType_t;

typedef struct PFSJoint {
	Joint_t				j_Head;
	PFSJointType_t		j_Type;
	SessionAdaptor_t*	j_pAdaptor;
	int					(*j_fGarbage)(void*);
	void*				j_pArg;
} PFSJoint_t;

typedef struct PFSPath {
	list_t		p_Lnk;
	char		p_Name[PATH_NAME_MAX];
	list_t		p_AdaptorEvent;
	int			p_RefCnt;
	int			p_Flags;
	unsigned int			p_EventSeq;
} PFSPath_t;

typedef struct Ephemeral {
	list_t	e_Lnk;
	char	e_Name[PATH_NAME_MAX];
} Ephemeral_t;

typedef struct PFSLock {
	list_t				l_Lnk;
	list_t				l_AdaptorWait;	// Adaptors wait Unlock
	list_t				l_AdaptorHold;	// Adaptors hold Lock
	list_t				l_AdaptorEvent;	// Adaptors wait Event
	int					l_Cnt;
	char				l_Name[PATH_NAME_MAX];
	Hash_t				l_HoldHash;
	int					l_Ver;			// increment on write/read change
	unsigned int		l_EventSeq;
} PFSLock_t;

#define	QUEUE_POST_MAX	100

typedef struct PFSQueue {
	list_t			q_Lnk;
	bool_t			q_Peek;			// Peek mode
	int				q_CntMsg;		// number of messages
	list_t			q_AdaptorMsg;	// messages list
	list_t			q_AdaptorWait;	// Adaptors wait message
	list_t			q_AdaptorEvent;	// Adaptors wait Event
	char			q_Name[PATH_NAME_MAX];
	unsigned int	q_EventSeq;
} PFSQueue_t;

extern	int	PFSInit( int Core, int Ext, int id, bool_t bCksum,
			int	FdMax, char *pRoot, 
			int BlockMax, int SegSize, int SegNum, int UsecWB,
			struct PaxosAcceptor* pAcceptor );
extern int PFSRequest( struct PaxosAcceptor* pAcceptor, PaxosSessionHead_t* pM, 
									int fd );
extern int
PFSNotify( struct PaxosAcceptor* pAcceptor, 
			PaxosMode_t Mode, PaxosNotify_t Notify, 
			uint32_t seq, int server, uint32_t epoch, 
			int32_t len, void *pC, int from );

extern	int	PFSAdaptorOpen( struct PaxosAccept*, struct PaxosSessionHead* );
extern	int	PFSAdaptorClose( struct PaxosAccept*, struct PaxosSessionHead* );
extern	int	PFSValidate( struct PaxosAcceptor*, uint32_t,
								struct PaxosSessionHead*, int );
extern	int	PFSAccepted( struct PaxosAccept*, struct PaxosSessionHead* );
extern	int	PFSRejected( struct PaxosAcceptor*, struct PaxosSessionHead* );
extern	int	PFSAbort( struct PaxosAcceptor*, uint32_t );
extern	int	PFSSessionShutdown( struct PaxosAcceptor*, 
					PaxosSessionShutdownReq_t*, uint32_t nextDecide );

extern	Msg_t*	PFSBackupPrepare(int);
extern	int	PFSBackup();
extern	int	PFSRestore();
extern	int	PFSTransferSend( IOReadWrite_t* );
extern	int	PFSTransferRecv( IOReadWrite_t* );

extern	int	PFSGlobalMarshal( IOReadWrite_t* );
extern	int	PFSGlobalUnmarshal( IOReadWrite_t* );
extern	int	PFSAdaptorMarshal( IOReadWrite_t*, struct PaxosAccept* );
extern	int	PFSAdaptorUnmarshal( IOReadWrite_t*, struct PaxosAccept* );

extern	int	PFSSessionSnapshot( PaxosMode_t, struct PaxosAcceptor*, uint32_t );

extern	int	PFSSessionLock();
extern	int	PFSSessionUnlock();
extern	int	PFSSessionAny(struct PaxosAcceptor*,PaxosSessionHead_t*,int);

#endif	// _PFS_SERVER_

struct Session;
typedef struct File {
	int			f_Flags;
	char		f_Name[PATH_NAME_MAX];
	off_t		f_Offset;
	struct Session*	f_pSession;
} File_t;

#define	PFS_APPEND	((off_t)-1)


#define	PFS_SESSION_NORMAL	0
#define	PFS_SESSION_EVENT	1

#define	PFS_FAMILY	(1<<16)

typedef enum PFSCmd {
	PFS_OPEN 		= (PFS_FAMILY|0),
	PFS_CLOSE		= (PFS_FAMILY|1),
	PFS_READ		= (PFS_FAMILY|2),
	PFS_WRITE		= (PFS_FAMILY|3),
	PFS_DELETE		= (PFS_FAMILY|4),
	PFS_STAT		= (PFS_FAMILY|5),
	PFS_LOCK		= (PFS_FAMILY|6),
	PFS_UNLOCK		= (PFS_FAMILY|7),
	PFS_OUTOFBAND	= (PFS_FAMILY|8),
	PFS_DATA_RPL	= (PFS_FAMILY|9),
	PFS_DATA_REQ	= (PFS_FAMILY|10),
	PFS_DATA_APPLY	= (PFS_FAMILY|11),
	PFS_MKDIR		= (PFS_FAMILY|12),
	PFS_RMDIR		= (PFS_FAMILY|13),
	PFS_READDIR		= (PFS_FAMILY|14),
	PFS_EVENT_DIR	= (PFS_FAMILY|15),
	PFS_EVENT_LOCK	= (PFS_FAMILY|16),
	PFS_EVENT_GET	= (PFS_FAMILY|17),
	PFS_CNTRL		= (PFS_FAMILY|18),
	PFS_DUMP		= (PFS_FAMILY|19),
	PFS_ABORT		= (PFS_FAMILY|20),

	PFS_POST_MSG	= (PFS_FAMILY|21),
	PFS_PEEK_MSG	= (PFS_FAMILY|22),
	PFS_PEEK_ACK	= (PFS_FAMILY|23),
	PFS_EVENT_QUEUE	= (PFS_FAMILY|24),

	PFS_TRUNCATE	= (PFS_FAMILY|25),
	PFS_CREATE		= (PFS_FAMILY|26),
	PFS_COPY		= (PFS_FAMILY|27),
	PFS_CONCAT		= (PFS_FAMILY|28),

	PFS_RAS			= (PFS_FAMILY|29),
	PFS_UNLOCK_SUPER= (PFS_FAMILY|30),
} PFSCmd_t;

typedef struct PFSHead {
	PaxosSessionHead_t	h_Head;
	int			h_Error;
} PFSHead_t;

#define	MSGINIT( pF, cmd, code, len ) \
	({PFSHead_t*p=(PFSHead_t*)(pF); \
		SESSION_MSGINIT( pF, cmd, code, 0, len ); \
		p->h_Error=0;})

#define	MSGALLOC( cmd, code, len, fMalloc ) \
	({PFSHead_t* pF; pF = (PFSHead_t*)fMalloc( len ); \
		MSGINIT( pF, cmd, code, len ); pF;})

#define	MSGFREE( pF, fFree )	fFree( pF );

typedef	struct PFSSRASReq {
	PFSHead_t	r_Head;
	char		r_Cell[PAXOS_CELL_NAME_MAX];
	char		r_Path[PATH_NAME_MAX];
} PFSRASReq_t;

typedef struct PFSRASRpl {
	PFSHead_t	r_Head;
} PFSRASRpl_t;

typedef struct PFSCopyReq {
	PFSHead_t	c_Head;
	char		c_From[PATH_NAME_MAX];
	char		c_To[PATH_NAME_MAX];
} PFSCopyReq_t;

typedef struct PFSCopyRpl {
	PFSHead_t	c_Head;
} PFSCopyRpl_t;

typedef struct PFSConcatReq {
	PFSHead_t	c_Head;
	char		c_From[PATH_NAME_MAX];
	char		c_To[PATH_NAME_MAX];
} PFSConcatReq_t;

typedef struct PFSConcatRpl {
	PFSHead_t	c_Head;
} PFSConcatRpl_t;
	
typedef struct PFSCreateReq {
	PFSHead_t	c_Head;
	int			c_Flags;
	char		c_Name[PATH_NAME_MAX];
} PFSCreateReq_t;

typedef struct PFSCreateRpl {
	PFSHead_t	c_Head;
} PFSCreateRpl_t;
	
typedef struct PFSWriteReq {
	PFSHead_t	w_Head;
	int			w_Flags;
	char		w_Name[PATH_NAME_MAX];
	off_t		w_Offset;
	size_t		w_Len;
	uchar_t		w_Data[1];
} PFSWriteReq_t;

typedef struct PFSWriteRpl {
	PFSHead_t	w_Head;
} PFSWriteRpl_t;
	
typedef struct PFSReadReq {
	PFSHead_t	r_Head;
	char		r_Name[PATH_NAME_MAX];
	off_t		r_Offset;
	size_t		r_Len;
} PFSReadReq_t;

typedef struct PFSReadRpl {
	PFSHead_t	r_Head;
	size_t		r_Len;
} PFSReadRpl_t;
	
typedef struct PFSDeleteReq {
	PFSHead_t	d_Head;
	char		d_Name[PATH_NAME_MAX];
} PFSDeleteReq_t;

typedef struct PFSDeleteRpl {
	PFSHead_t	d_Head;
} PFSDeleteRpl_t;

typedef struct PFSTruncateReq {
	PFSHead_t	t_Head;
	char		t_Name[PATH_NAME_MAX];
	off_t		t_Len;
} PFSTruncateReq_t;

typedef struct PFSTruncateRpl {
	PFSHead_t	t_Head;
} PFSTruncateRpl_t;

typedef struct PFSStatReq {
	PFSHead_t	s_Head;
	char		s_Name[PATH_NAME_MAX];
} PFSStatReq_t;

typedef struct PFSStatRpl {
	PFSHead_t	s_Head;
	struct stat	s_Stat;
} PFSStatRpl_t;

typedef struct PFSLockReq {
	PFSHead_t	l_Head;
	bool_t		l_bRead;
	bool_t		l_bTry;
	char		l_Name[PATH_NAME_MAX];
} PFSLockReq_t;

typedef struct PFSLockRpl {
	PFSHead_t	l_Head;
	int			l_Ver;
} PFSLockRpl_t;

typedef struct PFSUnlockReq {
	PFSHead_t	l_Head;
	char		l_Name[PATH_NAME_MAX];
	int			l_Ver;
} PFSUnlockReq_t;

typedef struct PFSUnlockRpl {
	PFSHead_t	l_Head;
} PFSUnlockRpl_t;

typedef struct PFSUnlockSuperReq {
	PFSHead_t		l_Head;
	PaxosClientId_t	l_Id;
	char			l_Name[PATH_NAME_MAX];
	int				l_Ver;
} PFSUnlockSuperReq_t;

typedef struct PFSUnlockSuperRpl {
	PFSHead_t	l_Head;
} PFSUnlockSuperRpl_t;

typedef struct PFSEventLockReq {
	PFSHead_t	el_Head;
	bool_t		el_Set;
	char		el_Name[PATH_NAME_MAX];
	PaxosClientId_t	el_Id;
	bool_t		el_bRecall;
} PFSEventLockReq_t;

typedef struct PFSEventLockRpl {
	PFSHead_t	el_Head;
} PFSEventLockRpl_t;

typedef struct PFSEventLock {
	PFSHead_t	el_Head;
	char		el_Name[PATH_NAME_MAX];
	bool_t		el_Read;
	int			el_Ver;
	time_t			el_Time;
	unsigned int	el_EventSeq;
} PFSEventLock_t;

typedef struct PFSOutOfBandReq {
	PFSHead_t	o_Head;
} PFSOutOfBandReq_t;

typedef struct PFSOutOfBandRpl {
	PFSHead_t	o_Head;
} PFSOutOfBandRpl_t;

typedef struct PFSDataReq {
	PFSHead_t	d_Head;
	int			d_Server;
	uint32_t	d_Epoch;
} PFSDataReq_t;

typedef struct PFSDataRpl {
	PFSHead_t	d_Head;
	int			d_From;
} PFSDataRpl_t;

typedef struct PFSDataApply {
	PFSHead_t	a_Head;
	int			a_From;
} PFSDataApply_t;

typedef struct PFSMkdirReq {
	PFSHead_t	m_Head;
	char		m_Name[PATH_NAME_MAX];
} PFSMkdirReq_t;

typedef struct PFSMkdirRpl {
	PFSHead_t	m_Head;
} PFSMkdirRpl_t;

typedef struct PFSRmdirReq {
	PFSHead_t	r_Head;
	char		r_Name[PATH_NAME_MAX];
} PFSRmdirReq_t;

typedef struct PFSRmdirRpl {
	PFSHead_t	r_Head;
} PFSRmdirRpl_t;

typedef struct PFSReadDirReq {
	PFSHead_t	r_Head;
	char		r_Name[PATH_NAME_MAX];
	int32_t		r_Index;
	int32_t		r_Number;
} PFSReadDirReq_t;

typedef	struct PFSDirent {
	struct dirent	de_ent;
#define	de_Name	de_ent.d_name
} PFSDirent_t;

typedef struct PFSReadDirRpl {
	PFSHead_t		r_Head;
	int32_t			r_Number;
} PFSReadDirRpl_t;

typedef	struct PFSEventDirReq {
	PFSHead_t	ed_Head;
	bool_t		ed_Set;
	char		ed_Name[PATH_NAME_MAX];
	PaxosClientId_t	ed_Id;
} PFSEventDirReq_t;

typedef	struct PFSEventDirRpl {
	PFSHead_t	ed_Head;
} PFSEventDirRpl_t;

typedef enum PFSDirEntryMode {
	DIR_ENTRY_CREATE,
	DIR_ENTRY_DELETE,
	DIR_ENTRY_UPDATE,
} PFSDirEntryMode_t;

typedef	struct PFSEventDir {
	PFSHead_t			ed_Head;
	char				ed_Name[PATH_NAME_MAX];
	PFSDirEntryMode_t	ed_Mode;
	char				ed_Entry[FILE_NAME_MAX];
	time_t				ed_Time;
	unsigned int		ed_EventSeq;
} PFSEventDir_t;

typedef struct PFSEventGetReq {
	PFSHead_t	eg_Head;
} PFSEventGetReq_t;

typedef struct PFSEventGetRpl {
	PFSHead_t	eg_Head;
	int			eg_Count;
} PFSEventGetRpl_t;

typedef struct PFSPostMsgReq {
	PFSHead_t	p_Head;
	char		p_Name[PATH_NAME_MAX];
	size_t		p_Len;
} PFSPostMsgReq_t;

typedef struct PFSPostMsgRpl {
	PFSHead_t	p_Head;
} PFSPostMsgRpl_t;

typedef struct PFSPeekMsgReq {
	PFSHead_t	p_Head;
	char		p_Name[PATH_NAME_MAX];
} PFSPeekMsgReq_t;

typedef struct PFSPeekMsgRpl {
	PFSHead_t	p_Head;
	size_t		p_Len;
} PFSPeekMsgRpl_t;

typedef struct PFSPeekAckReq {
	PFSHead_t	a_Head;
	char		a_Name[PATH_NAME_MAX];
} PFSPeekAckReq_t;

typedef struct PFSPeekAckRpl {
	PFSHead_t	a_Head;
} PFSPeekAckRpl_t;

typedef struct PFSEventQueueReq {
	PFSHead_t	eq_Head;
	bool_t		eq_Set;
	char		eq_Name[PATH_NAME_MAX];
	PaxosClientId_t	eq_Id;
} PFSEventQueueReq_t;

typedef struct PFSEventQueueRpl {
	PFSHead_t	eq_Head;
} PFSEventQueueRpl_t;

typedef struct PFSEventQueue {
	PFSHead_t	eq_Head;
	char		eq_Name[PATH_NAME_MAX];
	int32_t		eq_PostWait;	// 0-Post 1-Wait
	PaxosClientId_t	eq_Id;
	time_t			eq_Time;
	unsigned int	eq_EventSeq;
} PFSEventQueue_t;

extern	int PFSUnlock( struct Session*, char *pName );
extern	int PFSUnlockSuper( struct Session*, 
					struct Session *pOwner, char *pName );
extern	int PFSLockW( struct Session* pSession, char *pathname );
extern	int PFSLockR( struct Session* pSession, char *pathname );
extern	int PFSTryLockW( struct Session* pSession, char *pathname );
extern	int PFSTryLockR( struct Session* pSession, char *pathname );

extern	int	PFSCreate( struct Session*, char *pName, int Flags );
extern	struct File* PFSOpen( struct Session*, char *pName, int Flags );
extern	int PFSClose( struct File* pF );

extern	long PFSWrite( struct File* pF, char* pBuff, size_t Len );
extern	long PFSWriteInBand( File_t* pF, char* pBuff, size_t Len );
extern	long PFSWriteOutOfBand( File_t* pF, char* pBuff, size_t Len );

extern	long PFSRead(  struct File* pF, char* pBuff, size_t Len );

extern	int PFSLseek( struct File* pF, off_t Offset );
extern	int PFSDelete( struct Session*, char *pathname );
extern	int PFSTruncate( struct Session*, char *pathname, off_t Len );
extern	int PFSStat( struct Session*, char *pathname, struct stat* );

extern	int PFSCopy( struct Session*, char *pFrom, char *pTo );
extern	int PFSConcat( struct Session*, char *pFrom, char *pTo );

extern	int PFSMkdir( struct Session* pSession, char *pathname );
extern	int PFSRmdir( struct Session* pSession, char *pathname );
extern	int PFSReadDir( struct Session* pSession, char *pathname, 
		PFSDirent_t* pE, int32_t Index, int32_t* pNumber );

extern	int PFSEventDirSet( struct Session* pSession,
							char* pName, struct Session *pEvent );
extern	int PFSEventDirCancel( struct Session* pSession,
							char* pName, struct Session *pEvent );
extern	int _PFSEventLockSet( struct Session* pSession, 
						char* pName, struct Session *pEvent, bool_t bRcall );

#define	PFSEventLockSet( pS, pN, pE ) \
				_PFSEventLockSet( (pS), (pN), (pE), FALSE )
extern	int PFSEventLockCancel( struct Session* pSession, 
							char* pName, struct Session *pEvent );

extern	int PFSEventGetByCallback( struct Session* pSession, 
				int32_t* pCnt, int32_t* pOmitted, 
				int(*fCallback)(struct Session*,PFSHead_t*) );

extern	int	PFSPost( struct Session*, char* pQueue, char* pBuf, size_t Len );
extern	int	PFSPeek( struct Session*, char* pQueue, char* pBuf, size_t Len );
extern	int	PFSPeekAck( struct Session*, char* pQueue );
extern	int PFSEventQueueSet( struct Session* pSession, 
							char* pName, struct Session *pEvent );
extern	int PFSEventQueueCancel( struct Session* pSession, 
							char* pName, struct Session *pEvent );
extern	int	PFSEventQueue( struct Session*, PFSEventQueue_t* );

extern	int PFSRasRegister(struct Session *pSession, char *pPath, char *pEntry);
extern	int PFSRasUnregister(struct Session *pSession, 
									char *pPath, char *pEntry);
extern	int PFSRasThreadCreate( char *pCellName, 
								struct Session *pSession,
								char *pPath, int Id,
								int (*fCallback)(struct Session*,PFSHead_t*) );
extern	int	_PFSRasSend( struct Session*, PFSHead_t* );

typedef struct PFSRasTag {
	char			r_Path[PATH_NAME_MAX];
	char			r_Ephem[FILE_NAME_MAX];
	char			r_Cell[FILE_NAME_MAX];
	int				r_Id;
	int				(*r_fCallback)(struct Session*, PFSHead_t*);
} PFSRasTag_t;

typedef	struct PFSDlg {
	list_t		d_Lnk;
	char		d_Name[PATH_NAME_MAX];

#define	PFS_DLG_R		1
#define	PFS_DLG_W		2	
	int			d_Own;
	int			d_Ver;
	Hash_t		d_Hold;
	Cnd_t		d_Cnd;
	int			d_Wait;
	int			d_WaitRecall;
	Mtx_t		d_Mtx;
	struct Session	*d_pS;
	struct Session	*d_pE;
	int			(*d_fInit)( struct PFSDlg*, void* );
	int			(*d_fTerm)( struct PFSDlg*, void* );
	void		*d_pTag;
	int			d_OwnOld;
	int			d_Err;
} PFSDlg_t;

extern	int	PFSDlgInit( struct Session *pS, struct Session *pE,
							PFSDlg_t*, char *pName, 
							int(*fInit)(struct PFSDlg*,void*pArg),
							int(*fTerm)(struct PFSDlg*,void*pArg),
							void*pTag );
extern	int	PFSDlgInitByPNT( struct Session *pS, struct Session *pE,
							PFSDlg_t*, char *pName, 
							int(*fInit)(struct PFSDlg*,void*pArg),
							int(*fTerm)(struct PFSDlg*,void*pArg),
							void*pTag );
extern	int	PFSDlgDestroy( PFSDlg_t *pDlg );

#define	PFSDlgLock( pD )	MtxLock( &(pD)->d_Mtx )
#define	PFSDlgUnlock( pD )	MtxUnlock( &(pD)->d_Mtx )

extern	int	PFSDlgHoldW( PFSDlg_t*, void* );
extern	int	PFSDlgHoldR( PFSDlg_t*, void* );
extern	int	PFSDlgTryHoldW( PFSDlg_t*, void* );
extern	int	PFSDlgTryHoldR( PFSDlg_t*, void* );
extern	int	PFSDlgRelease( PFSDlg_t* );
extern	int	PFSDlgReturn( PFSDlg_t*, void* );
extern	int	PFSDlgRecall( PFSEventLock_t*, PFSDlg_t*, void* );

extern	int	PFSDlgHoldWBy( PFSDlg_t*, void*, void* );
extern	int	PFSDlgHoldRBy( PFSDlg_t*, void*, void* );
extern	int	PFSDlgReleaseBy( PFSDlg_t*, void* );
extern	int	PFSDlgReturnBy( PFSDlg_t*, void*, void* );

#define	PFSDlgEventSet( pD )	\
		_PFSEventLockSet( (pD)->d_pS, (pD)->d_Name, (pD)->d_pE, TRUE )
#define	PFSDlgEventCancel( pD )	\
		PFSEventLockCancel( (pD)->d_pS, (pD)->d_Name, (pD)->d_pE )

#ifdef	_PFS_SERVER_

typedef	struct PFSCntrlReq {
	PFSHead_t	c_Head;
} PFSCntrlReq_t;

typedef	struct PFSCntrlRpl {
	PFSHead_t		c_Head;
	PFSControl_t	c_Cntrl;
	PFSControl_t*	c_pCntrl;
} PFSCntrlRpl_t;

typedef	struct PFSDumpReq {
	PFSHead_t		d_Head;
	void*			d_pStart;
	int				d_Size;
} PFSDumpReq_t;

typedef	struct PFSDumpRpl {
	PFSHead_t		d_Head;
	char			d_Data[1];
} PFSDumpRpl_t;

extern	PFSReadRpl_t*	PFSReadDirect( void*, File_t*, size_t );
extern	int PFSReadCntrl( struct Session* pSession, void* pH, 
					PFSControl_t* pCntrl, PFSControl_t** ppCntrl );
extern	int PFSReadMem( struct Session* pSession, void* pH, 
			void* pStart, int Size, void* pData );

#endif	// _PFS_SERVER_

extern	char* RootOmitted( char *pName );

#endif	// __PFS__
