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
 *	svc_create.c
 *
 *	����		R_CMD_CREATE
 *				R_CMD_DROP	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"

/*****************************************************************************
 *
 *  �֐���	svc_create()
 *
 *  �@�\	�e�[�u�����쐬����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_create( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	r_req_create_t	*reqp;
	r_res_create_t	*resp;
	int		len;
	r_tbl_t		*td;
	r_usr_t		*ud;
	int		i;

DBG_ENT(M_RDB,"svc_create");

	/*
	 * �v���p�P�b�g��M�o�b�t�@���A���b�N
	 * 	[ R_CMD_CREATE�v���p�P�b�g�T�C�Y���ϒ�������]   
	 */
	len	= hp->h_len + sizeof(p_head_t);
	if( !(reqp = (r_req_create_t *)neo_malloc( len ) ) ) {
		len	= sizeof(*hp);
		if( p_recv( pd, hp, &len ) ) {	/* discard */
			if( neo_errno != E_LINK_BUFSIZE )
				goto err1;
		}
		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err1;
	}
	
	/*
	 * �v���p�P�b�g��ǂݏo��
	 */
	if( p_recv( pd, (p_head_t*)reqp, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err2;
	}

	if ( _svc_inf.f_stat == R_SERVER_STOP ) {
		err_reply( pd, hp, E_RDB_STOP );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR10, neo_errsym() );
		goto err2;
	}

	/*
	 * ���v���C�\���̃������A���b�N
	 * 	( R_CMD_CREATE�����p�P�b�g�T�C�Y���ϒ�������)
	 */
	len	= sizeof(r_res_create_t) + reqp->c_item_num*sizeof(r_item_t);
	if( !(resp = (r_res_create_t*)neo_malloc( len ) ) ) {
		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err2;
	}

	/*
	 * �e�[�u�����`�F�b�N
	 */
	if( HashGet( &_svc_inf.f_HashOpen, reqp->c_name ) ) {
			err_reply( pd, hp, E_RDB_EXIST );
			neo_rpc_log_err( M_RDB, svc_myname, 
					&( PDTOENT(pd)->p_clnt ), 
						CREATEERR01, neo_errsym() );
			goto err3;
	}

	/*
	 * �Â��e�[�u�����������������
	 */
	if ( (neo_errno = drop_table()) ) {
		err_reply( pd, hp, neo_errno ) ;
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR05, neo_errsym() );
		goto	err3;
	}

	/*
	 * �I�[�v�����[�U/�e�[�u���L�q�q�������A���b�N
	 */
	if( !(ud = _r_usr_alloc() ) ) {
		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err3;
	}
	if( !(td = svc_tbl_alloc() ) ) {
		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err4;
	}

	/*
	 * �e�[�u��(�t�@�C��)�N���G�C�g
	 *  ���Ƀt�@�C�������݂����ꍇ�́A�V�K�ɍ�蒼�����
	 */
	if( TBL_CREATE( reqp->c_name, R_TBL_NORMAL, (long)time(0), 
			reqp->c_rec_num, reqp->c_item_num, reqp->c_item,
			0, 0 ) ) {
		err_reply( pd, hp, neo_errno );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR06, neo_errsym() );
		goto err5;
	}

	/*
	 * �e�[�u��(�t�@�C��)�I�[�v��
	 */
	if( TBL_OPEN( td, reqp->c_name ) ) {
		err_reply( pd, hp, neo_errno );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR07, neo_errsym() );
		goto err5;
	}

	/*
	 * ���R�[�h�Ǘ��\����/���R�[�h�o�b�t�@�������A���b�N
	 */
	if( (neo_errno = rec_cntl_alloc( td )) ) {
		err_reply( pd, hp, neo_errno );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err6;
	}
	if( (neo_errno = rec_buf_alloc( td )) ) {
		err_reply( pd, hp, neo_errno );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err7;
	}

	/*
	 *	r_rec_t�t���O�ݒ�
	 */
DBG_A(("rec_num=%d\n", td->t_rec_num ));
	for( i = 0; i < td->t_rec_num; i++ ) {
		td->t_rec[i].r_stat |= R_REC_CACHE;
	}

	/*
	 * �A�N�Z�X�^�C���ݒ�
	 */
	td->t_access	= time( 0 );

	/*
	 * �I�[�v�����[�U�\���̏��ݒ�
	 */
	r_port_t	*pe = PDTOENT(pd);

	_dl_insert( &ud->u_port, &(PDTOENT(pd)->p_usr_ent) );
	ud->u_pd	= pd;
	ud->u_mytd	= td;
	ud->u_td	= reqp->c_td;	// client td

	_dl_insert( ud, &td->t_usr );
	td->t_usr_cnt = 1;

	HashPut( &pe->p_HashUser, td->t_name, ud );
#ifdef	ZZZ
#else
	ud->u_refer = 1;
#endif

	_dl_insert( td, &(_svc_inf.f_opn_tbl ) );

	HashPut( &_svc_inf.f_HashOpen, td->t_name, td );


	/*
	 * ���v���C���ݒ�
	 */
	reply_head( (p_head_t*)reqp, (p_head_t*)resp, len, 0 );

	resp->c_td		= td;
	resp->c_ud		= ud;
	resp->c_rec_num		= td->t_rec_num;
	resp->c_rec_size	= td->t_rec_size;
	resp->c_item_num	= td->t_scm->s_n;
	bcopy( td->t_scm->s_item, resp+1, td->t_scm->s_n*sizeof(r_item_t) );

	neo_rpc_log_msg( "RDB_CREATE", M_RDB, svc_myname,
			    &( PDTOENT(pd)->p_clnt), CREATELOG01, td->t_name );
	/*
	 * ���v���C
	 */
	if( p_reply_free( pd, (p_head_t*)resp, len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err3;
	}

	/*
	 * �ʐM�o�b�t�@�̉��
	 */
	neo_free( reqp );

DBG_EXIT(0);
	return( 0 );

err7: DBG_T("err7");
	rec_cntl_free( td );
err6: DBG_T("err6");
	TBL_CLOSE( td );
	TBL_DROP( reqp->c_name );
err5: DBG_T("err5");
	_r_tbl_free( td );
err4: DBG_T("err4");
	_r_usr_free( ud );
err3: DBG_T("err3");
	neo_free( resp );
err2: DBG_T("err2");
	neo_free( reqp );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	svc_drop()
 *
 *  �@�\	�e�[�u�����폜����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_drop( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	r_tbl_t		*td;
	r_req_drop_t	req;
	r_res_drop_t	res;
	int		len;
DBG_ENT(M_RDB,"svc_drop");

	/*
	 * �v���p�P�b�g��ǂݏo��
	 */
	len	= sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}

	if ( _svc_inf.f_stat == R_SERVER_STOP ) {
		err_reply( pd, hp, E_RDB_STOP );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR10, neo_errsym() );
		goto err1;
	}

	/*
	 * �e�[�u���`�F�b�N
	 */
	if( (td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, req.d_name)) ) {

			if( td->t_usr_cnt == 0 ) {

				if ( (neo_errno = drop_td( td )) ) {
					err_reply( pd, hp, neo_errno );
					goto	err1;
				}
			} else {
				err_reply( pd, hp, E_RDB_TBL_BUSY );
				neo_rpc_log_err( M_RDB, svc_myname, 
						&( PDTOENT(pd)->p_clnt ),
						CREATEERR02, neo_errsym() );
				goto err1;
			}
	}

	/*
	 * �e�[�u��(�t�@�C��)�폜
	 */
	if( TBL_DROP( req.d_name ) ) {
		err_reply( pd, hp, neo_errno );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR08, neo_errsym() );
		goto err1;
	}

	neo_rpc_log_msg( "RDB_DROP", M_RDB, svc_myname,
			    &( PDTOENT(pd)->p_clnt), CREATELOG02, req.d_name );

	/*
	 * ���v���C
	 */
	reply_head( (p_head_t*)&req, (p_head_t*)&res, sizeof(res), 0 );

	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err1;
	}
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}

