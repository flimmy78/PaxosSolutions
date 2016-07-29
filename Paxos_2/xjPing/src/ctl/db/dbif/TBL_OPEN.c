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
 *	TBL_OPEN.c
 *
 *	����	TBL_OPEN : �e�[�u���I�[�v��
 *			TBL_CLOSE: �e�[�u���N���[�Y
 * 
 ******************************************************************************/

#include	"TBL_RDB.h"

/*****************************************************************************
 *
 *  �֐���	TBL_OPEN()
 *
 *  �@�\	�e�[�u�����I�[�v������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
TBL_OPEN( td, name )
	r_tbl_t		*td;		/* �e�[�u���L�q�q	 */
	r_tbl_name_t	name;		/* �I�[�v���Ώۃe�[�u����*/
{
	my_tbl_t	*md;
	int		err;
	uint64_t	Off;

DBG_ENT(M_RDB,"TBL_OPEN");
DBG_A(("name[%s]\n", name ));

	strncpy( td->t_name, name, sizeof(td->t_name) );

	/*
	 * DB�e�[�u���L�q�q�G���A�𐶐�����
	 */
	if( !( md = my_tbl_alloc() ) ) {
		err	= E_RDB_NOMEM;
		goto err1;
	}
	md->m_td	= td;
	td->t_tag	= (void *)md;

	/*
	 * DB�e�[�u�����I�[�v������
	 */

/*****
	if( (err = fd_open( md )) )
		goto err2;
****/

	/*
	 * ���R�[�h���A�A�C�e�����Ȃǂ̃t�@�C��
	 * �w�b�_�����擾����
	 */
	if( my_head_read( md, &Off ) )
		goto err2;
	if( my_item_read( md, &Off ) )
		goto err2;
	if( my_depend_read( md, &Off ) )
		goto err2;

	/*
	 * �f�[�^���̃I�t�Z�b�g���Z�o����
	 */
	md->m_data_off	= my_data_off( md );

DBG_EXIT(0);
	return( 0 );

err2: DBG_T("err2");
	my_tbl_free( md );
err1: DBG_T("err1");
	neo_errno = err;
DBG_EXIT(-1);
	return( -1 );
}

#ifdef	ZZZ
#else
int
TBL_OPEN_SET( r_tbl_t *td )
{
	my_tbl_t	*md;
	int		err;

	/*
	 * DB�e�[�u���L�q�q�G���A�𐶐�����
	 */
	if( !( md = my_tbl_alloc() ) ) {
		err	= E_RDB_NOMEM;
		goto err1;
	}
	md->m_td	= td;
	td->t_tag	= (void *)md;

	md->m_data_off	= my_data_off( md );

	return( 0 );
err1:
	neo_errno = err;
	return( -1 );
}
#endif


/*****************************************************************************
 *
 *  �֐���	TBL_CLOSE()
 *
 *  �@�\	�e�[�u�����N���[�Y����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
TBL_CLOSE( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
	my_tbl_t	*md;
	int		err;

DBG_ENT(M_RDB,"TBL_CLOSE");

	md	= TDTOMY( td );

	if( !md )
		goto ret;

	/*
	 * �t�@�C���w�b�_����USED���R�[�h�����X�V����
	 */
	if( (err = my_head_update( md )) )
		goto err1;

	/*
	 * DB�e�[�u�����N���[�Y����
	 */
/***
	if( (err = fd_close( md )) )
		goto err1;
***/

	/*
	 * DB�e�[�u���L�q�q�G���A���������
	 */
	my_tbl_free( md );

ret:
DBG_EXIT(0);
	return( 0 );
err1:
	neo_errno = err;
DBG_EXIT(-1);
	return( -1 );
}


/*****************************************************************************
 *
 *  �֐���	TBL_CNTL()
 *
 *  �@�\	
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1	
 *
 ******************************************************************************/
int
TBL_CNTL( td, cmd, arg )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
	int	cmd;		
	char	*arg;
{
	my_tbl_t	*md;
	uint64_t	Off;

DBG_ENT(M_RDB,"TBL_CNTL");

	md	= TDTOMY( td );

	if( !md )
		goto ret;

	switch( PNT_INT32(arg) ) {
		case 0:
			/*
			 * �t�@�C���w�b�_���̏����X�V����
			 * (modified, used)
			 */
			if( (neo_errno = my_head_update( md )) )
				goto err1;
			break;
		case 1:	/* reload may not be used ? */
			if( (neo_errno = my_head_read( md, &Off )) )
				goto err1;
			break;
	}

ret:
DBG_EXIT( 0 );
	return( 0 );

err1:
DBG_EXIT(-1);
	return( -1 );
}
