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
 *	rdb_search.c
 *
 *	����	DB_MAN library R_CMD_SEARCH
 *				���������̐ݒ�
 * 
 ******************************************************************************/

#include	"neo_db.h"

/******************************************************************************
 * �֐���
 *		rdb_search()
 * �@�\        
 *		���������̐ݒ�
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
rdb_search( td, str )
	r_tbl_t		*td;	/* �e�[�u���L�q�q	*/
	char		*str;	/* ���������i�e�L�X�g�j	*/
{
	r_req_search_t	*reqp;
	r_res_search_t	res;
	int		len;
	int		n;

DBG_ENT(M_RDB,"rdb_search");

	/*
	 *	�����`�F�b�N
	 */
	if( td->t_ident != R_TBL_IDENT ) {
#ifdef	ZZZ
#else
		neo_errno = E_RDB_TD;
		goto err1;
#endif
	}
	if( str ) {
		if( (n = strlen( str ) + 1) > R_TEXT_MAX) {
			neo_errno = E_RDB_INVAL;
			goto err1;
		}
	} else {
		n = 0;
	} 

	/*
	 *	���N�G�X�g�o�b�t�@�̃������A���b�N
	 *	( R_CMD_SEARCH �̃p�P�b�g�͉ϒ� )
	 */
	len = sizeof(r_req_search_t) + n;
	if( !(reqp = (r_req_search_t*)neo_malloc(len) ) ) {
		neo_errno = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 *	 ���N�G�X�g���̐ݒ�
	 */
	_rdb_req_head( (p_head_t*)reqp, R_CMD_SEARCH, len );
	
	strncpy( reqp->s_name, td->t_name, sizeof(reqp->s_name) );
	reqp->s_n	= n;

	if( n )
		strcpy( (char*)(reqp+1), str );

	/*
	 *	���N�G�X�g
	 */
	if( p_request( TDTOPD(td), (p_head_t*)reqp, len ) )
		goto	err2;

	/*
	 *	���X�|���X
	 */
	len = sizeof(r_res_search_t);
	if( p_response( TDTOPD(td), (p_head_t*)&res, &len ) )
		goto	err2;
	
	if( (neo_errno = res.s_head.h_err) )
		goto err2;

	/*
	 *	�ʐM�o�b�t�@�̃��������
	 */
	neo_free( reqp );

DBG_EXIT(0);
	return( 0 );
err2: DBG_T("err2");
	neo_free( reqp );
err1:
DBG_EXIT(-1);
	return( -1 );
}
