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
 *	skl_eplg.c
 *
 *	����	�G�s���[�O����(�o�^�^�폜)
 * 
 ******************************************************************************/

#include	"skl_eplg.h"

/*
 *	�G�s���[�O���� �Ǘ����X�g �G���g��
 */
static _dlnk_t	_epilogue_entry;


/******************************************************************************
 * �֐���
 *		_neo_init_epilog()
 * �@�\        
 *		�G�s���[�O���� ������
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_init_epilog()
{
DBG_ENT( M_SKL, "_neo_init_epilog" );

	_dl_init( &_epilogue_entry );

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_neo_exec_epilog()
 * �@�\        
 *		�G�s���[�O���� ���s
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_exec_epilog()
{
	_neo_epilog_t	*pt, *tpt;

DBG_ENT( M_SKL, "_neo_exec_epilog" );

        for( pt = (_neo_epilog_t *)_dl_head( &_epilogue_entry );
			_dl_valid( &_epilogue_entry, pt ); ) {
		
DBG_C(( "exec class=%d\n", pt->class ));
		(*pt->func)();

		tpt = pt;
		pt = (_neo_epilog_t *)_dl_next( pt );

		_dl_remove( tpt );
		neo_free( tpt );
	}

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		neo_set_epilog()
 * �@�\        
 *		�G�s���[�O���� �o�^
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_set_epilog( class, func )
int	class;		/* I �N���X	*/
int	(*func)();	/* I �㏈���֐�	*/
{
	_neo_epilog_t	*pt;

DBG_ENT( M_SKL, "neo_set_epilog" );
DBG_C(( "class=%d\n", class ));

	/*
	 *	�����̃`�F�b�N
	 */
	if( (class < 0) || (class >= M_APL) ) {
		neo_errno = E_SKL_INVARG;
		goto err;
	}

        for( pt = (_neo_epilog_t *)_dl_head( &_epilogue_entry );
			_dl_valid( &_epilogue_entry, pt );
			pt = (_neo_epilog_t *)_dl_next( pt ) ) {

		if( pt->class == class ) {
DBG_T( "ERR: Duplicate class" );
			neo_errno = E_SKL_INVARG;
			goto err;
		}
	}


	/*
	 *	�o�^���ݒ�
	 */
	if( !(pt = (_neo_epilog_t *)neo_malloc( sizeof(_neo_epilog_t) )) ) {
		goto err;
	}

	pt->class = class;
	pt->func  = func;

	/*
	 *	���X�g�̍Ō�ɒǉ�
	 */
	_dl_insert( pt, &_epilogue_entry );
	

DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}


/******************************************************************************
 * �֐���
 *		neo_del_epilog()
 * �@�\        
 *		�G�s���[�O���� �폜
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_del_epilog( class )
int	class;		/* I �N���X	*/
{
	_neo_epilog_t	*pt;

DBG_ENT( M_SKL, "neo_del_epilog" );
DBG_C(( "class=%d\n", class ));

        for( pt = (_neo_epilog_t *)_dl_head( &_epilogue_entry );
			_dl_valid( &_epilogue_entry, pt );
			pt = (_neo_epilog_t *)_dl_next( pt ) ) {

		if( pt->class == class ) {
			_dl_remove( pt );
			neo_free( pt );
			goto ok;
		}
	}

DBG_T( "ERR: Not found class" );
	neo_errno = E_SKL_INVARG;
	goto err;

ok:;
DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}

