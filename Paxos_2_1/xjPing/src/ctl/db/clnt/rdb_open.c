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
 *	rdb_open.c
 *
 *	����		R_CMD_CLOSE	�e�[�u���N���[�Y
 * 
 ******************************************************************************/

#include	"neo_db.h"

/*****************************************************************************
 *
 *  �֐���	rdb_open()
 *
 *  �@�\	�e�[�u�����I�[�v������
 * 
 *  �֐��l
 *     		����	�I�[�v�����ꂽ�e�[�u���̋L�q�q�̃A�h���X
 *		�ُ�	0 (NULL)
 *
 ******************************************************************************/
r_tbl_t	*
rdb_open( md, name )
	r_man_t		*md;		/* DB_MAN�L�q�q�̃A�h���X   */
	r_tbl_name_t	name;		/* �I�[�v�������e�[�u���� */
{
	r_tbl_t		*td;
	r_req_open_t	req;
	r_res_open_t	*resp;
	p_head_t	head;
	Msg_t	*pMsg;
	r_scm_t		*sp;

DBG_ENT(M_RDB,"rdb_open");

	/*
	 * �e�[�u���������`�F�b�N
	 */
	if ( strlen(name) > R_TABLE_NAME_MAX ) {
		neo_errno = E_RDB_INVAL;
		goto	err1;
	}

	/*
	 * DB_MAN�L�q�q�L�����`�F�b�N
	 *	(Multi_open to the same table is permitted)
	 */
	if( md->r_ident != R_MAN_IDENT ) {
		neo_errno = E_RDB_MD;
		goto err1;
	}

	/*
	 * �e�[�u���L�q�q�̈�𐶐�����
	 */
	if( !(td = (r_tbl_t *)_r_tbl_alloc() ) ) {
		neo_errno = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 * �e�[�u���I�[�v���v���p�P�b�g���쐬
	 * ���A�T�[�o�֑���
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_OPEN, sizeof(req) );

	bzero( req.o_name, sizeof(req.o_name) );
	strncpy( req.o_name, name, sizeof(req.o_name) );
	req.o_td = td;

	if( p_request( md->r_pd, (p_head_t*)&req, sizeof(req) ) )
		goto err2;

	/*
	 * �����p�P�b�g��҂����[�h�Ńs�[�N���āA
	 * ��M�o�b�t�@��p�ӂ���
	 */
	if( p_peek( md->r_pd, (p_head_t*)&head, sizeof(head), P_WAIT ) < 0 )
		goto err2;

	if( !(pMsg = p_responseByMsg( md->r_pd ) ) ) {
		goto err2;
	}
	resp	= (r_res_open_t*)MsgFirst( pMsg );
	if( (neo_errno = resp->o_head.h_err ) ) {
		goto err3;
	}

	/*
	 * �X�L�[�}���쐬����
	 */
	if( !(sp = (r_scm_t*)neo_malloc(sizeof(r_scm_t)
				+ sizeof(r_item_t)*(resp->o_n - 1) ) ) ) {
		neo_errno = E_RDB_NOMEM;
		goto err3;
	}
	bcopy( resp->o_items, sp->s_item, sizeof(r_item_t)*resp->o_n );
	sp->s_n	= resp->o_n;
	
	/*
	 * ���[�J���̃e�[�u���L�q�q�̏���ݒ肵�āA
	 * DB_MAN�L�q�q�֓o�^����
	 */
	strncpy( td->t_name, name, sizeof(td->t_name) );
	td->t_md	= md;
	td->t_rec_num	= resp->o_rec_num;
	td->t_rec_used	= resp->o_rec_used;
	td->t_rec_size	= resp->o_rec_size;

	td->t_scm 	= sp;

	_dl_insert( td, &md->r_tbl );
	
DBG_A(("REMOTE(td=0x%x,ud=0x%x\n", td->t_rtd, td->t_rud ));

	MsgDestroy( pMsg );

DBG_EXIT(td);
	return( td );

err3:	MsgDestroy( pMsg );
err2: DBG_T("err2");
	_r_tbl_free( td );
err1:
DBG_EXIT(0);
	return( 0 );
}


/*****************************************************************************
 *
 *  �֐���	rdb_close()
 *
 *  �@�\	�e�[�u�����N���[�Y����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
rdb_close( td )
	r_tbl_t	*td;	/* �N���[�Y���悤�Ƃ���e�[�u���L�q�q�A�h���X */
{
	r_req_close_t	req;
	r_res_close_t	*resp;
	Msg_t			*pMsg;

DBG_ENT(M_RDB,"rdb_close");

	/*
	 * �e�[�u���L�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	/*
	 * �N���[�Y�v���p�P�b�g���쐬���A�T�[�o���֑���
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_CLOSE, sizeof(req) );

	strncpy( req.c_name, td->t_name, sizeof(req.c_name) );

	if( p_request( td->t_md->r_pd, (p_head_t*)&req, sizeof(req) ) )
		goto err1;

	/*
	 * �����p�P�b�g����M�܂�
	 */
	if( !(pMsg = p_responseByMsg( td->t_md->r_pd ) ) ) {
		goto err1;
	}
	resp	= (r_res_close_t*)MsgFirst( pMsg );
#ifdef	ZZZ
#else
	if( (neo_errno = resp->c_head.h_err ) ) {
		goto err2;
	}
#endif
	/*
	 * �e�[�u����DB_MAN�L�q�q�̃e�[�u�����X�g����O���āA
	 * �̈���������
	 */
	_dl_remove( td );
#ifdef	ZZZ
#else
	td->t_ident	= 0;
#endif
	_r_tbl_free( td );

	MsgDestroy( pMsg );
DBG_EXIT(0);
	return( 0 );
#ifdef	ZZZ
#else
err2:
	MsgDestroy( pMsg );
#endif
err1:
DBG_EXIT(-1);
	return( -1 );
}

#ifdef	ZZZ
int
rdb_close_client( r_tbl_t *td )
{
	/*
	 * �e�[�u����DB_MAN�L�q�q�̃e�[�u�����X�g����O���āA
	 * �̈���������
	 */
	_dl_remove( td );
	_r_tbl_free( td );
	return( 0 );
}
#endif
int
rdb_close_client( td )
	r_tbl_t	*td;	/* �N���[�Y���悤�Ƃ���e�[�u���L�q�q�A�h���X */
{
	r_req_close_client_t	req;
	r_res_close_client_t	*resp;
	Msg_t			*pMsg;

DBG_ENT(M_RDB,"rdb_close_client");

	/*
	 * �e�[�u���L�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	/*
	 * �N���[�Y�v���p�P�b�g���쐬���A�T�[�o���֑���
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_CLOSE_CLIENT, sizeof(req) );

	strncpy( req.c_name, td->t_name, sizeof(req.c_name) );

	if( p_request( td->t_md->r_pd, (p_head_t*)&req, sizeof(req) ) )
		goto err1;

	/*
	 * �����p�P�b�g����M�܂�
	 */
	if( !(pMsg = p_responseByMsg( td->t_md->r_pd ) ) ) {
		goto err1;
	}
	resp	= (r_res_close_client_t*)MsgFirst( pMsg );
#ifdef	ZZZ
#else
	if( (neo_errno = resp->c_head.h_err ) ) {
		goto err2;
	}
#endif
	/*
	 * �e�[�u����DB_MAN�L�q�q�̃e�[�u�����X�g����O���āA
	 * �̈���������
	 */
	_dl_remove( td );
#ifdef	ZZZ
#else
	td->t_ident	= 0;
#endif
	_r_tbl_free( td );

	MsgDestroy( pMsg );
DBG_EXIT(0);
	return( 0 );
#ifdef	ZZZ
#else
err2:
	MsgDestroy( pMsg );
#endif
err1:
DBG_EXIT(-1);
	return( -1 );
}
