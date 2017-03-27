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
 *	svc_open.c
 *
 *	����		R_CMD_OPEN
 *				R_CMD_CLOSE	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"
#include	"join.h"

/*****************************************************************************
 *
 *  �֐���	svc_open()
 *
 *  �@�\	�N���C�A���g����I�[�v���v����ǂ݂����āA�w�肳�ꂽ
 *		�e�[�u�����I�[�v�����āA������Ԃ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_open( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q�A�h���X   */
	p_head_t	*hp;		/* �I�[�v���v���p�P�b�g�w�b�_ */
{
	r_req_open_t	*reqp;
	Msg_t			*pMsg;
	r_res_open_t	*resp;
	int		len;
	r_usr_t		*ud;
	r_tbl_t		*td;

DBG_ENT(M_RDB,"svc_open");

	/*
	 * �I�[�v���v���p�P�b�g��ǂݏo��
	 */
	if( !(pMsg = p_recvByMsg( pd ) ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}
	reqp	= (r_req_open_t*)MsgFirst( pMsg );
LOG(neo_log, LogDBG, "o_name[%s]", reqp->o_name );

	/*
	 * rdb_close_client �̂���reopen����������B
	 */
	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, reqp->o_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, reqp->o_name );

	if( td && ud ) {
#ifdef	ZZZ
#else
		ud->u_refer++;
#endif
		len = sizeof(r_res_open_t) + sizeof(r_item_t)*(td->t_scm->s_n - 1 );
		if( !(resp = (r_res_open_t*)neo_malloc( len ) ) ) {
			neo_errno = E_RDB_NOMEM;
			neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
			goto err2;
		}

		reply_head( (p_head_t*)reqp, (p_head_t*)resp, len, 0 );
		resp->o_td		= td;
		resp->o_ud		= ud;
		strncpy( resp->o_name, td->t_name, sizeof(resp->o_name) );
		resp->o_rec_num		= td->t_rec_num;
		resp->o_rec_used	= td->t_rec_used;
		resp->o_rec_size	= td->t_rec_size;
		bcopy( &td->t_scm->s_n, &resp->o_n, 
			sizeof(td->t_scm->s_n) + sizeof(r_item_t)*td->t_scm->s_n );

		goto reopen;
	}
	if( (neo_errno = prc_open( reqp, &resp, &len, 1 )) ) {

		err_reply( pd, hp, neo_errno );

		goto err2;
	}

	/*
	 * �Y���I�[�v�����[�U���|�[�g�G���g���ɓo�^����
	 */
	td = resp->o_td;
	ud = resp->o_ud;
	_dl_insert( &ud->u_port, &(PDTOENT(pd)->p_usr_ent) );

	HashPut( &pe->p_HashUser, td->t_name, ud );
#ifdef	ZZZ
#else
	ud->u_refer++;
#endif

	/*
	 * �Y���I�[�v�����[�U�̏���ݒ肷��
	 */
	ud->u_pd	= pd;

reopen:
	if( p_reply_free( pd, (p_head_t*)resp, len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err2;
	}
	
	neo_rpc_log_msg( "RDB_OPEN", M_RDB, svc_myname, 
		   &( PDTOENT(pd)->p_clnt), OPENLOG01, ud->u_mytd->t_name );

	MsgDestroy( pMsg );
DBG_EXIT(0);
	return( 0 );
//err3:	neo_free( resp );
err2:	MsgDestroy( pMsg );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	svc_close()
 *
 *  �@�\	�N���C�A���g�����v�����ꂽ�e�[�u�����N���[�Y���A
 *		������Ԃ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_close( pd, hp )
	p_id_t	*pd;			/* �ʐM�|�[�g�L�q�q�A�h���X	*/
	p_head_t	*hp;		/* �N���[�Y�v���p�P�b�g�w�b�_	*/
{
	r_req_close_t	*reqp;
	r_res_close_t	*resp;
	Msg_t			*pMsg;
#ifdef	ZZZ
#else
	r_usr_t		*ud;
	r_tbl_t		*td;
#endif

DBG_ENT(M_RDB,"svc_close");

	/*
	 * �N���[�Y�v���p�P�b�g��ǂ݂���
	 */
	if( !(pMsg = p_recvByMsg( pd )) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}
	reqp	= (r_req_close_t*)MsgFirst( pMsg );
LOG(neo_log, LogDBG, "o_name[%s]", reqp->c_name );

	r_port_t	*pe = PDTOENT(pd);

	reqp->c_td = td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, reqp->c_name);
	reqp->c_ud = ud = (r_usr_t*)HashGet( &pe->p_HashUser, reqp->c_name );
#ifdef	ZZZ
#else
	if( !ud ) {
		neo_errno = E_RDB_UD;
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err2;
	}
#endif

	resp	= (r_res_close_t*)neo_malloc( sizeof(*resp) );
	if( !resp ) {
		neo_errno = E_RDB_NOMEM;
		goto err2;
	}
#ifdef	ZZZ
	if( (neo_errno = prc_close( reqp, resp, 1 )) ) {

		err_reply( pd, hp, neo_errno );

		goto err3;
	}
#else
	if( !(td->t_stat & T_TMP) )	ud->u_refer--;
	ASSERT( ud->u_refer >= 0 );
	if( ud->u_refer == 0 ) {
		if( (neo_errno = prc_close( reqp, resp, 1 )) ) {

			err_reply( pd, hp, neo_errno );

			goto err3;
		}
	} else {
		memcpy( resp, reqp, sizeof(p_head_t) );
		resp->c_head.h_len	= sizeof(*resp) - sizeof(p_head_t);
	}
#endif
	if( p_reply_free( pd, (p_head_t*)resp, sizeof(*resp) ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err2;
	}

	neo_rpc_log_msg( "RDB_CLOSE", M_RDB, svc_myname, 
		 	    &( PDTOENT(pd)->p_clnt ), OPENLOG02, reqp->c_td->t_name );
	MsgDestroy( pMsg );
DBG_EXIT(0);
	return( 0 );

err3:	neo_free( resp );
err2:	MsgDestroy( pMsg );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}

#ifdef	ZZZ
#else
int
svc_close_client( pd, hp )
	p_id_t	*pd;			/* �ʐM�|�[�g�L�q�q�A�h���X	*/
	p_head_t	*hp;		/* �N���[�Y�v���p�P�b�g�w�b�_	*/
{
	r_req_close_client_t	*reqp;
	r_res_close_client_t	res;
	Msg_t			*pMsg;
	r_usr_t		*ud;

DBG_ENT(M_RDB,"svc_close_client");

	/*
	 * �N���[�Y�v���p�P�b�g��ǂ݂���
	 */
	if( !(pMsg = p_recvByMsg( pd )) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}
	reqp	= (r_req_close_client_t*)MsgFirst( pMsg );
LOG(neo_log, LogDBG, "c_name[%s]", reqp->c_name );

	r_port_t	*pe = PDTOENT(pd);

	ud = (r_usr_t*)HashGet( &pe->p_HashUser, reqp->c_name );

	if( !ud ) {
		neo_errno = E_RDB_UD;
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err2;
	}
	if( ud->u_refer <= 0 ) {
		neo_errno = E_RDB_CLOSED;
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err2;
	}
	--ud->u_refer;
	ASSERT( ud->u_refer >= 0 );
	memcpy( &res, reqp, sizeof(p_head_t) );
	res.c_head.h_len	= sizeof(res) - sizeof(p_head_t);
	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err2;
	}

	neo_rpc_log_msg( "RDB_CLOSE_CLIENT", M_RDB, svc_myname, 
		 	    &( PDTOENT(pd)->p_clnt ), OPENLOG02, reqp->c_name );
	MsgDestroy( pMsg );
DBG_EXIT(0);
	return( 0 );

err2:	MsgDestroy( pMsg );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}
#endif

/*****************************************************************************
 *
 *  �֐���	close_user()
 *
 *  �@�\	�I�[�v�����[�U���N���[�Y����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
close_user( ud )
	r_usr_t	*ud;	/* �N���[�Y���悤�Ƃ���I�[�v�����[�U�|�C���^ */
{
	r_tbl_t		*td;

DBG_ENT(M_RDB,"close_user");

	/*
	 * �e�[�u����Ԃ̍X�V�r�b�g�������Ă����
	 * �w�b�_�����X�V����
	 */
	td = ud->u_mytd;

	if( td->t_stat & R_UPDATE ) {
		TBL_CNTL( td, 0, 0 );	/* header update */
		td->t_stat	&= ~R_UPDATE;
	}

	/*
	 * UNKOWN
	 */
	if( ud->u_stat & R_USR_RWAIT )
		svc_cancel_rwait( td, ud, ud->u_index );

	/*
	 * �|�[�g�G���g���̃��[�U���X�g����Y�����[�U���O��
	 */
	_dl_remove( &ud->u_port );

	/*
	 * �e�[�u���̃I�[�v�����[�U���X�g����Y�����[�U���O��
	 */
	_dl_remove( &ud->u_tbl );

DBG_A(("cnt=%d\n", td->t_usr_cnt ));
	td->t_usr_cnt--;

	if( ud->u_pd ) {
		r_port_t	*pe = PDTOENT(ud->u_pd);

		HashRemove( &pe->p_HashUser, td->t_name );
	}

	/*
	 *	????
	 */
	_dl_remove( ud );
	
	_r_usr_free( ud );

DBG_EXIT(0);
	return( 0 );
//err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}

