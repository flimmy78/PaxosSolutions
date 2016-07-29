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
 *	rdb_sort.c
 *
 *	����	 	�e�[�u�����\�[�g����
 * 
 ******************************************************************************/

#include	"neo_db.h"

/*****************************************************************************
 *
 *  �֐���	rdb_sort()
 *
 *  �@�\	�e�[�u���̃��R�[�h���\�[�g����
 * 
 *  �֐��l
 *     		����	 0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
rdb_sort( td, n, sort )
	r_tbl_t		*td;		/* �\�[�g����e�[�u���L�q�q */
	int		n;		/* �\�[�g�p�L�[�̐�	    */
	r_sort_t	sort[];		/* �\�[�g�p�L�[		    */
{
	r_item_t	*ip;
	int		i, j;
	r_req_sort_t	*reqp;
	r_res_sort_t	res;
	int		len;

DBG_ENT(M_RDB,"rdb_sort");

	/*
	 * �e�[�u���L�q�q�L�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	/*
	 * �\�[�g�p�L�[�̐���100�ȓ��Ɍ��肷��
	 */
	if( n < 0 || td->t_scm->s_n < n ) {
		neo_errno = E_RDB_INVAL;
		goto err1;
	}

	/*
	 * �e�\�[�g�p�L�[�̃A�C�e�����ƃ\�[�g�����w����`�F�b�N����
	 */
	for( j = 0; j < n ; j++ ) {
		if( sort[j].s_order != R_ASCENDANT 
			&& sort[j].s_order != R_DESCENDANT ) {

			neo_errno = E_RDB_INVAL;
			goto err1;
		}
		for( i = 0; i < td->t_scm->s_n; i++ ) {
			ip = &td->t_scm->s_item[i];
			if( !strcmp( sort[j].s_name, ip->i_name ) )
				goto find;
		}
		neo_errno = E_RDB_NOITEM;

		goto err1;
	find:
		continue;
	}

	/*
	 * �v���p�P�b�g�p�G���A�𐶐�����
	 */
	len = sizeof(r_req_sort_t) + n*sizeof(r_sort_t);
	if( !(reqp = (r_req_sort_t *)neo_malloc(len) ) ) {
		neo_errno = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 * �v���p�P�b�g���쐬���A�T�[�o�֑���
	 */
	_rdb_req_head( (p_head_t*)reqp, R_CMD_SORT, len );

	strncpy( reqp->s_name, td->t_name, sizeof(reqp->s_name) );
	reqp->s_n 	= n;
	bcopy( sort, reqp+1, n*sizeof(r_sort_t) );

	if( p_request( TDTOPD(td), (p_head_t*)reqp, len ) )
		goto err2;

	/*
	 * �����p�P�b�g��M�҂�
	 */
	len = sizeof(r_res_sort_t);
	if( p_response( TDTOPD(td), (p_head_t*)&res, &len )
				||( neo_errno = res.s_head.h_err) )
		goto err2;

	/*
	 * �v���p�P�b�g�p�G���A���������
	 */
	neo_free( reqp );

DBG_EXIT(0);
	return( 0 );

err2: DBG_T("err2");
	neo_free( reqp );
err1:
DBG_EXIT(-1);
	return( -1 );
}
