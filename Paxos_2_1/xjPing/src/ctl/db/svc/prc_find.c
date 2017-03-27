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


#include	"neo_db.h"
#include	"svc_logmsg.h"
#include	"y.tab.h"

/*****************************************************************************
 *
 *  �֐���	prc_find()
 *
 *  �@�\	���R�[�h�̌���
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
prc_find( reqp, respp, res_lenp )
	r_req_find_t	*reqp;
	r_res_find_t	**respp;
	int		*res_lenp;
{
	r_res_find_t	*resp;
	int	len;
	r_tbl_t		*td;
	r_usr_t		*ud;
	int		i;
	int		m, pn;
	r_value_t	v;
	int		cnt, n;
	char		*bufp;
	int		mode;
	r_expr_t	*check;
DBG_ENT(M_RDB,"prc_find");

#ifdef XXX
	/*
	 * ���N�G�X�g���V�[�u
	 */
	len = sizeof(req);
	if( p_recv( pd, &req, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}
#endif

	/*
	 * �e�[�u��/�I�[�v�����[�U�L�q�q�L�����`�F�b�N
	 */
	td	 = reqp->f_td;
	ud	 = reqp->f_ud;
	mode	= reqp->f_mode;

	if( (neo_errno = check_td_ud( td, ud )) || 
	    (neo_errno = 
                check_lock( td, ud, ((mode&R_LOCK) ? R_WRITE : R_READ) )) ) {
		goto err1;
	}

	/*
	 * ���v���C�p�̃o�b�t�@�̃������A���b�N
	 * �擾���ꂽ�������� R_PAGE_SIZE���ő�l�Ƃ���
	 */
	if( R_PAGE_SIZE >= td->t_rec_size ) {
		pn	= R_PAGE_SIZE/td->t_rec_size;
		m	= reqp->f_num < pn ? reqp->f_num : pn;
	} else {
		pn	= m = 1;
	}
	len = sizeof(r_res_find_t) + m*td->t_rec_size;
	if( !(resp = (r_res_find_t *)neo_malloc( len )) ) {
		
		neo_errno = E_RDB_NOMEM;
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err1;
	}

	check	= ud->u_search.s_expr;
	cnt = 0;	/* �g�[�^���擾���R�[�h���̏����� */

	/*
	 * ���� ( for )
	 *      ������:  �R�s�[�p�o�b�t�@�̃|�C���^(bufp)��
	 *		 ���X�|���X�p�o�b�t�@�̃��R�[�h�̈�̐擪�ɐݒ�
	 *		 �擾���R�[�h��(i)��0�ɏ�����
	 *	�I������:�J�[�\���ʒu(i)���e�[�u���̍Ō�ɗ���܂�
	 *	����:	 �J�[�\������i�߂�
	 *
	 *	*���v���C�̑��M�� R_PAGE_SIZE�����R�[�h�o�b�t�@�̍ő�l
	 *	 �Ƃ���A�]���āA1��ő��M�ł��Ȃ����́A�������đ�����
	 *	 ���̎��A(n)�́A�o�b�t�@�ɐݒ肳��Ă��郌�R�[�h�̐�
	 *	 (f_num)�ŁA�܂��c�肪����Ƃ��Af_more��1��ݒ肷��
	 *
	 */
	for( bufp = (char *)(resp+1), n = 0;
		(i = ud->u_cursor) < td->t_rec_num; ud->u_cursor++ ) {

		if( !(td->t_rec[i].r_stat & R_REC_CACHE) )	/* load */
			svc_load( td, i, pn );
		
		if( !(td->t_rec[i].r_stat & R_USED ) )
			continue;

		if( td->t_rec[i].r_head->r_Cksum64 ) {
			assert( !in_cksum64( td->t_rec[i].r_head, td->t_rec_size, 0 ) );
		}
		/*
		 * �����������ݒ肵�Ă������ꍇ�́A������]������
		 * 		( rdb_evaluate()�� clnt/rdb_expr.c )
		 */
		if( check ) {
			if( rdb_evaluate( check, (char*)td->t_rec[i].r_head, &v ) 
					|| v.v_type != VBOOL ) {

				neo_errno = E_RDB_SEARCH;

				goto err2;
			}
		}

		/*
		 * �����������������郌�R�[�h����������
		 */
		if( !check || v.v_value.v_int ) {	/* found */

			/*
			 * �r�����[�h���ݒ肳��Ă���΁A�r����������
			 */
			if ( mode & R_LOCK )  {

				/* 
				 * ���R�[�h�r���`�F�b�N
				 */
				if( (td->t_rec[i].r_stat & R_LOCK)
					&& td->t_rec[i].r_access != ud ) {

					goto	record_lock ;
				}  else {
					td->t_rec[i].r_stat &= ~(R_LOCK);
					td->t_rec[i].r_stat |= (mode & R_LOCK);
					td->t_rec[i].r_access = ud ;
				}

			} else {

				/*
				 * ���R�[�h�Q�Ɣr���`�F�b�N
				 */
				if( ( td->t_rec[i].r_stat & R_EXCLUSIVE )
					&& td->t_rec[i].r_access != ud ) {

					goto record_lock;
				} 
			}

			/*
			 * ���R�[�h�f�[�^�̃R�s�[&�擾���R�[�h���̃J�E���g
			 */
			bcopy( td->t_rec[i].r_head, bufp, td->t_rec_size );
			((r_head_t *)bufp)->r_stat	= td->t_rec[i].r_stat;
			bufp += td->t_rec_size;
			n++; cnt++;

			/*
			 * �g�[�^���擾���R�[�h�����A�v�����R�[�h����
			 * �B�����烊�v���C��Ԃ��A�ǉ�(f_more)�͖���
			 */
			if( cnt >= reqp->f_num ) {

				resp->f_more = 0;

				ud->u_cursor++;

				goto reply;
			}

			/*
			 * ���v���C1�񂠂���̍ő呗�M���R�[�h��(m)����
			 * �擾���R�[�h��(n)�����������͍Č���
			 */
			if( n < m )
				continue;
			
			/*
			 * �g�[�^���擾���R�[�h�����A�v�����R�[�h����
			 * �B���Ă��Ȃ��̂ŁA�ǉ�(f_more)���s��
			 */
			resp->f_more = 1;

			ud->u_cursor++;

			goto reply;
		} else
			continue;
	}

	/*
	 * �J�[�\�����e�[�u���̍Ō�ɗ���(�����Y�����郌�R�[�h����)
	 */
	resp->f_more	= 0;


	/*
	 * ���v���C
	 */
reply:
	len = sizeof(r_res_find_t) + n*td->t_rec_size;

	reply_head( (p_head_t*)reqp, (p_head_t*)resp, len, 0 );

	resp->f_num	= n;

	*respp		= resp;
	*res_lenp	= len;
DBG_EXIT(0);
	return( 0 );

record_lock:

	/*
	 * �r�����������Ă��āA�Q�Ƃ�r�����ł��Ȃ����A�G���[�ƂȂ�
	 * ���Ƀ��R�[�h���擾������A�����܂ł𑗐M����A�ǉ�(f_more)
	 * ���s�킹��
	 */
	if ( n ) {
		resp->f_more	= 1;
		goto	reply;
	} else 
		neo_errno = E_RDB_REC_BUSY;

err2: DBG_T("err2");
	neo_free( resp );
err1: 
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	prc_rewind()
 *
 *  �@�\	�J�[�\����擪�ɖ߂�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
prc_rewind( reqp, resp )
	r_req_rewind_t	*reqp;
	r_res_rewind_t	*resp;
{
	r_res_rewind_t	res;
	r_tbl_t		*td;
	r_usr_t		*ud;
DBG_ENT(M_RDB,"prc_rewind");

#ifdef XXX
	/*
	 * ���N�G�X�g���V�[�u
	 */
	len = sizeof(req);
	if( p_recv( pd, &req, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err1;
	}
#endif

	/*
	 * �e�[�u��/�I�[�v�����[�U�L�q�q�L�����`�F�b�N
	 */
	td = reqp->r_td;
	ud = reqp->r_ud;

	if( (neo_errno = check_td_ud( td, ud )) ) {
		goto err1;
	}

	/*
	 * �J�[�\����0�ɐݒ�
	 */
DBG_A(("cursor=%d\n", ud->u_cursor ));
	ud->u_cursor	= 0;

	/*
	 * ���v���C
	 */
	reply_head( (p_head_t*)reqp, (p_head_t*)resp, sizeof(res), 0 );

DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}
