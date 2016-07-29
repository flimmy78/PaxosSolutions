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
 *	svc_commit.c
 *
 *	����		R_CMD_COMMIT
 *				R_CMD_ROLLBACK	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"
#include	"hash.h"

/*****************************************************************************
 *
 *  �֐���	svc_commit()
 *
 *  �@�\	�����e�[�u����flush����
 * 
 *  �֐��l
 *   	����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_commit( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	r_req_commit_t	req;
	r_res_commit_t	res;
	int		len;
	int	Ret;
DBG_ENT(M_RDB,"svc_commit");

	/*
	 * �ڑ��v����ǂ݂���
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() ) ;
		goto err1;
	}

	Ret = sql_commit( pd, NULL );
	/*
	 * �����p�P�b�g�쐬�A������Ԃ�
	 */
DBG_A(("reply(pe=0x%x)\n", pe ));
	reply_head( (p_head_t*)&req, (p_head_t*)&res, sizeof(res), Ret );

	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() ) ;
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
 *  �֐���	svc_rollback()
 *
 *  �@�\	�����e�[�u����flush����
 * 
 *  �֐��l
 *   	����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_rollback( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	r_req_rollback_t	req;
	r_res_rollback_t	res;
	int		len;
	int	Ret;
DBG_ENT(M_RDB,"svc_rollback");

	/*
	 * �ڑ��v����ǂ݂���
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() ) ;
		goto err1;
	}

	Ret = sql_rollback( pd, NULL );
	/*
	 * �����p�P�b�g�쐬�A������Ԃ�
	 */
DBG_A(("reply(pe=0x%x)\n", pe ));
	reply_head( (p_head_t*)&req, (p_head_t*)&res, sizeof(res), Ret );

	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() ) ;
		goto err1;
	}
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}
