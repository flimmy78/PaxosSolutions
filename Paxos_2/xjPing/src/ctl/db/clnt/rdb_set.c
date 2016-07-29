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
 *	rdb_set.c
 *
 *	����	  �A�C�e���l�̐ݒ�ƎQ��
 * 
 ******************************************************************************/

#include	"neo_db.h"

/*****************************************************************************
 *
 *  �֐���	rdb_set()
 *
 *  �@�\	���R�[�h�̎w��A�C�e���l�̐ݒ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
rdb_set( td, rp, item, data )
	r_tbl_t		*td;		/* �e�[�u���L�q�q		*/
	r_head_t	*rp;		/* �ݒ�惌�R�[�h�o�b�t�@	*/ 
	char		*item;		/* �ݒ��A�C�e����		*/
	char		*data;		/* �ݒ�p�f�[�^�l		*/
{
	r_item_t	*ip;
	int		i;
	char	*rec = (char*)rp;

DBG_ENT(M_RDB,"rdb_set");

	/*
	 * �e�[�u���L�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	/*
	 * �w��A�C�e�����X�L�[�}�̒��ɑ{���o���A
	 * ���R�[�h���̃I�t�Z�b�g�l���擾����
	 */
	for( i = 0; i < td->t_scm->s_n; i++ ) {
		ip = &td->t_scm->s_item[i];
		if( !strcmp( item, ip->i_name ) )
			goto find;
	}
	neo_errno = E_RDB_NOITEM;

	goto err1;
find:
	/*
	 * �f�[�^�����R�[�h�ɐݒ肷��
	 */
	bcopy( data, rec + ip->i_off, ip->i_len ); 

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(-1);
	return( -1 );
}


/*****************************************************************************
 *
 *  �֐���	rdb_get()
 *
 *  �@�\	���R�[�h�̂���X�L�[�}�l���擾����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
rdb_get( td, rp, item, data, lenp )
	r_tbl_t		*td;		/* �e�[�u���L�q�q	*/
	r_head_t	*rp;		/* ���R�[�h�o�b�t�@	*/
	char		*item;		/* �A�C�e����		*/
	char		*data;		/* �A�C�e���n�i�[�G���A	*/
	int		*lenp;		/* �擾��		*/
{
	r_item_t	*ip;
	int		i;
	char	*rec = (char*)rp;

DBG_ENT(M_RDB,"rdb_get");

	/*
	 * �e�[�u���L�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	/*
	 * �w�肵���A�C�e�����X�L�[�}�̒�����{���o���A
	 * ���R�[�h���̃I�t�Z�b�g�l���擾����
	 */
	for( i = 0; i < td->t_scm->s_n; i++ ) {
		ip = &td->t_scm->s_item[i];
		if( !strcmp( item, ip->i_name ) )
			goto find;
	}
	neo_errno = E_RDB_NOITEM;

	goto err1;
find:
	/*
	 * ���R�[�h�̊Y���X�L�[�}�l�����o��
	 */
	bcopy( rec + ip->i_off, data, *lenp = ip->i_len ); 

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(-1);
	return( -1 );
}
