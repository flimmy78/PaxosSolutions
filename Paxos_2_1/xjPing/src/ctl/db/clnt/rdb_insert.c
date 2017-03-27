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
 *	rdb_insert.c
 *
 *	����                 ���R�[�h��}������
 * 
 ******************************************************************************/

#include	"neo_db.h"

/*****************************************************************************
 *
 *  �֐���	rdb_insert()
 *
 *  �@�\	���R�[�h���e�[�u���ɑ}������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 *  ����
 *		X 1. �{�łł͑}�����郌�R�[�h����Ɍ��肷��
 *		2. m items�w�肵���ꍇ�ɂ́A���R�[�h�̏d��
 *		 �o�^�̃`�F�b�N���s��
 *
 ******************************************************************************/
int
rdb_insert( td, n, rec, m, items, mode )
	r_tbl_t		*td;		/* �}����̃e�[�u���L�q�q	*/
	int		n;		/* �}�����������R�[�h��  	*/
	r_head_t	*rec;		/* �}�����郌�R�[�h�̊i�[�̈�	*/
	int		m;		/* �`�F�b�N����A�C�e���� 	*/
	r_item_name_t	items[];	/* �`�F�b�N����A�C�e����	*/
	int		mode;		/* ���[�h:R_SHARE,  R_EXCLUSIVE */
{
	r_req_insert_t	*reqp;
	r_res_insert_t	res;
	int		len;
	int		i, j;

DBG_ENT(M_RDB,"rdb_insert");

	/*
	 * �e�[�u���L�q�q�L�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	/*
	 * ���� m, n �̃`�F�b�N
	 * ( �{�łł́A���}�����R�[�h��n����Ɍ��肷��
	 */
	if( n < 0 || m < 0 || td->t_scm->s_n < m ) {
		neo_errno = E_RDB_INVAL;
		goto err1;
	}

	if ( mode & ~R_LOCK ) {
		neo_errno = E_RDB_INVAL;
		goto err1;
	}

	/*
	 * �A�C�e�����̃`�F�b�N(m���w�肵���ꍇ)
	 */
	if( m ) {
	  for( j = 0; j < m; j++ ) {
		for( i = 0; i < td->t_scm->s_n; i++ ) {
			if( !strcmp( items[j], td->t_scm->s_item[i].i_name ) ) {
				goto next;
			}
		}
		neo_errno = E_RDB_NOITEM;
		goto err1;
	  next:;
	  }
	}

	/*
	 * �v���p�P�b�g�p�o�b�t�@�𐶐�����
	 */
	len = sizeof(r_req_insert_t) 
		+ n*td->t_rec_size 
		+ m*sizeof(r_item_name_t);
	if( !(reqp = (r_req_insert_t*)neo_malloc(len) ) ) {
		neo_errno = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 * �v���p�P�b�g���쐬���A�T�[�o�֑���
	 */
	_rdb_req_head( (p_head_t*)reqp, R_CMD_INSERT, len );
	
	strncpy( reqp->i_name, td->t_name, sizeof(reqp->i_name) );
	reqp->i_mode	= mode;
	reqp->i_rec	= n;
	reqp->i_item	= m;

	bcopy( rec, reqp+1, n*td->t_rec_size );
	if( m )
		bcopy( items, (char*)(reqp+1) + n*td->t_rec_size, 
						m*sizeof(r_item_name_t) );
	if( p_request( TDTOPD(td), (p_head_t*)reqp, len ) )
		goto	err2;

	/*
	 * �����p�P�b�g����M�܂�
	 */
	len = sizeof(r_res_insert_t);
	if( p_response( TDTOPD(td), (p_head_t*)&res, &len ) )
		goto	err2;
	
	if( (neo_errno = res.i_head.h_err) )
		goto err2;

	/*
	 * ���[�J���̃e�[�u���̃��R�[�h�������X�V����
	 */
	td->t_rec_num	= res.i_rec_num;
	td->t_rec_used	= res.i_rec_used;
	bcopy( &res.i_rec, rec, sizeof(r_head_t) );

	neo_free( reqp );
DBG_EXIT(0);
	return( 0 );
err2: DBG_T("err2");
	neo_free( reqp );
err1:
DBG_EXIT(-1);
	return( -1 );
}
