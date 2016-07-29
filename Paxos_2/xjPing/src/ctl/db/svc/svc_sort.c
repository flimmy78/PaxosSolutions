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
 *	svc_sort.c
 *
 *	����	R_CMD_SORT   procedure
 *				
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"

/*****************************************************************************
 *
 *  �֐���	svc_sort()
 *
 *  �@�\	�N���C�A���g���w�肵���e�[�u�����\�[�g���A������Ԃ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_sort( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q�A�h���X */
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	    */
{
	int	len;
	r_req_sort_t	*reqp;
	r_res_sort_t	res;
	r_tbl_t		*td;
	r_usr_t		*ud;
	int		i, j;
	r_sort_index_t	*abs, *rel;
	int		num;

DBG_ENT(M_RDB,"svc_sort");

	/*
	 * �v���p�P�b�g��M�p�o�b�t�@�G���A�𐶐�����
	 */
	len = hp->h_len + sizeof(p_head_t);
	if( !(reqp = (r_req_sort_t*)neo_malloc(len) ) ) {

		len = sizeof(p_head_t);
		if( p_recv( pd, hp, &len ) )
			goto err1;
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

	/*
	 * �w�肵���e�[�u���ƃI�[�v�����[�U�̗L���`�F�b�N
	 */
	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, reqp->s_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, reqp->s_name );
	HashClear( &td->t_HashEqual );

	if( (neo_errno = check_td_ud( td, ud ))
			|| (neo_errno = check_lock( td, ud, R_WRITE ) ) ) {
		err_reply( pd, hp, neo_errno );
		neo_rpc_log_err( M_RDB, svc_myname, &( PDTOENT(pd)->p_clnt ), 
						RDBCOMERR09, neo_errsym() );
		goto err2;
	}

	/*
	 * �w�肵���e�[�u�����A�N�Z�X���Ă��鑼���[�U��
	 * ���݂���΁A�\�[�g�ł��Ȃ�
	 */
	if( td->t_usr_cnt > 1 ) {
		err_reply( pd, hp, E_RDB_TBL_BUSY );
		neo_rpc_log_err( M_RDB, svc_myname, &( PDTOENT(pd)->p_clnt ), 
						RDBCOMERR09, neo_errsym() );
		goto err2;
	}

	/*
	 * �\�[�g�p���R�[�h���� Index�e�[�u���G���A��𐶐�����
	 */
	if( !(abs = (r_sort_index_t*)
		neo_malloc( td->t_rec_num*sizeof(r_sort_index_t))) ) {
		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err2;
	}

	if( !(rel = (r_sort_index_t*)
		neo_malloc( td->t_rec_num*sizeof(r_sort_index_t))) ) {
		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err3;
	}

	/*
	 * ���R�[�h������L Index �e�[�u���G���A�ɐݒ肷��
	 */
	for( i = 0, num = 0; i < td->t_rec_num; i++ ) {

		/*
		 * �L���b�V��������Ă��Ȃ����R�[�h���t�@�C��
		 * ���烍�[�h����
		 */
		if( !(td->t_rec[i].r_stat & R_REC_CACHE ) )
			svc_load( td, i, R_PAGE_SIZE/td->t_rec_size + 1 );

		if( td->t_rec[i].r_stat & R_USED ) {
			abs[num].i_head 
				= rel[num].i_head = td->t_rec[i].r_head;
			rel[num].i_id	= num;
			num++;
		}
	}

	/*
	 * �e�[�u�����\�[�g����
	 * �\�[�g�́A�܂��\�[�g���͊���쐬���āAIndex �e�[�u���ɑ΂���
	 * �s���B�\�[�g�� �\�[�g�ς�Index �e�[�u���ɂ���āA����
	 * ��������̃��R�[�h�����ւ��āA����(�~��)�ɕ��ׂ�
	 */
	if( num ) {
		if( (neo_errno = _rdb_sort_open( td,  reqp->s_n, (r_sort_t*)(reqp+1) ) )
			|| (neo_errno = _rdb_sort_start( rel, num ))
			|| (neo_errno = _rdb_sort_close() )
			|| (neo_errno = 
				_rdb_arrange_data(abs,rel,num,td->t_rec_size))){

			err_reply( pd, hp, neo_errno );

			goto err4;
		}
		/*
		 * ���R�[�h�̏�Ԃ��X�V����
		 */
		for( i = 0; i < num; i++ ) {

DBG_A(("<%i->%i,%d[%s]>\n", i, abs[i].i_id, abs[i].i_head->r_abs, 
		abs[i].i_head+1 ));

			if( abs[i].i_id == i )
				continue;

			j = abs[i].i_head->r_abs;
			td->t_rec[j].r_stat 	|= R_UPDATE;
			abs[i].i_head->r_stat	|= R_UPDATE;

			td->t_rec[j].r_stat	|= R_BUSY;
			td->t_rec[i].r_access	= ud;
		}
	}
	td->t_version++;

	/*
	 * �����p�P�b�g���쐬���A�N���C�A���g���֑���
	 */
	reply_head( (p_head_t*)reqp, (p_head_t*)&res, sizeof(res), 0 );

	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err4;
	}

	/*
	 * �v���p�P�b�g��M�p�G���A�A�\�[�g�p�Վ�
	 * Index �G���A���������
	 */
	neo_free( rel );
	neo_free( abs );
	neo_free( reqp );

DBG_EXIT(0);
	return( 0 );

err4: DBG_T("err4");
	neo_free( rel );
err3: DBG_T("err3");
	neo_free( abs );
err2: DBG_T("err2");
	neo_free( reqp );
err1: 
DBG_EXIT(neo_errno);
	return( neo_errno );
}
