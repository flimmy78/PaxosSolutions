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
 *	rdb_update.c
 *
 *	����		���R�[�h���X�V����
 * 
 ******************************************************************************/

#include	"neo_db.h"

/*****************************************************************************
 *
 *  �֐���	rdb_update()
 *
 *  �@�\	���R�[�h���X�V����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
rdb_update( td, n, rec, mode )
	r_tbl_t		*td;		/* �e�[�u���L�q�q	*/
	int		n;		/* �X�V���郌�R�[�h��	*/
	r_head_t	*rec;		/* �X�V�p�V���R�[�h	*/
	int		mode;		/* ���[�h		*/
{
	r_req_update_t	*reqp;
	r_res_update_t	res;
	int		len, len1;

DBG_ENT(M_RDB,"rdb_update");

	/*
	 * �e�[�u���L�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	if ( ( n <= 0 )  || ( mode & ~(R_LOCK|R_WAIT|R_NOWAIT) ) ) {
		neo_errno = E_RDB_INVAL;
		goto err1;
	}
	/*
	 * �v���p�P�b�g�p�o�b�t�@�G���A�𐶐�����
	 */
	len = sizeof(r_req_update_t) + n * td->t_rec_size ;
	if( !(reqp = (r_req_update_t*)neo_malloc(len) ) ) {
		neo_errno = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 * �v���p�P�b�g���쐬����
	 */
	_rdb_req_head( (p_head_t*)reqp, R_CMD_UPDATE, len );
	
	strncpy( reqp->u_name, td->t_name, sizeof(reqp->u_name) );
	reqp->u_mode	= mode;
	reqp->u_numbers	= n;

	bcopy( rec, reqp+1, n * td->t_rec_size );

	/*
	 * �v���p�P�b�g�𑗐M����
	 */
again:
	if( p_request( TDTOPD(td), (p_head_t*)reqp, len ) )
		goto	err2;

	/*
	 * �����p�P�b�g����M�܂�
	 */
	len1 = sizeof(r_res_update_t);
	if( p_response( TDTOPD(td), (p_head_t*)&res, &len1 ) )
		goto	err2;
	
	/*
	 * �����p�P�b�g�̃w�b�_���G���[�ԍ��̃`�F�b�N
	 */
	neo_errno = res.u_head.h_err ;

	if( neo_errno ) {

		/*
	  	 * �X�V�Ώۃ��R�[�h�����b�N�������[�h������
	  	 * NOWAIT���w�肵���ꍇ�ɂ́A�G���[�Ŗ߂�
		 */
		if( neo_errno != E_RDB_REC_BUSY
			|| mode & R_NOWAIT ) {

			goto err2;

		/*
		 * �Ď��s���� 
		 */
		} else {
			if( rdb_event( td, R_EVENT_REC, res.u_rec.r_abs ) )
				goto err2;

			goto again;
		}
	}

	/*
	 *	record to user
	 */

	/***
	bcopy(&res.u_rec, rec, sizeof(r_head_t) );
	***/
	neo_free( reqp );
DBG_EXIT(0);
	return( 0 );
err2: DBG_T("err2");
	neo_free( reqp );
err1:
DBG_EXIT(-1);
	return( -1 );
}
