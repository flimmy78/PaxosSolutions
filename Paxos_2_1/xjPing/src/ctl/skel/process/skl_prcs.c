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

/******************************************************************************
 *
 *         
 *	skl_prcs.c
 *
 *	����	�X�P���g�� �v���Z�X�Ǘ�
 * 
 ******************************************************************************/

#include	"skl_prcs.h"

/*
 *	�v���Z�X�Ǘ����X�g �G���g��
 */
static _dlnk_t	_child_entry;

/*
 *	�I���ʒm�t���O
 */
static int	_isterm_flag;

static	void	_neo_child_clean();

/******************************************************************************
 * �֐���
 *		_neo_init_prcs()
 * �@�\        
 *		�v���Z�X�Ǘ� ������
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_init_prcs()
{
DBG_ENT( M_SKL, "_neo_init_prcs" );

	_dl_init( &_child_entry );
	_isterm_flag = 0;

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		neo_fork()
 * �@�\        
 *		�q�v���Z�X�̐���
 * �֐��l
 *      ����: �e: 0< (�v���Z�Xid)
 *          : �q: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_fork()
{
	_neo_child_t	*pt;
	int		pid = -1, before;

DBG_ENT( M_SKL, "neo_fork" );

    	if( !(pt = (_neo_child_t *)neo_malloc(sizeof(_neo_child_t))) ) {
		goto end;
	}

	switch( pid = fork() ) {

	/*
	 *	Child process
	 */
	case 0:
		neo_free( pt );
		_neo_child_clean();
		break;

	/*
	 *	Error
	 */
	case -1:
		neo_free( pt );
		/* 
		 * UNIX ERR 
		 */
		neo_errno = unixerr2neo();
		break;
		
	/*
	 *	Parent process
	 */
	default:
		before = sigblock( sigmask( SIGCLD ) );

		pt->pid = pid;

		_dl_insert( pt, &_child_entry );

		sigsetmask( before );

		break;
	}

end:;
DBG_EXIT(pid);
	return( pid );
}


/*******************************************************************************
 * �֐���
 *		neo_exec
 * �@�\
 * 		�v���O�����̋N��
 * �֐��l
 *	����: ����
 *	!0:   �ُ�
 * ����
 ******************************************************************************/
int
neo_exec(path, execstr, envstr)
char	*path;		/* ���[�h���W���[���p�X�� */
char	*execstr;	/* ���s�`�� */
char	*envstr;	/* ���ϐ� */
{
	int	ac, nenv, i;
	char	*av[20], *env[20];
	struct	stat	st;

DBG_ENT( M_SKL, "neo_exec" );
DBG_C(( "path = (%s) execstr = (%s) envstr = (%s) \n", path, execstr, envstr ));

	/*
	 *	�t�@�C������
	 */
	if (stat(path, &st) < 0) {	 /* ### �t�@�C�����Ȃ� */
		DBG_A(( "%s �̃t�@�C��������܂���", path ));
		neo_errno = E_SKL_INVARG;
		goto err;
	}
	ac = _neo_ArgMake(ARR_SIZE(av), av, execstr, 0, 0);

	/*
	 *	���ϐ�
	 */
	if (envstr) {
		nenv = _neo_ArgMake(ARR_SIZE(env), env, envstr, 0, 0);
		if (nenv > 0) {
			for (i=0; i < nenv; i++) {
				putenv(env[i]);
			}
		}
	}

	/*
	 *	fd�N���[�Y
	 */
	for (i = 3; i < NOFILE; i++)
		close(i);

	/*
	 *	�N��
	 */
	execv(path, av);

	/* 
	 * UNIX ERR 
	 */
	neo_errno = unixerr2neo();

	/* ### �N���G���[(exec) */
	DBG_A(( "%s �N���G���[�ł�", path ));

err:;
DBG_EXIT(-1);
	return( -1 );
}


/******************************************************************************
 * �֐���
 *		neo_term()
 * �@�\        
 *		�q�v���Z�X�ւ̏I���v��
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
neo_term()
{
	_neo_child_t	*pt;
	int		before;

DBG_ENT( M_SKL, "neo_term" );

	/*
	 *	SIGCLD �u���b�N
	 */
	before = sigblock( sigmask( SIGCLD ) );

	/*
	 *	�I���V�O�i�����M
	 */
        for( pt = (_neo_child_t *)_dl_head( &_child_entry );
			_dl_valid( &_child_entry, pt );
			pt = (_neo_child_t *)_dl_next( pt ) ) {

		kill( pt->pid, SIGTERM );
	}

	/*
	 *	�q�v���Z�X�I������
	 */
	while( _dl_head( &_child_entry ) )
		sigpause( 0 );
	
	/*
	 *	SIGCLD �u���b�N����
	 */
	sigsetmask( before );

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		neo_isterm()
 * �@�\        
 *		�v���Z�X�I���v���̊Ď�
 * �֐��l
 *	�v���L��: !0
 *	�v������: 0
 * ����
 ******************************************************************************/
int
neo_isterm()
{
DBG_ENT( M_SKL, "neo_isterm" );

	if( _isterm_flag )
		neo_errno = E_SKL_TERM;

DBG_EXIT(_isterm_flag);
	return( _isterm_flag );
}


/******************************************************************************
 * �֐���
 *		_neo_isterm_set()
 * �@�\        
 *		�v���Z�X�I���v���̃Z�b�g
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_isterm_set()
{
DBG_ENT( M_SKL, "_neo_isterm_set" );

	_isterm_flag = -1;

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_neo_child_remove()
 * �@�\        
 *		�q�v���Z�X�̃��X�g����̍폜
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_child_remove( pid )
int	pid;		/* I �v���Z�Xid	*/
{
	_neo_child_t	*pt;

DBG_ENT( M_SKL, "_neo_child_remove" );
DBG_C(( "pid=%d\n", pid ));

	for( pt = (_neo_child_t *)_dl_head( &_child_entry );
			_dl_valid( &_child_entry, pt );
			pt = (_neo_child_t *)_dl_next( pt ) ) {

		if( pt->pid == pid ) {
			_dl_remove( pt );
			neo_free( pt );
			break;
		}
	}

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_neo_child_clean()
 * �@�\        
 *		�q�v���Z�X�̃��X�g����̑S�폜
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static void
_neo_child_clean()
{
	_neo_child_t	*pt, *tpt;

DBG_ENT( M_SKL, "_neo_child_clean" );

	for( pt = (_neo_child_t *)_dl_head( &_child_entry );
			_dl_valid( &_child_entry, pt ); ) {

DBG_C(( "Clean struct (pid=%d)\n", pt->pid ));

		tpt = pt;
		pt = (_neo_child_t *)_dl_next( pt );

		_dl_remove( tpt );
		neo_free( tpt );
	}

DBG_EXIT(0);
}
