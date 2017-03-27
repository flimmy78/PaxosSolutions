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
 *	svc_flush.c
 *
 *	����		R_CMD_FLUSH
 *				R_CMD_CANCEL	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"

/*****************************************************************************
 *
 *  �֐���	svc_flush()
 *
 *  �@�\	�X�V�f�[�^���f�B�X�N�ɏ����o��
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_flush( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	r_req_flush_t	req;
	r_res_flush_t	res;
	int		len;
	r_tbl_t		*td;
	r_usr_t		*ud;

DBG_ENT(M_RDB,"svc_flush");

	/*
	 * ���N�G�X�g���V�[�u
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}

	/*
	 * �e�[�u���̗L����/�e�[�u������\���`�F�b�N
	 *  �������A������̃e�[�u���r���̓`�F�b�N���Ȃ�
	 */
	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, req.f_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, req.f_name );

	if( (neo_errno = check_td_ud( td, ud )) ) {
		err_reply( pd, hp, neo_errno );
		goto err1;
	}

	/*
	 * �X�V���ꂽ�L���b�V���̓��e���f�B�X�N(�t�@�C��)�֏����o��
	 */
	if( (neo_errno = svc_store( td, 0, td->t_rec_num, ud )) ) {
		err_reply( pd, hp, neo_errno );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR04, neo_errsym() );
		goto err1;
	}

	td->t_modified	= time(0);
	td->t_version++;
	/*
	 * ���X�|���X���v���C
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


/*****************************************************************************
 *
 *  �֐���	svc_cancel()
 *
 *  �@�\	�X�V�f�[�^�����̃f�[�^�ɖ߂�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_cancel( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	r_req_cancel_t	req;
	r_res_cancel_t	res;
	int		len;
	r_tbl_t		*td;
	r_usr_t		*ud;
	int		i;
	int		inc;

DBG_ENT(M_RDB,"svc_cancel");

	/*
	 * ���N�G�X�g���V�[�u
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}

	/*
	 * �e�[�u���̗L����/�e�[�u������\���`�F�b�N
	 *  �������A������̃e�[�u���r�����`�F�b�N����
	 *  �Ȃ��Ȃ�΁A�r���������Ă��郆�[�U�ɖ��f�Ńo�b�t�@�̓��e��ύX����
	 *  ���Ƃ́A�ł��Ȃ�����B
	 */
//	td = req.c_td;
//	ud = req.c_ud;

	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, req.c_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, req.c_name );

	if( (neo_errno = check_td_ud( td, ud ))
			|| (neo_errno = check_lock( td, ud, R_WRITE ) ) ) {
		err_reply( pd, hp, neo_errno );
		neo_rpc_log_err( M_RDB, svc_myname, &( PDTOENT(pd)->p_clnt ), 
						RDBCOMERR09, neo_errsym() );
		goto err1;
	}

	/*
	 * �X�V�f�[�^�����̃f�[�^�ɖ߂�
	 */
	for ( i = 0; i < td->t_rec_num; i++ ) {

		if( td->t_rec[i].r_access != ud
				|| !(td->t_rec[i].r_stat & R_BUSY) )
			continue;
		

		if( td->t_rec[i].r_stat & R_USED )	
			inc = 1;
		else					
			inc = 0;

		if( (neo_errno = svc_load( td, i, 1 )) ) {

			err_reply( pd, hp, neo_errno );

			goto err1;
		}

		if( td->t_rec[i].r_stat & R_USED )	
			inc = (inc ? 0 : 1);
		else					
			inc = (inc ? -1 : 0);

		td->t_rec_used	+= inc;
		td->t_stat	|= R_UPDATE;

		/*
		 * �}�����R�[�h���L�����Z�������ꍇ�ɂ�,
		 * ���g�p�̈�INDEX���X�V����
		 */
		if ( inc == -1 && i < td->t_unused_index )
			td->t_unused_index = i ;
	}
	 
	ud->u_added = 0;

	/*
	 * ���X�|���X���v���C
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

