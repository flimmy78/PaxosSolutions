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
 *	svc_insert.c
 *
 *	����	R_CMD_INSERT	 procedure
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"
#include	"./hash.h"

/*****************************************************************************
 *
 *  �֐���	svc_insert()
 *
 *  �@�\	�N���C�A���g����]������Ă������R�[�h��
 *		�w�肵���e�[�u���ɑ}�����āA������Ԃ�
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_insert( pd, hp )
	p_id_t		*pd;		/* �ʐM�|�[�g�L�q�q�A�h���X	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_		*/
{
	int	len, off;
	r_req_insert_t	*reqp;
	r_res_insert_t	res;
	r_tbl_t		*td;
	r_usr_t		*ud;
register
	int		i, j, k;
register
	int		stat;
	r_head_t	*rp;
	int		num_old;
	int		n;
	int		mode;
	int		m;
	r_item_t	**index = 0;
	r_item_name_t	*items;
	int		err;
//	int	l;
	bool_t	bDiff;

DBG_ENT(M_RDB,"svc_insert");

	/*
	 * �v���p�P�b�g��M�p�o�b�t�@�𐶐�����
	 */
	len = hp->h_len + sizeof(p_head_t);
	if( !(reqp = (r_req_insert_t*)neo_malloc(len) ) ) {

		len = sizeof(p_head_t);
		if( p_recv( pd, hp, &len ) )	/* discard */
			goto err1;

		err_reply( pd, hp, E_RDB_NOMEM );
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR03, neo_errsym() );
		goto err1;

	}

	/*
	 * �v���p�P�b�g����M����
	 */
	if( p_recv( pd, (p_head_t*)reqp, &len ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR01, neo_errsym() );
		goto err2;
	}

	/*
	 * �e�[�u���L�q�q�ƃI�[�v�����[�U�L�q�q�̗L���`�F�b�N
	 */
	r_port_t	*pe = PDTOENT(pd);

	td = (r_tbl_t*)HashGet(&_svc_inf.f_HashOpen, reqp->i_name);
	ud = (r_usr_t*)HashGet( &pe->p_HashUser, reqp->i_name );

	n	= reqp->i_rec;
	mode	= reqp->i_mode;
	m	= reqp->i_item;

	if( (neo_errno = check_td_ud( td, ud ))
			|| (neo_errno = check_lock( td, ud, R_WRITE ) ) ) {
		err_reply( pd, hp, neo_errno );
		neo_rpc_log_err( M_RDB, svc_myname, &( PDTOENT(pd)->p_clnt ), 
						RDBCOMERR09, neo_errsym() );
		goto err2;
	}

	/*
	 * �d���o�^�`�F�b�N����ꍇ�ɂ́A�`�F�b�N�p�⏕�G���A��p�ӂ���
	 */
	if( m ) {
		if( !(index = (r_item_t**)neo_malloc( m* sizeof(r_item_t*))) ) {

			err_reply( pd, hp, E_RDB_NOMEM );

			goto err2;
		}
		items = (r_item_name_t*)((char*)(reqp+1)+n*td->t_rec_size);
	}

	/*
	 * �`�F�b�N����A�C�e������L�⏕�G���A�ɐݒ肷��
	 */
	for( j = 0; j < m; j++ ) {
		for( i = 0; td->t_scm->s_n; i++ ) {
			if(!strcmp( items[j], td->t_scm->s_item[i].i_name ) ) {

				index[j] = &td->t_scm->s_item[i];

				goto next;
			}
		}
		err_reply( pd, hp, E_RDB_NOITEM );
		neo_rpc_log_err( M_RDB, svc_myname, &( PDTOENT(pd)->p_clnt ), 
						INSERTERR01, neo_errsym() );
		goto err3;
	next:;
	}

	/*
	 * �e�[�u���̑S�Ẵ��R�[�h�ɑ΂��āA�w�肵���A�C�e���Ńe�[�u����
	 * ���R�[�h�Ɠ]�����ꂽ�o�^���R�[�h���r���āA���R�[�h�̏d���o�^
	 * �̃`�F�b�N���s��
	 */
	num_old	= td->t_rec_num;

	if( m ) {
	  for( i = 0; i < num_old; i++ ) {

		/*
		 * �L���b�V��������Ă��Ȃ����R�[�h���t�@�C������
		 * ���[�h����
		 */
		if( !(td->t_rec[i].r_stat & R_REC_CACHE) )
			svc_load( td, i, R_PAGE_SIZE/td->t_rec_size + 1 );

		/*
		 * ���g�p���R�[�h�̈�̓`�F�b�N���Ȃ�
		 */
		if( td->t_rec[i].r_stat & R_USED ) {
			/* Cksum64 */
			if( td->t_rec[i].r_head->r_Cksum64 ) {
				assert( !in_cksum64(td->t_rec[i].r_head, td->t_rec_size, 0 ) );
			}
			bDiff = TRUE;
			for( k = 0; k < n; k++  ) {
				for( j = 0; j < m; j++ ) {
					off	= index[j]->i_off;
					len	= index[j]->i_len;
					if( bcmp( 
						(char*)(reqp+1)
							+ k*td->t_rec_size
							+ off,
						(char*)td->t_rec[i].r_head+off,
							len ) ) {
						goto next_diff;
					}
				}
				bDiff = FALSE;
				break;
next_diff:;
			}
			if( !bDiff ) {
				err_reply( pd, hp, E_RDB_EXIST);

				neo_rpc_log_err( M_RDB, 
					svc_myname, 
					&(PDTOENT(pd)->p_clnt), 
					INSERTERR02,
					neo_errsym() );
				goto err3;
			}
		}
	  }
	}

for( k = 0; k < n; k++ ) {
	/*
	 * ���g�p���R�[�h�̈��{���o��
	 */
	for( i = td->t_unused_index /*0*/; i < num_old; i++ ) {

		/*
		 * �L���b�V��������Ă��Ȃ����R�[�h���t�@�C������
		 * ���[�h����
		 */
		if( !((stat = td->t_rec[i].r_stat) & R_REC_CACHE) )
			svc_load( td, i, R_PAGE_SIZE/td->t_rec_size + 1 );

		if( !(td->t_rec[i].r_stat& R_USED ) ) 
			goto find;

	}

	/*
	 * �󂫗̈悪���݂��Ă��Ȃ��ꍇ�ɂ́A�e�[�u���̊g�����s��
	 *	I must expand area
	 *		All data shall be on core.
	 */
	if( (err = svc_switch( td, num_old, num_old*2 )) ) {
		err_reply( pd, hp, err );
		neo_proc_err( M_RDB, svc_myname, INSERTERR03, neo_errsym() );
		goto err3;
	}

	for( i = num_old; i < num_old*2; i++ )
		td->t_rec[i].r_stat	|= R_REC_CACHE;

	i = num_old;
	num_old	= td->t_rec_num;

find:
	/*
	 * �������󂫗̈���g���āA���R�[�h��}������
	 * �܂��A�e�[�u���̏����X�V����
	 */
//LOG(neo_log,"k=%d i=%d num_old=%d rec_num=%d", k, i, num_old, td->t_rec_num );

	rp = td->t_rec[i].r_head;

	bcopy( (char*)(reqp+1) + k*td->t_rec_size, rp, td->t_rec_size );
DBG_A(("rp=0x%x<%s>,i=%d\n", rp, rp+1, i ));

	rp->r_rel = rp->r_abs = i;
	rp->r_stat = td->t_rec[i].r_stat &= R_REC_MASK;
	rp->r_stat = td->t_rec[i].r_stat |= R_INSERT|R_USED;

	td->t_rec[i].r_stat	|= R_BUSY;
	td->t_rec[i].r_access	= ud;

	r_item_t	*ip;
	for( j = 0; j < td->t_scm->s_n; j++ ) {
		ip	= &td->t_scm->s_item[j];

		hash_add( td, ip, i );
	}
	td->t_rec_used++;
	ud->u_added ++;
	ud->u_insert++;
	td->t_unused_index	= i + 1;
	td->t_stat	|= R_UPDATE;

	/*
	 * ���b�N���[�h��������A�Y�����R�[�h�̏�Ԃɐݒ肷��
	 */
	if( mode & R_LOCK ) {
		td->t_rec[i].r_stat	|= (mode&R_LOCK);
	}
	rp->r_Cksum64 = 0;
	if( _svc_inf.f_bCksum ) {
		rp->r_Cksum64 = in_cksum64( rp, td->t_rec_size, 0 );
	}

}
	if( mode & R_LOCK ) {
		if( !HashGet( &td->t_HashRecordLock, ud ) ) {
			HashPut( &td->t_HashRecordLock, ud, ud );
		}
	}
	td->t_version++;
	/*
	 * �����p�P�b�g���쐬���A�v�����ɕԂ�
	 */
	reply_head( (p_head_t*)reqp, (p_head_t*)&res, sizeof(res), 0 );

	res.i_rec_num	= td->t_rec_num;
	res.i_rec_used	= td->t_rec_used;
	bcopy( rp, &res.i_rec, sizeof(res.i_rec) );

	if( p_reply( pd, (p_head_t*)&res, sizeof(res) ) ) {
		neo_proc_err( M_RDB, svc_myname, RDBCOMERR02, neo_errsym() );
		goto err3;
	}

	if( index )	neo_free( index );
	neo_free( reqp );

DBG_EXIT(0);
	return( 0 );

err3: DBG_T("err3");
	if( index )	neo_free( index );
err2: DBG_T("err2");
	neo_free( reqp );
err1: 
DBG_EXIT(neo_errno);
	return( neo_errno );
}
