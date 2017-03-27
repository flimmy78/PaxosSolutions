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
 *	rdb_direct.c
 *
 *	����	DB_MAN library R_CMD_DIRECT
 *				���R�[�h��Δԍ��Ń��R�[�h���擾����
 * 
 ******************************************************************************/

#include	"neo_db.h"

/******************************************************************************
 * �֐���
 *		rdb_direct()
 * �@�\        
 *		���R�[�h��Δԍ��Ń��R�[�h���擾����
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
rdb_direct( td, abs, bufp, mode )
	r_tbl_t		*td;	/* I �e�[�u���L�q�q	*/
	int		abs;	/* I ���R�[�h��Δԍ�	*/
	char		*bufp;	/* O ���R�[�h�o�b�t�@	*/
	int		mode;	/* I �r�����[�h		*/
{
	p_id_t		*pd;
	r_req_direct_t	req;
	r_res_direct_t	*resp;
	r_res_direct_t	head;
	int		len;

DBG_ENT(M_RDB,"rdb_direct");

	/*
	 *	�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}
	if( abs < 0 ) {
		neo_errno = E_RDB_INVAL;
		goto err1;
	}
	if( mode & ~(R_EXCLUSIVE|R_SHARE|R_WAIT|R_NOWAIT) ) {
		neo_errno = E_RDB_INVAL;
		goto err1;
	}
	if( (mode & R_EXCLUSIVE) && (mode & R_SHARE) ) {
		neo_errno = E_RDB_INVAL;
		goto err1;
	}

	/*
	 *	���X�|���X�p�o�b�t�@�̃������A���b�N
	 *	(R_CMD_DIRECT �p�P�b�g�͉ϒ�)
	 */
	len = sizeof(r_res_direct_t) + td->t_rec_size;
	if( !(resp = (r_res_direct_t *)neo_malloc( len )) ) {
		neo_errno = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 *	���N�G�X�g�p�P�b�g�̏��ݒ�
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_DIRECT, sizeof(req) );
	
	strncpy( req.d_name, td->t_name, sizeof(req.d_name) );
	req.d_abs	= abs;
	req.d_mode	= mode;

	/*
	 *	���N�G�X�g
	 */
again:
	if( p_request( pd = TDTOPD(td), (p_head_t*)&req, sizeof(req) ) )
		goto	err2;

	/*
	 *	���X�|���X(R_CMD_DIRECT �p�P�b�g�͉ϒ�)
	 */
	if( p_peek( pd, (p_head_t*)&head, sizeof(head), P_WAIT ) < 0 )
		goto err1;
	if( (neo_errno = head.d_head.h_err) ) {

		len = sizeof(head);
		p_response( pd, (p_head_t*)&head, &len );

		if( (neo_errno != E_RDB_REC_BUSY) || (mode&R_NOWAIT)  )
			goto err2;

		/*
		 *	�r���ł��Ȃ����́A���R�[�h�C�x���g��҂�
		 */
		if( rdb_event( td, R_EVENT_REC, abs ) )
			goto err2;
		goto again;
	}

	len = sizeof(r_res_direct_t) + td->t_rec_size;
	if( p_response( pd, (p_head_t*)resp, &len ) )
		goto	err2;

	/*
	 *	�f�[�^�R�s�[
	 */
	bcopy( resp+1, bufp, td->t_rec_size );

	neo_free( resp );
	
DBG_EXIT(0);
	return( 0 );

err2: DBG_T("err2");
	neo_free( resp );
err1:
DBG_EXIT(-1);
	return( -1 );
}
