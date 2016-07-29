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
 *	rdb_misc.c
 *
 *	����	���̑����� : ���R�[�h�T�C�Y�A���R�[�h���̎Q�Ƌy��
 *				     �e�[�u���L�q�q�A�I�[�v�����[�U�\����
 *				     �̐����̕ԋp
 * 
 ******************************************************************************/

#include	"neo_db.h"

/*****************************************************************************
 *
 *  �֐���	rdb_size()
 *
 *  �@�\	���R�[�h�̃T�C�Y��₢���킹��
 * 
 *  �֐��l
 *     		����	���R�[�h�̃T�C�Y
 *		�ُ�	-1
 *
 ******************************************************************************/
int
rdb_size( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	return( td->t_rec_size );
err1:
	return( -1 );
}


/*****************************************************************************
 *
 *  �֐���	rdb_number()
 *
 *  �@�\	���R�[�h����₢���킹��
 * 
 *  �֐��l
 *     		����	���R�[�h��
 *		�ُ�	-1
 *
 *   ��	 	�����̃��[�h
 *			0:	���R�[�h����
 *			1:	USED���R�[�h��
 *			2:	���g�p���R�[�h��
 *
 ******************************************************************************/
int
rdb_number( td, mode )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
	int	mode;		/* ���[�h: 0, 1, 2 	*/
{
	int			ret;
	p_id_t			*pd;
	r_req_rec_nums_t	req;
	r_res_rec_nums_t	res;
	int			len;

DBG_ENT(M_RDB,"rdb_number");

	/*
	 *	�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
		neo_errno = E_RDB_TD;
		goto err1;
	}

	/*
	 *	���N�G�X�g�p�P�b�g�̏��ݒ�
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_REC_NUMS, sizeof(req) );
	
	strncpy( req.r_name, td->t_name, sizeof(req.r_name) );
	/*
	 *	���N�G�X�g
	 */
	if( p_request( pd = TDTOPD(td), (p_head_t*)&req, sizeof(req) ) )
		goto	err1;

	/*
	 *	���X�|���X
	 */
	len = sizeof( res ) ;
	if( p_response( pd, (p_head_t*)&res, &len ) )
		goto	err1;
	td->t_rec_num  = res.r_total;
	td->t_rec_used = res.r_used;

	switch( mode ) {
		case R_ALL_REC:
			ret = td->t_rec_num;
			break;
		case R_USED_REC:
			ret = td->t_rec_used;
			break;
		case R_UNUSED_REC:
			ret = td->t_rec_num - td->t_rec_used;
			break;
		default:
			neo_errno = E_RDB_INVAL;
			goto err1;
	}

DBG_EXIT(ret);
	return(ret);
err1:
DBG_EXIT(-1);
	return( -1 );

}


/*****************************************************************************
 *
 *  �֐���	_r_usr_alloc()
 *
 *  �@�\	�I�[�v�����[�U�\���̂̐����Ə�����
 * 
 *  �֐��l
 *     		����	�\���̂̃A�h���X
 *		�ُ�	NULL
 *
 ******************************************************************************/
r_usr_t	*
_r_usr_alloc()
{
	r_usr_t	*up;
DBG_ENT(M_RDB,"r_usr_alloc");

	if( (up = (r_usr_t *)neo_zalloc( sizeof(r_usr_t) )) ) {

		up->u_Id	= up;	// for Cell 
		_dl_init( &up->u_tbl );	
		_dl_init( &up->u_port );	
		_dl_init( &up->u_twait );	
		_dl_init( &up->u_rwait );	

		up->u_ident	= R_USR_IDENT;

		up->u_search.s_max	= R_EXPR_MAX;
	}
DBG_A(("up=0x%x\n", up ));
DBG_EXIT(up);
	return( up );
}


/*****************************************************************************
 *
 *  �֐���	_r_usr_free()
 *
 *  �@�\	�I�[�v�����[�U�\���̂̉��
 * 
 *  �֐��l
 *     		����
 *
 ******************************************************************************/
void
_r_usr_free( up )
	r_usr_t	*up;		/* �������I�[�v�����[�U�\���̂̃A�h���X*/
{
DBG_ENT(M_RDB,"r_usr_free");
DBG_A(("up=0x%x\n", up ));
	up->u_ident	= 0;
	neo_free( up );
DBG_EXIT(0);
}

/*****************************************************************************
 *
 *  �֐���	_r_tbl_alloc()
 *
 *  �@�\	�e�[�u���L�q�q�̐����Ə�����
 * 
 *  �֐��l
 *     		����	�L�q�q�̃A�h���X
 *		�ُ�	NULL
 *
 ******************************************************************************/
r_tbl_t	*
_r_tbl_alloc()
{
	r_tbl_t	*td;

	if( (td = (r_tbl_t *)neo_zalloc(sizeof(r_tbl_t))) ) {
		td->t_ident	= R_TBL_IDENT;
		_dl_init( &td->t_usr );
		_dl_init( &td->t_wait );
	}
	return( td );
}


/*****************************************************************************
 *
 *  �֐���	_r_tbl_free()
 *
 *  �@�\	�e�[�u���L�q�q�̕ԋp
 * 
 *  �֐��l
 *     		����
 *
 ******************************************************************************/
void
_r_tbl_free( td )
	r_tbl_t	*td;		/* �ԋp����e�[�u���L�q�q*/
{
	td->t_ident	= 0;
	if( td->t_scm ) {
		neo_free( td->t_scm );
		td->t_scm	= 0;
	}
	if( td->t_depend ) {
		neo_free( td->t_depend );
		td->t_depend	= 0;
	}
	neo_free( td );
}

/*****************************************************************************
 *
 *  �֐���	_rdb_req_head()
 *
 *  �@�\	RDB�t�@�~���v���g�R���v���p�P�b�g�w�b�_���쐬
 * 
 *  �֐��l
 *     		����
 *
 ******************************************************************************/
void
_rdb_req_head( hp, cmd, len )
	p_head_t	*hp;
	int		cmd;
	int		len;
{
	static	int	idcnt = 0;

	hp->h_tag	= P_HEAD_TAG;
	hp->h_mode	= P_PKT_REQUEST;
	hp->h_id	= idcnt++;
	hp->h_err	= 0;
	hp->h_len	= len - sizeof(p_head_t);
	hp->h_family	= P_F_RDB;
	hp->h_vers	= R_VER0;
	hp->h_cmd	= cmd;
}
