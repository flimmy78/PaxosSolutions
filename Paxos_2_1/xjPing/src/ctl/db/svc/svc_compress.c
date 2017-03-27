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
 *	svc_compress.c
 *
 *	����	server   R_CMD_COMPRESSK	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"

/*****************************************************************************
 *
 *  �֐���	svc_compress()
 *
 *  �@�\	�v���p�P�b�g�Ŏw�肵���e�[�u�������k���āA������Ԃ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_compress( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q�A�h���X */
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	    */
{
	int	len;
	r_req_compress_t	req;
	r_res_compress_t	res;
	r_tbl_t			*td;
	r_usr_t			*ud;
	int			numo,
				numn;
	int			i;
	int			err;
	int			stat0;

DBG_ENT(M_RDB,"svc_compress");

	/*
	 * �v���p�P�b�g��ǂݏo��
	 */
	len	= sizeof( req );
	if( p_recv( pd, (p_head_t*)&req, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}

	/*
	 * �w�肵���e�[�u���ƃI�[�v�����[�U�̗L���`�F�b�N���s��
	 */
	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, req.c_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, req.c_name );
	HashClear( &td->t_HashEqual );

	if( (neo_errno = check_td_ud( td, ud )) ) {
		err_reply( pd, hp, neo_errno );
		goto err1;
	}

	/*
	 * �e�[�u�����A�N�Z�X���Ă��鑼���[�U�����݂���΁A
	 * ���k�ł��Ȃ�
	 */
	if( td->t_usr_cnt > 1 ) {
		err_reply( pd, hp, E_RDB_TBL_BUSY );
		neo_rpc_log_err( M_RDB, svc_myname, &( PDTOENT(pd)->p_clnt ), 
						RDBCOMERR09, neo_errsym() );
		goto err1;
	}

	/*
	 * �e�[�u���̈��k�́A�t�@�C���A�������̗����ɂ��čs��
	 * ���k�́A���R�[�h�Ǘ��\���̂𑖍����A���g�p�̈���l�߂�
	 * ( ���Lfor����   i: �������̌��݃��R�[�h�ԍ�
	 *		numn: ���g�p�̈�̐擪�ԍ�
	 */
	numo	= td->t_rec_num;
	stat0 	= td->t_rec[0].r_stat;
	for( i = 0, numn = 0; i < numo; i++ ) {

		/*
		 * ���L���b�V��������Ă��郌�R�[�h���t�@�C��
		 * ���烍�[�h����
		 */
		if( !(td->t_rec[i].r_stat & R_REC_CACHE ) )
			svc_load( td, i, R_PAGE_SIZE/td->t_rec_size + 1 );

		/*
		 * ���R�[�h��O�̖��g�p�̈�Ɉړ����A�e�[�u�����l�߂�
		 */
		if( td->t_rec[i].r_stat & R_USED )   {
		    if( i != numn ) {	
			td->t_rec[numn].r_stat	= td->t_rec[i].r_stat;
			td->t_rec[numn].r_access= td->t_rec[i].r_access;
			td->t_rec[numn].r_wait	= td->t_rec[i].r_wait;

			bcopy( td->t_rec[i].r_head, td->t_rec[numn].r_head,
					td->t_rec_size );

			td->t_rec[numn].r_stat		|= R_UPDATE;
			td->t_rec[numn].r_head->r_stat	|= R_UPDATE;

			td->t_rec[i].r_stat	&=  ~(R_USED|R_BUSY|R_DIRTY);

			td->t_rec[numn].r_head->r_rel =
				td->t_rec[numn].r_head->r_abs = numn;
		    }
		    numn++;
		}
	}

	/*
	 *  1993.10.28 by Yang
	 */
	td->t_rec[0].r_stat |= (stat0 &R_REC_FREE );

	/*
	 * ���k�̌��ʂ��t�@�C���ɔ��f����
	 */
	if( numn < numo ) {

		if( (err = svc_switch( td, numn, numn )) ) {
			err_reply( pd, hp, err );
			goto err1;
		}
	}

	/*
	 * �����p�P�b�g���쐬���A�N���C�A���g���ɕԂ�
	 */
	reply_head( (p_head_t*)&req, (p_head_t*)&res, sizeof(res), 0 );

	res.c_rec_num	= numn;

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
