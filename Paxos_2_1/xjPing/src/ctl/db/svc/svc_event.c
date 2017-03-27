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
 *	svc_event.c
 *
 *	����		R_CMD_EVENT
 *				R_CMD_EVENT_CANCEL	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"

/*****************************************************************************
 *
 *  �֐���	svc_event()
 *
 *  �@�\	�C�x���g��������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_event( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	int	len;
	r_req_event_t	req;
	r_tbl_t		*td;
	r_usr_t		*ud;
	int		err;

DBG_ENT(M_RDB,"svc_event");

	/*
	 * �v���p�P�b�g��ǂݏo��
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}

	/*
	 * �e�[�u���A�I�[�v�����[�U�L�q�q�L���`�F�b�N
	 * �y�уe�[�u������\���`�F�b�N
	 */
	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, req.e_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, req.e_name );

	if( (neo_errno = check_td_ud( td, ud ))
			||(neo_errno = check_lock( td, ud, R_WRITE ) ) ) {

		err_reply( pd, hp, neo_errno );
		neo_rpc_log_err( M_RDB, svc_myname, &( PDTOENT(pd)->p_clnt ), 
						RDBCOMERR09, neo_errsym() );
		goto err1;
	}

	/*
	 * �C�x���g�����y�щ����ԓ�
	 */
	bcopy( &req, &ud->u_head, sizeof(ud->u_head) );

	switch( req.e_class ) {

		case R_EVENT_REC:
			if( (err = svc_rec_wait( td, ud, req.e_code )) ) {
				err_reply( pd, hp, err );
				goto err1;
			}
			break;

		default:
			err_reply( pd, hp, E_RDB_INVAL );
			neo_rpc_log_err( M_RDB, svc_myname, 
						&( PDTOENT(pd)->p_clnt ), 
						EVENTERR01, neo_errsym() );
			goto err1;
	}

DBG_EXIT(0);
	return( 0 );
err1: 
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	svc_rec_wait()
 *
 *  �@�\	
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_rec_wait( td, ud, index )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
	r_usr_t	*ud;		/* �I�[�v�����[�U�L�q�q	*/
	int	index;		/* ���R�[�h��Δԍ�	*/
{
	r_res_event_t	res;
	int		err;

DBG_ENT(M_RDB,"svc_rec_wait");

	/*
	 * ���R�[�h��Δԍ��`�F�b�N
	 */
	if( index == -1 )
		index	= ud->u_cursor;

	if( index < 0 || td->t_rec_num <= index ) {
		err = E_RDB_INVAL;
		goto err1;
	}

	/*
	 * ���R�[�h���񃍃b�N����������A�C�x���g���������Ƃ��āA
	 * ������Ԃ��B�����łȂ��ꍇ�́A�Y���I�[�v�����[�U��
	 * ���R�[�h�̑҂����[�U���X�g�ɓo�^����
	 */
	if( !(td->t_rec[index].r_stat & R_LOCK ) ) {	/* ok */
DBG_A(("rec_nowait:index=%d\n", index ));
		
		reply_head( &ud->u_head, (p_head_t*)&res, sizeof(res), 0 );

		if( p_reply( UDTOPD(ud), (p_head_t*)&res, sizeof(res) ) ) {

			err = neo_errno;

			goto err1;
		}
	} else {	/* wait */
DBG_A(("rec_wait:index=%d\n", index ));
		
		if( td->t_rec[index].r_wait )
			_dl_insert( &ud->u_rwait, 
					&((td->t_rec[index].r_wait)->u_rwait) );
		else
			td->t_rec[index].r_wait	= ud;

		ud->u_stat	|= R_USR_RWAIT;
		ud->u_index	= index;
	}
DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(err);
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	svc_rec_post()
 *
 *  �@�\	���R�[�h���b�N������S�Ă̑҂����[�U��
 *		�ʒm����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_rec_post( td, index )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
	int	index;		/* ���R�[�h��Δԍ�	*/
{
	r_res_event_t	res;
	_dlnk_t		*dp;
	r_usr_t		*ud;
	int		err;
DBG_ENT(M_RDB,"svc_rec_post");

	/*
	 * �Y�����R�[�h�̑S�Ă̑҂����[�U�ɑ΂��āA
	 * ���b�N������ʒm����
	 */
	while( (ud = td->t_rec[index].r_wait) ) {

		dp = _dl_head( &ud->u_rwait );

		_dl_remove( &ud->u_rwait );
		_dl_init( &ud->u_rwait );

		ud->u_stat	&= ~R_USR_RWAIT;

		td->t_rec[index].r_wait	= RWTOUSR( dp );

		reply_head( (p_head_t*)&ud->u_head, (p_head_t*)&res, sizeof(res), 0 );

		if( p_reply( UDTOPD(ud), (p_head_t*)&res, sizeof(res) ) ) {

			err = neo_errno;

			goto err1;
		}
	}
DBG_EXIT(0);
	return( 0 );

err1:
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	svc_event_cancel()
 *
 *  �@�\	�C�x���g�����v�����L�����Z������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_event_cancel( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �L�����Z���v���w�b�_	*/
{
	r_req_event_cancel_t	req;
	r_res_event_cancel_t	res;
	int			len;
	r_tbl_t			*td;
	r_usr_t			*ud;
	int			i;

DBG_ENT(M_RDB,"svc_event_cancel");

	/*
	 * �L�����Z���v����ǂݏo��
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) )
		goto err1;

	/*
	 * �e�[�u���A�I�[�v�����[�U�L�q�q�L���`�F�b�N
	 */
	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, req.e_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, req.e_name );
	i  = req.e_code;

	if( (neo_errno = check_td_ud( td, ud )) ) {

		err_reply( pd, hp, neo_errno );

		goto err1;
	}
DBG_A(("code=%d,rec_num=%d\n", i, td->t_rec_num ));

	/*
	 * ���R�[�h��Δԍ��`�F�b�N
	 */
	if( i == -1 )
		i	= ud->u_cursor;

	if( i < 0 || td->t_rec_num <= i ) {

		err_reply( pd, hp, E_RDB_INVAL );

		goto err1;
	}
	/*
	 * ���R�[�h�����̃L�����Z������
	 */

	svc_cancel_rwait( td, ud, i );

	/*
	 * ������Ԃ�
	 */
	reply_head( (p_head_t*)&req, (p_head_t*)&res, sizeof(res), 0 );

	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) )
		goto err1;
	
DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	svc_cancel_rwait()
 *
 *  �@�\	���R�[�h�̃C�x���g�������L�����Z������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_cancel_rwait( td, ud, i )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
	r_usr_t	*ud;		/* �I�[�v�����[�U�L�q�q	*/
	int	i;		/* ���R�[�h�̐�Δԍ�	*/
{
DBG_ENT(M_RDB,"svc_cancel_rwait");
DBG_A(("td=0x%x,ud=0x%x,i=%d\n", td, ud, i ));

	/*
	 * ���R�[�h�̑҂����[�U���X�g����
	 * �Y�����[�U���O��
	 */
	if( ud->u_stat & R_USR_RWAIT ) {

		if( td->t_rec[i].r_wait == ud ) {

			td->t_rec[i].r_wait = RWTOUSR( _dl_head(&ud->u_rwait) );
		}
		if( _dl_head( &ud->u_rwait) ) {

			_dl_remove( &ud->u_rwait );
			_dl_init( &ud->u_rwait );

		}
		ud->u_stat	&= ~R_USR_RWAIT;
	}
DBG_EXIT( 0 );
	return( 0 );
}
