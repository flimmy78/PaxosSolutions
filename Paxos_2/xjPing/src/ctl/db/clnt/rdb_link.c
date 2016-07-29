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

_dlnk_t		_neo_r_man;
static	int	_r_init;

static	p_port_t	myport, svcport;

/*****************************************************************************
 *
 *  �֐���	rdb_link()
 *
 *  �@�\	�f�[�^�x�[�X�ւ̐ڑ�
 * 
 *  �֐��l
 *     		����	DB_MAN�L�q�q�A�h���X
 *
 *		�ُ�	0(NULL)
 *
 ******************************************************************************/
r_man_t	*
rdb_link( myname, name )
	p_name_t	myname;		/* ���[�J���|�[�g��	*/
	r_man_name_t	name;		/* �f�[�^�x�[�X��	*/
{
	p_id_t		*pd;
	r_man_t		*md;
	r_req_link_t	req;
	r_res_link_t	res;
	int		len;
	extern char *_neo_procbname, _neo_hostname[];
	extern int  _neo_pid;

DBG_ENT(M_RDB,"rdb_link");
DBG_A(("myname=[%s],name=[%s]\n", myname, name ));

	if( !_r_init ) {
		_dl_init( &_neo_r_man );
		_r_init	= 1;
	}

	/*
	 * ���[�J���|�[�g�A�h���X���擾����
	 */
	if( p_getaddrbyname( myname, &myport ) ) {
		goto err1;
	}

	/*
	 * �T�[�o�|�[�g�A�h���X���擾����
	 */
	if( p_getaddrbyname( name, &svcport ) ) {
		goto err1;
	}

	/*
	 * ���[�J���ʐM�|�[�g���I�[�v������
	 */
	myport.p_role = P_CLIENT;
	if( !( pd = p_open( &myport ) ) )
		goto err1;

	/*
	 * DB_MAN �T�[�o�֐ڑ�����(�ʐM�|�[�g�ڑ�)
	 */
	if( p_connect( pd, &svcport ) ) {
		goto err2;
	}

	/*
	 * DB_MAN�T�[�o�֐ڑ�����(�f�[�^�x�[�X�ڑ��j
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_LINK, sizeof(r_req_link_t) );

	neo_setusradm( &req.l_usr, 
		_neo_procbname, _neo_pid, myname, _neo_hostname, PNT_INT32(pd) );

	if( p_request( pd, (p_head_t*)&req, sizeof(req) ) ) {
		goto err3;
	}
	
	/*
	 * �T�[�o����̐ڑ�������҂�
	 */
	len = sizeof(res);
	if( p_response( pd, (p_head_t*)&res, &len ) ) {
		goto err3;
	}
	
	if( (neo_errno = res.l_head.h_err) )
		goto err3;

	/*
	 * DB_MAN�L�q�q�𐶐�����
	 */
	if( !(md = (r_man_t *)neo_malloc( sizeof(r_man_t) )) ) {
		neo_errno	= E_RDB_NOMEM;
		goto err3;
	}
	memset( md, 0, sizeof(*md) );

	/*
	 * ��������DB_MAN�L�q�q���������āA_neo_r_man�֌q��
	 */
	md->r_ident	= R_MAN_IDENT;
	strncpy( md->r_name, name, sizeof(md->r_name) );

	_dl_insert( md, &_neo_r_man );
	_dl_init( &md->r_tbl );
	_dl_init( &md->r_result );
	_dl_init( &md->r_stmt );
	md->r_pd  = pd;
	pd->i_tag = md;

	neo_setusradm( &md->r_MyId, 
		_neo_procbname, _neo_pid, myname, _neo_hostname, PNT_INT32(pd) );

	neo_rpc_log_msg( "LINK", M_RDB, myname, &res.l_usr, "", "" );

DBG_EXIT(md);
	return( md );
err3:
	if ( p_disconnect( pd ) )
		goto	err1;
err2:
	p_close( pd );
err1:
DBG_EXIT(0);
	return( 0 );
}


/*****************************************************************************
 *
 *  �֐���	rdb_unlink()
 *
 *  �@�\	�f�[�^�x�[�X�Ɛؒf������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1 ���邢��
 *			�擪���N���[�Y�e�[�u���A�h���X
 *
 ******************************************************************************/
int
rdb_unlink( md )
	r_man_t	*md;		/* DB_MAN�L�q�q�A�h���X */
{
	r_req_unlink_t	req;
	r_res_unlink_t	res;
	int		len;

DBG_ENT(M_RDB,"rdb_unlink");

	/*
	 * DB_MAN�L�q�q�L���t���O���`�F�b�N����
	 */
	if( !md || md->r_ident != R_MAN_IDENT ) {
		neo_errno	= E_RDB_MD;
		goto err1;
	}

	/*
	 * �f�[�^�x�[�X�Ƃ̐ؒf������
	 */
	_rdb_req_head( (p_head_t*)&req, R_CMD_UNLINK, sizeof(req) );

	if( p_request( md->r_pd, (p_head_t*)&req, sizeof(req) ) )
		goto err1;

	len = sizeof(res);
	if( p_response( md->r_pd, (p_head_t*)&res, &len ) )
		goto err1;
	
	if( (neo_errno = res.u_head.h_err) )
		goto err1;

	/*
	 * �ʐM�|�[�g�Ԃ̐ؒf�����āADB_MAN�L�q�q�L���t���O�N���A
	 */
	if( p_disconnect( md->r_pd ) )
		goto err1;
	
	md->r_ident = 0;


	/*
	 * �ʐM�|�[�g���N���[�Y���āADB_MAN�L�q�q��_neo_r_man����
	 * �O���āA�G���A���������
	 */
	p_close( md->r_pd );

	_dl_remove( md );

	neo_proc_msg( "UNLINK", M_RDB, md->r_name, "", "" );

	neo_free( md );


DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(-1);
	return( -1 );
}


