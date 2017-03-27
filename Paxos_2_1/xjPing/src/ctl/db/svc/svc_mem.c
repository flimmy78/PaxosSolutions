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
 *	svc_mem.c
 *
 *	����		���R�[�h�ɂ��Ẵ������Ǘ�
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"

/*****************************************************************************
 *
 *  �֐���	rec_cntl_alloc()
 *
 *  �@�\	���R�[�h�Ǘ��\���̂̃A���P�b�g
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
rec_cntl_alloc( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
	int	err;

DBG_ENT(M_RDB,"rec_cntl_alloc");

	/*
	 * ���R�[�h�����̃��R�[�h�Ǘ��\���̗p
	 * �������𐶐����A�e�[�u���L�q�q�Ɍq����
	 */
	if( td->t_rec_num > 0 && !(td->t_rec = 
		(r_rec_t *)neo_zalloc(sizeof(r_rec_t)*td->t_rec_num)) ) {

		err = E_RDB_NOMEM;
		goto err1;
	}

DBG_A(("rec=0x%x(num=%d)\n", td->t_rec, td->t_rec_num ));
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(err);
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	rec_cntl_free()
 *
 *  �@�\	���R�[�h�Ǘ��\���̂̃t���[
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
rec_cntl_free( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
DBG_ENT(M_RDB,"rec_cntl_free");
DBG_A(("rec=0x%x\n", td->t_rec));
	if( td->t_rec )
		neo_free( td->t_rec );
DBG_EXIT(0);
	return( 0 );
}


/*****************************************************************************
 *
 *  �֐���	rec_buf_alloc()
 *
 *  �@�\	���R�[�h�o�b�t�@�������̃A���P�b�g
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
rec_buf_alloc( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
	r_head_t	*dp;
	r_rec_t		*rp;
	int		i;
	int		err;

DBG_ENT(M_RDB,"rec_buf_alloc");

	if( td->t_rec_num <= 0 )	return( 0 );
	/*
	 * ���R�[�h�����̃��R�[�h�i�[�p�o�b�t�@�𐶐�����
	 */
	if( !(dp =
		(r_head_t *)neo_zalloc(((size_t)td->t_rec_size)*td->t_rec_num)) ) {

		err = E_RDB_NOMEM;
		goto err1;
	}
DBG_A(("rec_num=%d,size=%d,bufp=0x%x\n", td->t_rec_num, td->t_rec_size, dp));

	/*
	 * �e���R�[�h�̈�̃w�b�_�����N���A���A�̈�̃A�h���X
	 * ��Ή����郌�R�[�h�́��Ǘ��\���̂ɐݒ肵�A�Ǘ��\����
	 * �������A�^�b�`���t���O������
	 */
	for( i = 0, rp = td->t_rec; i < td->t_rec_num;
			i++, dp = (r_head_t*)((char*)dp + td->t_rec_size) ) {

		bzero( dp, sizeof(r_head_t) );
		rp[i].r_head	= dp;
		rp[i].r_stat	|= R_REC_BUF;
	}

	/*
	 * �擪���R�[�h�Ǘ��\���̂����̃o�b�t�@�t���[�t���O������
	 */
	rp[0].r_stat	|= R_REC_FREE;
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(err);
	return( err );
}

/*****************************************************************************
 *
 *  �֐���	rec_buf_free()
 *
 *  �@�\	���R�[�h�i�[�G���A�̃t���[
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
rec_buf_free( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
	int	err;
	int	i;
	r_rec_t	*rp;

DBG_ENT(M_RDB,"rec_buf_free");

	rp = td->t_rec;
	if( !rp ) {
		DBG_EXIT(0);
		return( 0 );
	}

	/*
	 * �擪���R�[�h�o�b�t�@�t���[�w�����Ȃ����
	 * �G���[�ƂȂ�
	 */
	if( !(rp[0].r_stat & R_REC_FREE) ) {
		err = E_RDB_CONFUSED;
		goto err1;
	}

	for( i = 0; i < td->t_rec_num; i++ ) {
		/*
		 * ���R�[�h�i�[�G���A���������
		 */
		if( rp[i].r_stat & R_REC_FREE )
			neo_free( rp[i].r_head );

		/*
		 * ���R�[�h�Ǘ��\���̂̃������A�^�b�`���t���O
		 * �ƃo�b�t�@�t���[�t���O���N���A����
		 */
		rp[i].r_stat &= ~(R_REC_FREE|R_REC_BUF);
	}
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(err);
	return( err );
}
