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

r_tbl_t		*_search_tbl;
r_usr_t		*_search_usr;


int
prc_search( reqp, resp )
	r_req_search_t	*reqp;
	r_res_search_t	*resp;
{
	r_tbl_t		*td;
	r_usr_t		*ud;
	int		i;
	int		n;

	/* ��͖؂̃��[�g�ւ̃|�C���^(in clnt/rdb_gram.c) */
	extern r_expr_t	*_Rdb_yyreturn; 

DBG_ENT(M_RDB,"prc_search");

	/*
	 * �e�[�u��/�I�[�v�����[�U�L�q�q�L�����`�F�b�N
	 */
	td = reqp->s_td;
	ud = reqp->s_ud;

	n = reqp->s_n;

	if( (neo_errno = check_td_ud( td, ud )) ) {
		goto err1;
	}

	/*
	 * ���������̐ݒ�
	 *  ( lex_start(), yyparse()�� clnt/rdb_expr.c, clnt/rdb_gram.c)
	 */
	if( n ) {
		_search_tbl	= td;
		_search_usr	= ud;

		_search_usr->u_search.s_cnt	= 0;

DBG_A(("str=[%s]\n", reqp+1 ));
		
		/*
		 * �����͏����O����
		 */
		lex_start( (char*)(reqp+1), _search_usr->u_search.s_text,
				 sizeof(_search_usr->u_search.s_text) );


		/*
		 * �\����͖؍쐬
		 *   yyparse() �̃��^�[���l�́A���펞�� 0 �A�ُ펞�� 1
		 *   _Rdb_yyreturn �́A����� yyparse() ���I���������ɁA
		 *   ��͖؂̃��[�g�ւ̃|�C���^���ݒ肳��܂��B
		 */
		if( yyparse() ) {

			neo_errno = E_RDB_SEARCH;
			goto err1;
		}

		_search_usr->u_search.s_expr	= _Rdb_yyreturn;
		_search_usr->u_cursor		= 0;

	} else {
		ud->u_search.s_expr	= 0;
		ud->u_cursor		= 0;
	}

	/*
	 * �r���������Ă������R�[�h�̃��b�N����������
	 * ���̍ۂɁA�҂����[�U������Βʒm����
	 */
	for( i = 0; i < td->t_rec_num; i++ ) {
		if( (td->t_rec[i].r_access == ud)
			&& (td->t_rec[i].r_stat & R_LOCK) ) {

			td->t_rec[i].r_stat	&= ~R_LOCK;

			svc_rec_post( td, i );
		}
	}

	/*
	 * ���v���C
	 */
	reply_head( (p_head_t*)reqp, (p_head_t*)resp, sizeof(*resp), 0 );

DBG_EXIT(0);
	return( 0 );
err1: 
DBG_EXIT(neo_errno);
	return( neo_errno );
}
