/*******************************************************************************
 * 
 *  svc_misc.c ---xjPing (On Memory DB) Misc programs for Server
 * 
 *  Copyright (C) 2011-2016 triTech Inc. All Rights Reserved.
 * 
 *  See GPL-LICENSE.txt for copyright and license details.
 * 
 *  This program is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free Software
 *  Foundation; either version 3 of the License, or (at your option) any later
 *  version. 
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License along with
 *  this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 ******************************************************************************/
/* 
 *   �쐬            �n�ӓT�F
 *   ����
 *   �����A���쌠    �g���C�e�b�N
 */ 
/******************************************************************************
 *	svc_misc.c
 *
 *	����	db_man server   ���ʊ֐�
 *				���̑�����
 ******************************************************************************/
#include	"neo_db.h"
#include	"svc_logmsg.h"

/*****************************************************************************
 *
 *  �֐���	check_td_ud()
 *
 *  �@�\	�e�[�u���A�I�[�v�����[�U�L�q�q�L���`�F�b�N
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
check_td_ud( td, ud )
	r_tbl_t	*td;
	r_usr_t	*ud;
{
	int	err;

	if( (err = check_td( td )) || (err = check_ud( ud)) )
		goto ret;
	
	if( td != ud->u_mytd )
		err = E_RDB_INVAL;

	td->t_access	= time( 0 );

ret:
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	check_td()
 *
 *  �@�\	�e�[�u���L�q�q�L���`�F�b�N
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
check_td( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
	int	err = 0;

DBG_ENT(M_RDB,"check_td");

/*************************************************************
DBG_A(("td=0x%x(edata=0x%x,end=0x%x,sbrk(0)=0x%x)\n", 
		td, neo_edata, neo_end, sbrk(0)));
	if( CHK_ADDR( td ) ) {
		err = E_RDB_INVAL;
		goto ret;
	}
*************************************************************/

#ifdef ZZZ
	if( td->t_ident != R_TBL_IDENT ) {
#else
	if( !td || td->t_ident != R_TBL_IDENT ) {
#endif
		err = E_RDB_TD;
		goto ret;
	}
ret:
DBG_EXIT(err);
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	check_ud()
 *
 *  �@�\	�I�[�v�����[�U�L�q�q�L���`�F�b�N
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
check_ud( ud )
	r_usr_t	*ud;		/* �I�[�v�����[�U�L�q�q	*/
{
	int	err = 0;

DBG_ENT(M_RDB,"check_ud");

/**********************************************
	if( CHK_ADDR( ud ) ) {
		err = E_RDB_INVAL;
		goto ret;
	}
**********************************************/

#ifdef	ZZZ
	if( ud->u_ident != R_USR_IDENT ) {
#else
	if( !ud || ud->u_ident != R_USR_IDENT ) {
#endif
		err = E_RDB_UD;
		goto ret;
	}
ret:
DBG_EXIT(err);
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	check_lock()
 *
 *  �@�\	�e�[�u������\���`�F�b�N
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
check_lock( td, ud, mode )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
	r_usr_t	*ud; 		/* �I�[�v�����[�U�L�q�q */
	int	mode;		/* ����^�C�v		*/
{
	int	err = 0;

DBG_ENT(M_RDB,"check_access");

	if( (td->t_stat & R_LOCK ) && (ud != td->t_lock_usr) ) {

		if( td->t_stat & R_EXCLUSIVE ) {
			err = E_RDB_TBL_BUSY;
		} else if( mode & R_WRITE ) {
			err = E_RDB_TBL_BUSY;
		}
	}
DBG_EXIT(err);
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	reply_head()
 *
 *  �@�\	�����p�P�b�g�w�b�_���쐬
 * 
 *  �֐��l
 *     		����
 *
 *****************************************************************************/
void
reply_head( reqp, resp, len, err )
	p_head_t	*reqp;		/* �v���p�P�b�g�w�b�_	*/
	p_head_t	*resp;		/* �����p�P�b�g�w�b�_	*/
	int		len;		/* �����p�P�b�g�T�C�Y	*/
	int		err;		/* �G���[�ԍ�		*/
{
	bcopy( reqp, resp, sizeof(p_head_t) );
	resp->h_len	= len - sizeof(p_head_t);
	resp->h_err	= err;
}


/*****************************************************************************
 *
 *  �֐���	err_reply()
 *
 *  �@�\	�G���[�������p�P�b�g�̑��M
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
err_reply( pd, hp, err )
	p_id_t		*pd;		/* �ʐM�|�[�g�A�h���X	*/
	p_head_t	*hp;		/* �v���p�P�b�g�w�b�_	*/
	int		err;		/* �G���[�ԍ�		*/
{
	p_head_t	*pH;

	pH	= (p_head_t*)neo_malloc( sizeof(p_head_t) );
	memcpy( pH, hp, sizeof(*pH) );
	neo_errno	= pH->h_err	= err;
	pH->h_len	= 0;

	p_reply_free( pd, pH, sizeof(*pH) );

DBG_EXIT(neo_errno);
	return( neo_errno );
}

#define	OPEN_IDLE_MAX	50

void	hash_destroy();

r_tbl_t*
svc_tbl_alloc()
{
	r_tbl_t	*tdp;

	tdp = _r_tbl_alloc();
	HashInit( &tdp->t_HashEqual, PRIMARY_11, HASH_CODE_STR, HASH_CMP_STR,
				NULL, neo_malloc, neo_free, hash_destroy );	
	HashInit( &tdp->t_HashRecordLock, PRIMARY_11, HASH_CODE_PNT, HASH_CMP_PNT,
				NULL, neo_malloc, neo_free, NULL );	

	return( tdp );
}

void
svc_tbl_free( r_tbl_t *tdp )
{
	HashDestroy( &tdp->t_HashRecordLock );
	HashDestroy( &tdp->t_HashEqual );
	_r_tbl_free( tdp );
}

/*****************************************************************************
 *
 *  �֐���	drop_table()
 *
 *  �@�\	�e�[�u���h���b�v
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
drop_table()
{
	r_tbl_t	*td;
	r_tbl_t	*nd;
	long	t;
	int	cnt;
	int	err;

DBG_ENT(M_RDB,"drop_table");

	t = time( 0 );

	for( td = (r_tbl_t*)_dl_head(&(_svc_inf.f_opn_tbl)), cnt = 0;
			_dl_valid(&(_svc_inf.f_opn_tbl),td); td = nd ) {

		nd	= (r_tbl_t *)_dl_next( td );

		if( td->t_usr_cnt == 0 ) {

			cnt++;

			if( (t - td->t_access) > R_DROP_TABLE_TIME 
				|| cnt > OPEN_IDLE_MAX /* older ones */ ) {

				/* moved from prc_close */
				if( td->t_md ) {
					if( (err = svc_store
						(td, 0, td->t_rec_num, 0)) ){
						goto err1;
					}
				}

				if ( (err = drop_td( td )) )
					goto	err1;
			}
		}
	}
DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(err);
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	drop_td()
 *
 *  �@�\	�e�[�u���������������������
 * 
 *  �֐��l
 *     		����  	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
drop_td( td )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
{
	int	err;

DBG_ENT(M_RDB,"drop_td");

	/*
	 * �o�b�t�@�t���[
	 */
	if ( (err = rec_buf_free( td )) )
		goto	err1 ;

	if ( (err = rec_cntl_free( td )) )
		goto	err1 ;
	/*
	 *	remove td from list
	 */
	_dl_remove( td );
	td->t_ident	= 0;

	HashRemove( &_svc_inf.f_HashOpen, td->t_name );

	if ( (err = TBL_CLOSE( td )) )
		goto	err1;

	/*
	 *  Free tbl
	 */
	 svc_tbl_free( td );

DBG_EXIT(0);
	return( 0 );

err1:
DBG_EXIT(err);
	return( err );

}


/*****************************************************************************
 *
 *  �֐���	svc_switch()
 *
 *  �@�\	�e�[�u���T�C�Y�̕ύX
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_switch( td, store_rec_num, new_rec_num )
	r_tbl_t	*td;			/* �e�[�u���L�q�q	*/
	int	store_rec_num;		/* �������ރ��R�[�h��	*/
	int	new_rec_num;		/* �X�V�ヌ�R�[�h��	*/
{
	int	i;
	r_tbl_t		*tdwp;
	r_rec_t		*recwp;

DBG_ENT(M_RDB,"svc_switch");
DBG_A(("rec_num=%d,store_rec_num=%d,new_rec_num=%d\n",
		td->t_rec_num, store_rec_num, new_rec_num ));

	if( !(tdwp = svc_tbl_alloc() ) ) {
		goto err1;
	}

	tdwp->t_rec_num		= new_rec_num;
	tdwp->t_rec_size	= td->t_rec_size;

	if( rec_cntl_alloc( tdwp ) ) {
		goto err2;
	}
	if( rec_buf_alloc( tdwp ) ) {
		goto err3;
	}
	for( i = 0; i < store_rec_num; i++ ) {

		tdwp->t_rec[i].r_stat	= td->t_rec[i].r_stat;
		tdwp->t_rec[i].r_access	= td->t_rec[i].r_access;
		tdwp->t_rec[i].r_wait	= td->t_rec[i].r_wait;

		bcopy( td->t_rec[i].r_head, tdwp->t_rec[i].r_head,
					td->t_rec_size );
	}
	recwp		= tdwp->t_rec;
	tdwp->t_rec	= td->t_rec;
	td->t_rec	= recwp;

	td->t_rec_num	= new_rec_num;
	tdwp->t_rec_num	= store_rec_num;

	if( TBL_CNTL( td, 0, 0 ) ) {
		goto err4;
	}
	if( drop_td( tdwp ) ) {
		goto err4;
	}
	td->t_version++;

	DBG_EXIT( 0 );
	return( 0 );

err4:	rec_buf_free( tdwp );
err3:	rec_cntl_free( tdwp );
err2:	svc_tbl_free( tdwp );
err1:
	DBG_EXIT( neo_errno );
	return( neo_errno );
#ifdef XXX
	/*
	 *	save
	 */
	item_num = td->t_scm->s_n;
	if( !(items = (r_item_t*)neo_malloc(item_num*sizeof(r_item_t)) ) ) {
		err = E_RDB_NOMEM;
		goto err1;
	}
	bcopy( td->t_scm->s_item, items, item_num*(sizeof(r_item_t)) );

	/*
	 *	file delete/create
	 */
	old_rec_num	= td->t_rec_num;
	used		= td->t_rec_used;

	if( TBL_CLOSE( td ) 
		|| TBL_DROP( td->t_name ) 
		|| TBL_CREATE( td->t_name, td->t_type, td->t_modified,
						new_rec_num, item_num, items )
		|| TBL_OPEN( td, td->t_name ) ) {

		err = E_RDB_CONFUSED;
		goto err2;
	}
	neo_free( items );

	for ( i = 0; i < store_rec_num; i++ ) {
		td->t_rec[i].r_stat	|= R_UPDATE;
	}
	if( err = svc_store( td, 0, store_rec_num, 0 ) ) {
		goto err2;
	}
	/*
	 *	free old memories
	 */
	td->t_rec_num	= old_rec_num;

	if ( err = rec_buf_free( td ) )
		goto	err1;
	if ( err = rec_cntl_free( td ) )
		goto	err1;
	/*
	 *	attach new control
	 */
	td->t_rec_num	= new_rec_num;
	td->t_rec_used	= used;

	if( err = rec_cntl_alloc( td ) ) {
		goto err1;
	}
	if( err = rec_buf_alloc( td ) ) {
		rec_cntl_free( td );
		goto err1;
	}

	if ( err = TBL_CNTL( td, 0, 0 ) )
		goto	err1;

	if( err = svc_load( td, 0, old_rec_num ) )
		goto err1;
DBG_EXIT(0);
	return( 0 );

err2: DBG_T("err2");
	neo_free( items );
err1:
DBG_EXIT(err);
	return( err );
#endif
}

