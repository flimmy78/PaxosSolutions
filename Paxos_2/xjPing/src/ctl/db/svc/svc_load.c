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
 *	svc_load.c
 *
 *	����		�e�[�u���̃t�@�C������̃��[�h�y��
 *				�t�@�C���ւ̃X�g�A
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"svc_logmsg.h"
#include	"join.h"

/*****************************************************************************
 *
 *  �֐���	svc_load()
 *
 *  �@�\	�t�@�C�����������Ƀ��[�h����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_load( td, s, l )
	r_tbl_t	*td;		/* �e�[�u���L�q�q		*/
	int	s;		/* ���[�h�J�n���R�[�h�ԍ�	*/
	int	l;		/* ���[�h���郌�R�[�h�̐�	*/
{
	int	e, i, n, j;
	r_rec_t	*cp;

DBG_ENT(M_RDB,"svc_load");
DBG_A(("s=%d,l=%d\n", s, l ));

	/*
	 * ���[�h���郌�R�[�h�̏I���ԍ��ݒ�
	 */
	e = s + l -1;
	if( e >= td->t_rec_num )
		e = td->t_rec_num - 1;

	/*
	 *	Swap in
	*/
	for( i = s; i <= e; i++ ) {
		if( td->t_rec[i].r_stat & R_REC_SWAP ) {
			xj_swap_in_td( td, i, e );
		}
	}
	for( i = s; i <= e; i++ ) {
		if( !(td->t_rec[i].r_stat & R_REC_CACHE )
			|| td->t_rec[i].r_stat & R_DIRTY )
			goto loop;
	}
	DBG_EXIT( 0 );
	return( 0 );
loop:
	/*
	 * �t���[�Z�O�����g�̐擪�� s �Ƃ���
	 */
	i = s;
	n = 0;

	i++; n++;

	/*
	 * �Y���t���[�Z�O�����g�̍Ō��{��
	 */
	while( i <= e ) {
		cp = &td->t_rec[i];
		if( cp->r_stat & R_REC_FREE ) {
			break;
		} else {
			n++;
			i++;
		}
	}

	/*
	 * �Y���t���[�Z�O�����g�ɑΉ����郌�R�[�h
	 * �����[�h����
	 */
DBG_A(("LOAD(s=%d,n=%d,i=%d,e=%d)\n", s, n, i, e));
LOG( neo_log, LogDBG, "[%s] s=%d n=%d i=%d e=%d", td->t_name, s, n, i, e );
	if( TBL_LOAD( td, s, n, (char*)td->t_rec[s].r_head ) < 0 )
		goto err1;

	for( j = s; j < s+n; j++ ) {

		td->t_rec[j].r_stat	&= ~(R_USED|R_BUSY|R_DIRTY);
		td->t_rec[j].r_stat	|= td->t_rec[j].r_head->r_stat&R_USED;

		td->t_rec[j].r_stat	|= R_REC_CACHE;
	}

	/*
	 * �����[�h���R�[�h�c���Ă���Ȃ�΁A
	 * ���̃t���[�Z�O�����g��{��
	 */
	if( (s = i) <= e )
		goto loop;

LOG( neo_log, LogDBG, "EXT(0)" );
DBG_EXIT(0);
	return( 0 );
err1:
LOG( neo_log, LogDBG, "EXT(%d)", neo_errno );
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	svc_store()
 *
 *  �@�\	�e�[�u�����t�@�C���֏�������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
svc_store( td, s, l, ud )
	r_tbl_t	*td;		/* �e�[�u���L�q�q	*/
	int	s;		/* �J�n���R�[�h		*/
	int	l;		/* ���R�[�h�̐�		*/
	r_usr_t	*ud;		/* �I�[�v�����[�U�L�q�q	*/
{
	int	is;
	r_rec_t	*cp;
	int	i, n;
	int	k;
	int	e;
DBG_ENT(M_RDB,"svc_store");

	e = s + l;
	i = s;
DBG_A(("s=%d,l=%d,e=%d\n", s, l, e ));
loop:
	n = 0;

	/*
	 * ���A�K�v�ȃ��R�[�h�̊J�n�ԍ���{��
	 * [ �ҏW���ꂽ(�X�V�A�}���Ȃ�)���R�[�h����������������]
	 */
	while( i < e ) {
		cp = &td->t_rec[i];
		if( (cp->r_stat & R_DIRTY) && 
			( !ud || !cp->r_access || ud == cp->r_access ) ) {
			is = i;
			i++;
			n++;
			goto end;
		}
		i++;
	}
	goto exit;
end:
	/*
	 * ���A�K�v�ȃ��R�[�h�̍Ō�ԍ���{��
	 */
	while( i < e ) {
		cp = &td->t_rec[i];
		if( (cp->r_stat & R_DIRTY)
			&& ( !ud || !cp->r_access || ud == cp->r_access )
			&& !(cp->r_stat & R_REC_FREE ) ) {

			i++;
			n++;

			continue;
		} else {
			break;
		}
	}

	/* 
	 * �J�n�ԍ��`�Ō�ԍ��Ԃ̃��R�[�h�Z�O�����g
	 * ���t�@�C���ɏ���������
	 */
	if( TBL_STORE( td, is, n, (char*)td->t_rec[is].r_head ) )
		goto err1;

	/* 
	 * �J�n�ԍ��`�Ō�ԍ��Ԃ̃��R�[�h�̕ҏW���ꂽ
	 * �t���O���N���A����
	 */
	for( k = is; k < is + n ; k++ ) {
DBG_A(("k=%d\n", k ));
		td->t_rec[k].r_stat	&= ~(R_BUSY|R_DIRTY);
	}

	goto loop;
exit:

	/*
	 * update used information in the head of file
	 */
	if ( ud ) {

		td->t_rec_ori_used += ud->u_added;

		if ( TBL_CNTL(td, 0, 0 ) ) {
			td->t_rec_ori_used -= ud->u_added; 
			goto	err1;
		}

		ud->u_added = 0;
		ud->u_insert = 0;
		ud->u_update = 0;
		ud->u_delete = 0;
	}

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}
