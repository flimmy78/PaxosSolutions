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
 *	rdb_stop.c
 *
 *	����		DB_MAN �T�[�o���~������ 
 *			       R_CMD_RESTART
 *				DB_MAN �T�[�o���ăX�^�[�g������
 * 
 ******************************************************************************/

#include	"neo_db.h"

/******************************************************************************
 * �֐���
 *		rdb_stop()
 * �@�\        
 *		DB_MAN �T�[�o���~������
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
rdb_stop( md )
	r_man_t		*md;	/* DB_MAN �L�q�q	*/
{
	r_req_stop_t	req;
	r_res_stop_t	res;
	int		len;

DBG_ENT(M_RDB,"rdb_stop");

	/*
	 * 	���� �`�F�b�N
	 */
	if( md->r_ident != R_MAN_IDENT ) {
		neo_errno = E_RDB_MD;
		goto err1;
	}
	if( _dl_head(&md->r_tbl) ) {
		neo_errno = E_RDB_TBL_BUSY;
		goto err1;
	}

	/*
	 *	���N�G�X�g
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_STOP, sizeof(req) );

	if( p_request( md->r_pd, (p_head_t*)&req, sizeof(req) ) )
		goto err1;

	/*
	 *	���X�|���X
	 */
	len	= sizeof(res);
	if( p_response( md->r_pd, (p_head_t*)&res, &len )
						||(neo_errno = res.s_head.h_err) )
		goto err1;

DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(-1);
	return( -1 );
}

int
rdb_shutdown( md )
	r_man_t		*md;	/* DB_MAN �L�q�q	*/
{
	r_req_stop_t	req;
	r_res_stop_t	res;
	int		len;

DBG_ENT(M_RDB,"rdb_stop");

	/*
	 * 	���� �`�F�b�N
	 */
	if( md->r_ident != R_MAN_IDENT ) {
		neo_errno = E_RDB_MD;
		goto err1;
	}
	if( _dl_head(&md->r_tbl) ) {
		neo_errno = E_RDB_TBL_BUSY;
		goto err1;
	}

	/*
	 *	���N�G�X�g
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_SHUTDOWN, sizeof(req) );

	if( p_request( md->r_pd, (p_head_t*)&req, sizeof(req) ) )
		goto err1;

	/*
	 *	���X�|���X
	 */
	len	= sizeof(res);
	if( p_response( md->r_pd, (p_head_t*)&res, &len )
					||(neo_errno = res.s_head.h_err) )
		goto err1;

DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(-1);
	return( -1 );
}

/******************************************************************************
 * �֐���
 *		rdb_restart()
 * �@�\        
 *		DB_MAN �T�[�o���ăX�^�[�g������
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
rdb_restart( md, path )
	r_man_t		*md;	/* DB_MAN �L�q�q	*/
	r_path_name_t	path;	/* �f�[�^�x�[�X�p�X��	*/
{
	r_req_restart_t	req;
	r_res_restart_t	res;
	int		len;

DBG_ENT(M_RDB,"rdb_restart");
DBG_A(("path-name = (%s)\n", path ));

	/*
	 * 	���� �`�F�b�N
	 */
	if( md->r_ident != R_MAN_IDENT ) {
		neo_errno = E_RDB_MD;
		goto err1;
	}
	if( path ) {
		if( strlen( path ) >= R_PATH_NAME_MAX ) {
			neo_errno = E_RDB_INVAL;
			goto err1;
		}
	}

	/*
	 *	���N�G�X�g
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_RESTART, sizeof(req) );
	if( path )
		strcpy( req.s_path, path );
	else
		*(req.s_path) = 0;

	if( p_request( md->r_pd, (p_head_t*)&req, sizeof(req) ) )
		goto err1;

	/*
	 *	���X�|���X
	 */
	len	= sizeof(res);
	if( p_response( md->r_pd, (p_head_t*)&res, &len )
						||(neo_errno = res.s_head.h_err) )
		goto err1;

DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(-1);
	return( -1 );
}
