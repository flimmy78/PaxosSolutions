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
 *	rdb_find.c
 *
 *	����	���R�[�h�̌���
 *			       R_CMD_REWIND
 *				�J�[�\����擪�ɖ߂�
 * 
 ******************************************************************************/

#include	"neo_db.h"

/******************************************************************************
 * �֐���
 *		rdb_find()
 * �@�\        
 *		���R�[�h�̌���
 * �֐��l
 *      ����: �擾���R�[�h��
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
rdb_find( td, n, rp, mode )
	r_tbl_t		*td;	/* I �e�[�u���L�q�q	*/
	int		n;	/* I ���R�[�h���� �ő吔 */
	r_head_t	*rp;	/* O ���R�[�h�o�b�t�@	*/
	int		mode;	/* I �r��/�҂� ���[�h	*/
{
	p_id_t		*pd;
	r_req_find_t	req;
	r_res_find_t	*resp;
	Msg_t	*pMsg;
	r_res_find_t	head;
	int		total, size;
	char	*bufp	= (char*)rp;

DBG_ENT(M_RDB,"rdb_find");

	/*
	 *	�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}
	if( n <= 0 ) {
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
	total	= 0;

	/*
	 *	���N�G�X�g���̐ݒ�
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_FIND, sizeof(req) );
	
	strncpy( req.f_name, td->t_name, sizeof(req.f_name) );
	req.f_num	= n;
	req.f_mode	= mode;

	/*
	 *	���N�G�X�g
	 */
again:
	if( p_request( pd = TDTOPD(td), (p_head_t*)&req, sizeof(req) ) )
		goto	err2;
loop:
	/*
	 *	���X�|���X
	 *	( R_CMD_FIND �̃��v���C�p�P�b�g�͉ϒ� )
	 */
	if( p_peek( pd, (p_head_t*)&head, sizeof(head), P_WAIT ) < 0 )
		goto err2;
	if( (neo_errno = head.f_head.h_err) ) {

		/* discard */
		pMsg = p_responseByMsg( pd );
		MsgDestroy( pMsg );

		if( (neo_errno != E_RDB_REC_BUSY) || (mode&R_NOWAIT)  )
			goto err2;

		/*
		 *	�҂����[�h�� E_RDB_REC_BUSY �̎���, ���R�[�h�C�x���g�҂�
		 */
		if( rdb_event( td, R_EVENT_REC, -1 ) )	/* wait at cursor */
			goto err2;
		goto again;
	}

	if( !(pMsg = p_responseByMsg( pd ) ) ) {
		goto err2;
	}
	resp	= (r_res_find_t*)MsgFirst( pMsg );

	/*
	 *	�擾�������R�[�h�f�[�^�̃R�s�[ 
	 */
	if( resp->f_num ) {

		size	= td->t_rec_size*resp->f_num;
		bcopy( resp+1, bufp, size );

		bufp	+= size;
		total	+= resp->f_num;
	}
	MsgDestroy( pMsg );

	/*
	 *	���v���C�̑��M�� R_PAGE_SIZE �����R�[�h�o�b�t�@�̍ő�l�Ƃ���
	 *	�]���āA�P��ő��M�ł��Ȃ����́A�������đ�����
	 *	�܂��A�c�肪����Ƃ��́Ahead.f_more �� 1 ���ݒ肳��Ă���
	 */
	if( head.f_more )
		goto loop;

DBG_EXIT(total);
	return( total );

err2: DBG_T("err2");
	neo_free( resp );
err1:
DBG_EXIT(-1);
	return( -1 );
}

/******************************************************************************
 * �֐���
 *		rdb_rewind()
 * �@�\        
 *		�J�[�\����擪�ɖ߂�
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
rdb_rewind( td )
	r_tbl_t		*td;	/* I �e�[�u���L�q�q	*/
{
	r_req_rewind_t	req;
	r_res_rewind_t	*resp;
	Msg_t	*pMsg;
	p_id_t		*pd;

DBG_ENT(M_RDB,"rdb_rewind");

	/*
	 *	�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}
	/*
	 *	���N�G�X�g���ݒ�
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_REWIND, sizeof(req) );
	
	strncpy( req.r_name, td->t_name, sizeof(req.r_name) );
	/*
	 *	���N�G�X�g
	 */
	if( p_request( pd = TDTOPD(td), (p_head_t*)&req, sizeof(req) ) )
		goto	err1;

	/*
	 *	���X�|���X
	 */
	if( !(pMsg = p_responseByMsg( pd ) ) ) {
		goto	err1;
	}
	resp	= (r_res_rewind_t*)MsgFirst( pMsg );
	if( (neo_errno = resp->r_head.h_err ) ) {
		goto err2;
	}
	MsgDestroy( pMsg );

DBG_EXIT(0);
	return( 0 );

err2:	MsgDestroy( pMsg );
err1:
DBG_EXIT(-1);
	return( -1 );
}


int
rdb_seek( td, seek )
	r_tbl_t		*td;	/* I �e�[�u���L�q�q	*/
	int		seek;
{
	r_req_seek_t	req;
	r_res_seek_t	*resp;
	Msg_t			*pMsg;
	p_id_t		*pd;

DBG_ENT(M_RDB,"rdb_seek");

	/*
	 *	�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}
	/*
	 *	���N�G�X�g���ݒ�
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_SEEK, sizeof(req) );
	
	strncpy( req.r_name, td->t_name, sizeof(req.r_name) );
	req.r_seek	= seek;

	/*
	 *	���N�G�X�g
	 */
	if( p_request( pd = TDTOPD(td), (p_head_t*)&req, sizeof(req) ) )
		goto	err1;

	/*
	 *	���X�|���X
	 */
	if( !(pMsg = p_responseByMsg( pd ) ) ) {
		goto	err1;
	}
	resp	= (r_res_seek_t*)MsgFirst( pMsg );
	if( (neo_errno = resp->r_head.h_err ) ) {
		goto err2;
	}
	MsgDestroy( pMsg );

DBG_EXIT(0);
	return( 0 );

err2:	MsgDestroy( pMsg );
err1:
DBG_EXIT(-1);
	return( -1 );
}
