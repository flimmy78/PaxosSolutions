/******************************************************************************
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


#define	F_ULOCK 0
#define	F_LOCK 1
#define	F_TLOCK 2
#include	<fcntl.h>
#include	<unistd.h>

/******************************************************************************
 *
 *         
 *	skl_lock.c
 *
 *	����	�X�P���g�� �r������
 * 
 ******************************************************************************/

#include	<stdio.h>
#include	<string.h>
#include	"skl_lock.h"

/*
 *	�r�����䎯�ʎq �Ǘ����X�g �G���g��
 */
static _dlnk_t	_lock_entry;

int _neo_lockf( lk_id_t*pt, int cmd );

/******************************************************************************
 * �֐���
 *		_neo_init_lock()
 * �@�\        
 *		�r������ ������
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_init_lock()
{
DBG_ENT( M_SKL, "_neo_init_lock" );

	_dl_init( &_lock_entry );

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		neo_create_lock()
 * �@�\        
 *		�r�����䎯�ʎq�̍쐬
 * �֐��l
 *      ����: !NULL
 *      �ُ�: NULL
 * ����
 ******************************************************************************/
lk_id_t *
neo_create_lock( resource )
char	*resource;	/* I �t�@�C����	*/
{
	lk_id_t	*pt;
	int	fd;
	char	path[NEO_MAXPATHLEN+20];
	char	file[NEO_MAXPATHLEN], *pc;

DBG_ENT( M_SKL, "neo_create_lock" );
DBG_C(( "resource = (%s)\n", resource ));

	/*
	 *	�����`�F�b�N
	 */
	if( !resource || !(*resource) || (strlen(resource) > NEO_MAXPATHLEN) ) {
		neo_errno = E_SKL_INVARG;
		goto err;
	}

	/*
	 *	�t�@�C�����쐬
	 */
	for( pc = file; *resource; pc++, resource++ ) {
		if( *resource == '/' )
			*pc = '#';
		else
			*pc = *resource;
	}
	*pc = '\0';

	sprintf( path, "%s/%s/%s", getenv("HOME"), NEO_LOCKDIR, file );

        for( pt = (lk_id_t *)_dl_head( &_lock_entry );
			_dl_valid( &_lock_entry, pt );
			pt = (lk_id_t *)_dl_next( pt ) ) {

		if( !strcmp( pt->resource, path ) ) {
			neo_errno = E_SKL_INVARG;
			goto err;
		}
	}

	/*
	 *	�r�����ʎq�̍쐬
	 */
	if( !(pt = (lk_id_t *)neo_malloc( sizeof(lk_id_t) )) ) {
		goto err;
	}

	/*
	 *	���b�N�t�@�C���̃I�[�v��
	 */
	while( 1 ) {
		if( (fd = open( path, O_RDWR|O_CREAT|O_SYNC, 0644 )) < 0 ) {
			if( errno == EINTR ) {
				continue;
			} else {
				goto uerr;
			}
		}
		break;
	}

	/*
	 *	���b�N�t�@�C���ւ̃_�~�[��������
	 */
	while( 1 ) {
		if( write( fd, "dummy", 5 ) < 0 ) {
			if( errno == EINTR ) {
				continue;
			} else {
				goto uerr;
			}
		}
		break;
	}

	strcpy( pt->resource, path );
	pt->fd   = fd;

        _dl_insert( pt, &_lock_entry );

DBG_EXIT(pt);
	return( pt );
uerr:;
	/* 
	 * UNIX ERR 
	 */
	neo_errno = unixerr2neo();
	neo_free( pt );
err:;
DBG_EXIT(0);
	return( (lk_id_t *)NULL );
}


/******************************************************************************
 * �֐���
 *		neo_delete_lock()
 * �@�\        
 *		�r�����䎯�ʎq�̍폜
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_delete_lock( lk_id )
lk_id_t	*lk_id;		/* I �r�����䎯�ʎq	*/
{
	lk_id_t	*pt;

DBG_ENT( M_SKL, "neo_delete_lock" );
DBG_C(( "lk_id = (%#x)\n", lk_id ));

        for( pt = (lk_id_t *)_dl_head( &_lock_entry );
			_dl_valid( &_lock_entry, pt );
			pt = (lk_id_t *)_dl_next( pt ) ) {

		if( pt == lk_id ) {
			_dl_remove( pt );
			while( close( pt->fd ) ) {
				if( errno == EINTR ) {
					continue;
				} else {
					neo_errno = unixerr2neo();
					goto err;
				}
			}
			neo_free( pt );
			goto ok;
		}
	}

	neo_errno = E_SKL_INVARG;
	goto err;

ok:;
DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}


/******************************************************************************
 * �֐���
 *		neo_hold_lock()
 * �@�\        
 *		�r���̐錾
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_hold_lock( lk_id, wflag )
lk_id_t	*lk_id;		/* I �r�����䎯�ʎq	*/
int	wflag;		/* I �҂��t���O		*/
{
	lk_id_t	*pt;

DBG_ENT( M_SKL, "neo_hold_lock" );
DBG_C(( "lk_id = (%#x), waitflag = (%d)\n", lk_id, wflag ));

	/*
	 *	�����̃`�F�b�N
	 */
	if( wflag != NEO_WAIT && wflag != NEO_NOWAIT ) {
		neo_errno = E_SKL_INVARG;
		goto err;
	}

	/*
	 *	���b�N
	 */
        for( pt = (lk_id_t *)_dl_head( &_lock_entry );
			_dl_valid( &_lock_entry, pt );
			pt = (lk_id_t *)_dl_next( pt ) ) {

		if( pt == lk_id ) {
			lseek( pt->fd, 0L, SEEK_SET );
			/*
			 *	���b�N(�҂��L��)
			 */
			if( wflag == NEO_WAIT ) {
				if( _neo_lockf(pt, F_LOCK) )
					goto err;
			/*
			 *	���b�N(�҂�����)
			 */
			} else {
				if( _neo_lockf(pt, F_TLOCK) )
					goto err;
			}
			goto ok;
		}
	}

	neo_errno = E_SKL_INVARG;
	goto err;
	
ok:;
DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}

int
_neo_lockf( pt, cmd )
lk_id_t	*pt;
int	cmd;
{
	int	ret = 0;
	while( lockf( pt->fd, cmd, 0L ) ) {
		if( errno == EINTR ) {
			if( neo_isterm() )
				goto err;
			else
				continue;
		} else {
			neo_errno = unixerr2neo();
			goto err;
		}
	}

end:;
	return( ret );
err:;
	ret = -1;
	goto end;
}

/******************************************************************************
 * �֐���
 *		neo_release_lock()
 * �@�\        
 *		�r���̉���
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_release_lock( lk_id )
lk_id_t	*lk_id;		/* I �r�����䎯�ʎq	*/
{
	lk_id_t	*pt;

DBG_ENT( M_SKL, "neo_release_lock" );
DBG_C(( "lk_id = (%#x)\n", lk_id ));

	/*
	 *	�A�����b�N
	 */
        for( pt = (lk_id_t *)_dl_head( &_lock_entry );
			_dl_valid( &_lock_entry, pt );
			pt = (lk_id_t *)_dl_next( pt ) ) {

		if( pt == lk_id ) {
			lseek( pt->fd, 0L, SEEK_SET );
			while( lockf( pt->fd, F_ULOCK, 0L ) ) {
				if( errno == EINTR ) {
					continue;
				} else {
					neo_errno = unixerr2neo();
					goto err;
				}
			}
			goto ok;
		}
	}

	neo_errno = E_SKL_INVARG;
	goto err;
	
ok:;
DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}

