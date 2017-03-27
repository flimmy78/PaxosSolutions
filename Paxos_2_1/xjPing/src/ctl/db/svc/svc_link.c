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
 *	svc_link.c
 *
 *	����		R_CMD_LINK
 *				R_CMD_UNLINK	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"
#include	"hash.h"

/*****************************************************************************
 *
 *  �֐���	svc_link()
 *
 *  �@�\	�N���C�A���g�̐ڑ��v���������āA�|�[�g�G���g���𐶐�
 *		���āA������Ԃ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_link( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
{
	r_req_link_t	req;
	r_res_link_t	res;
	r_port_t	*pe;
	int		len;
	extern char *_neo_procbname, _neo_hostname[];
	extern int  _neo_pid;

DBG_ENT(M_RDB,"svc_link");

	/*
	 * �ڑ��v����ǂ݂���
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() ) ;
		goto err1;
	}

	/*
	 * �|�[�g�G���g���𐶐�����
	 */
	if( !(pe = (r_port_t *)neo_zalloc( sizeof(r_port_t) ) ) ) {
		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() ) ;
		goto err1;
	}
	
	/*
	 * �|�[�g�G���g����ʐM�|�[�g�ւ̌q���A������������
	 */
	_dl_init( &pe->p_usr_ent );
	pd->i_tag	= (void*)pe;

	HashInit( &pe->p_HashUser, PRIMARY_11, HASH_CODE_STR, HASH_CMP_STR,
			NULL, neo_malloc, neo_free, NULL );

	/*
	 * usr_adm_t from client
	 */
	bcopy( &req.l_usr, &pe->p_clnt, sizeof(usr_adm_t) );

	/*
	 * �����p�P�b�g�쐬�A������Ԃ�
	 */
DBG_A(("reply(pe=0x%x)\n", pe ));
	reply_head( (p_head_t*)&req, (p_head_t*)&res, sizeof(res), 0 );

	neo_setusradm( &res.l_usr, 
		_neo_procbname, _neo_pid, svc_myname, _neo_hostname, PNT_INT32(pd) );

	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() ) ;
		goto err1;
	}

	neo_rpc_log_msg( "LINK", M_RDB, svc_myname, &pe->p_clnt, "", "" );

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	svc_unlink()
 *
 *  �@�\	�N���C�A���g�̐ؒf�v�����������āA������Ԃ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_unlink( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q�A�h���X */
	p_head_t	*hp;		/* �ؒf�v���p�P�b�g�w�b�_   */
{
	r_req_unlink_t	req;
	r_res_unlink_t	res;
	r_port_t	*pe;
	int		len;

DBG_ENT(M_RDB,"svc_unlink");

	/*
	 * �ؒf�v����ǂ݂���
	 */
	len = sizeof(req);
	if( p_recv( pd, (p_head_t*)&req, &len ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() ) ;
		goto err1;
	}
//DBG_PRT("UNLINK RECV\n");

	/*
	 * �|�[�g�G���g�����Ȃ���΁A�G���[������Ԃ�
	 */
	pe = PDTOENT( pd );
	if( !pe ) {
		neo_proc_err( M_RDB, svc_myname, LINKERR01, neo_errsym() ) ;
		err_reply( pd, hp, E_RDB_CONFUSED );
		goto err1;
	}

	neo_rpc_log_msg( "UNLINK", M_RDB, svc_myname, &pe->p_clnt, "", "" );

	/*
	 * �|�[�g�G���g������O�̌㏈��������
	 */
	svc_garbage( pd );

	/*
	 * �����p�P�b�g���쐬���āA������Ԃ�
	 */
	reply_head( (p_head_t*)&req, (p_head_t*)&res, sizeof(res), 0 );
	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) < 0 ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() ) ;
		goto err1;
	}
//DBG_PRT("UNLINK REPLY sizeof(res)=%d\n", sizeof(res) );

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	svc_garbage()
 *
 *  �@�\	�ؒf�O�̌㏈��
 * 
 *  �֐��l 	0
 *
 ******************************************************************************/
int
svc_rollback_td( r_tbl_t *td, r_usr_t *ud )
{
	int	i;
	int	inc;
			/*
			 * ���R�[�h�ɂ��Ă̌㏈��
			 */
			for( i = 0; i < td->t_rec_num; i++ ) {

				if( td->t_rec[i].r_access != ud )
					continue;

				/*
				 * ���R�[�h���b�N����
				 */
				if( td->t_rec[i].r_stat & R_LOCK )
					svc_rec_post( td, i );
					
				/* 
				 * ��������̕ύX��p������
				 */
				if( td->t_rec[i].r_stat & R_DIRTY ) {

					if( td->t_rec[i].r_stat & R_INSERT ) {

						inc = -1;
						if ( td->t_unused_index > i )
							td->t_unused_index = i;

					}  else if ( td->t_rec[i].r_stat 
								& R_UPDATE ) {
						if( td->t_rec[i].r_stat
								& R_USED )	
							inc = 0;
						else 
							inc = 1;
					}

					if( !svc_load( td, i, 1 ) ) {
						td->t_rec_used	+= inc;
						td->t_stat	|= R_UPDATE;
					}
				}

				td->t_rec[i].r_access	= 0;
				td->t_rec[i].r_stat	&= ~R_LOCK;
			}
			if( td->t_stat & R_UPDATE ) {
				td->t_version++;
				HashClear( &td->t_HashEqual );
			}
			/*
			 * �e�[�u���̃��b�N�͉������Ȃ�
			 */
//			if( td->t_lock_usr == ud && td->t_stat & R_LOCK ) {
//
//				svc_tbl_post( td );
//				
//			}
	return( 0 );
}

int
svc_garbage( pd )
	p_id_t	*pd;		/* �ʐM�|�[�g�L�q�q�A�h���X */
{
	r_port_t	*pe;
	r_twait_t	*twaitp;
	r_tbl_t		*td;
	r_usr_t		*ud;
	_dlnk_t		*dp;

DBG_ENT(M_RDB,"svc_garbage");

	/*
	 * �|�[�g�G���g��������ꍇ�ɂ͌㏈��������
	 */
	if( (pe = PDTOENT( pd )) ) {

		/*
		 * �҂��e�[�u��������΁A�Y���e�[�u����
		 * �҂����[�U���X�g����Y�����[�U���O���āA�̈���������
		 */
		if( (twaitp = pe->p_twait) ) {

			/* remove twait from td */
			td = twaitp->w_td;
			td->t_wait_cnt--;

			_dl_remove( twaitp );

			/*  unlink usr from twaitp */
			while( (dp = _dl_head(&twaitp->w_usr)) )
				_dl_remove( dp );

DBG_A(("free twaitp=0x%x\n", twaitp ));
			neo_free( twaitp );
		}

		/*
		 * �I�[�v�����[�U���N���[�Y����
		 */
		while( (ud = PDTOUSR(_dl_head( &pe->p_usr_ent ))) ) {

			td = ud->u_mytd;

			svc_rollback_td( td, ud );

#ifdef	ZZZ
#else
			/*
			 * �e�[�u���̃��b�N����������
			 */
			if( td->t_lock_usr == ud && td->t_stat & R_LOCK ) {

				svc_tbl_post( td );
			
			}
#endif
DBG_A(("close_user(ud=0x%x)\n", ud ));
			close_user( ud );

			if( td->t_stat & T_TMP ) {
DBG_B(("drop_td:t_usr_cnt=%d\n", td->t_usr_cnt ));
				drop_td( td );
			}
		}
DBG_A(("free pe=0x%x\n", pe ));
		
		/*
		 * �|�[�g�G���g�����
		 */
		ASSERT( HashCount(&pe->p_HashUser) == 0 );
		HashDestroy( &pe->p_HashUser );

		neo_free( pe );
		pd->i_tag	= 0;
	}
DBG_EXIT(0);
	return( 0 );
}
