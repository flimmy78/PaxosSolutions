/*******************************************************************************
 * 
 *  p_server.c --- xjPing (On Memory DB)  Server programs
 * 
 *  Copyright (C) 2010-2016 triTech Inc. All Rights Reserved.
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
 *	p_server.c
 *
 *      ����	�T�[�o��p�v���O����
 ******************************************************************************/
#include	<stdio.h>
#include	<string.h>
#include	"neo_link.h"

int _p_ServerTCP( p_id_t* ad );

/*****************************************************************************
 * �֐���
 *		p_server()
 * �@�\
 *		�T�[�o�v���g�^�C�v
 * �֐��l
 *		 ���� 	0
 *      	 �ُ�  -1
 * ����
 ******************************************************************************/
int
p_server( namep )
	p_name_t	namep;		/* �T�[�o�p�|�[�g�l�[��	*/
{
	p_id_t 		*ap;
	p_port_t	addr;
	int		err;

DBG_ENT( M_LINK, "p_server");

	/*
	 * �Y���T�[�o�̐ڑ��|�[�g�̃A�h���X���l�[��
	 * �T�[�o�ɖ₢���킹�A�擾����
	 */
	if( p_getaddrbyname( namep, &addr ) )
		goto err1;

	addr.p_role = P_SERVER;
	/*
	 * �ڑ��p�|�[�g�𐶐�����
	 */
	if( !(ap = p_open( &addr ) ) )
		goto err1;
	
	/*
	 * �ʐM�^�C�v�ɂ�蕪�򂵁A�e���̃T�[�o�������s��
	 */
	err = _p_ServerTCP( ap );
	if( err && err != E_SKL_TERM )
		goto	err2;

	/*
	 * �ڑ��|�[�g���N���[�Y����
	 */
	if( p_close( ap ) )
		goto err1;

DBG_EXIT( 0 );
	return( 0 );
err2:
	p_close( ap );
	neo_errno	= err;
err1:
DBG_EXIT( -1 );
	return( -1 );
}

static	fd_set	rfds,
		efds;

static struct {
	p_id_t	*s_pd;
} pds[FD_SETSIZE];


/*****************************************************************************
 * �֐���
 *		_p_ServerTCP()
 * �@�\
 * �֐��l
 *		 ���� 	0
 *      	 �ُ�  -1
 * ����
 ******************************************************************************/
int
_p_ServerTCP( ad )
	p_id_t	*ad;		/* �ڑ��p�|�[�g�L�q�q	*/
{
	int		afd;	/* connection descriptor */
	int		i;
	int		width, n;
	p_id_t		*new;
	p_head_t	head;
	int		headlen;
	char		cntlcode;
	int		err = 0 ;
	int		proc_err = 0 ;

DBG_ENT( M_LINK, "_p_ServerTCP" );

	/*
	 * �|�[�g�񃊃X�g�̏����l�Ƃ��āA�ڑ��|�[�g��o�^����
	 */
	bzero( pds, sizeof(pds) );

	afd		= ad->i_rfd;
	pds[afd].s_pd	= ad;

loop:
	FD_ZERO( &rfds );
	FD_ZERO( &efds );
	for( i = 0; i < FD_SETSIZE; i++ ) {
		if( pds[i].s_pd ) {
			width = i;
			FD_SET( i, &rfds );

			/*
			 * �\�P�b�g�L�q�q�ً̋}��Ԃ��܂��N���A����Ă��Ȃ�
			 * �ƁA��O�`�F�b�N���Ȃ� (tcp�\�P�b�g�L�q�q�ً}���
			 * �͎���̎�M�C�x���g���������Ȃ�����Ɉێ������)
			 */
			if ( !(pds[i].s_pd->i_stat & P_ID_EXCEPT) )
				FD_SET( i, &efds );
		}
	}

	/*
	 * �|�[�g�񃊃X�g�̑S�Ẵ|�[�g�Ŏ�M
	 * �C�x���g�y�ї�O�C�x���g��҂�
	 */
	n = select( width+1, &rfds, 0, &efds, 0 );

	/*
	 * �҂��Ă���Ƃ���ŃV�O�i������������
	 * TERM�ȊO�̃V�O�i����������A��������
	 */
	if ( n < 0 && errno == EINTR )  {

DBG_C( ("_p_ServerTCP: EINTR\n") );
		if ( neo_isterm() ) {
			err = neo_errno;
			goto	err1;
		}

		goto  loop;
	
	} else if ( n < 0 )  {
		err = unixerr2neo();
		goto  err1;
	}

	/*
	 * �ڑ��|�[�g�Ŏ�M�C�x���g������������
	 * p_accept()�ɂĐV�|�[�g�𐶐����A�|�[�g�񃊃X�g�ɓo�^����
	 */
	if( FD_ISSET( afd, &rfds ) ) {	/* connection */

DBG_C( ("_p_ServerTCP: accept\n") );
		if (  ( new = p_accept( ad ) ) != 0 )
		
			pds[new->i_rfd].s_pd	= new;

		FD_CLR( afd, &rfds );
	}

	/*
	 * ���ꂩ�̃|�[�g�ŗ�O�C�x���g������������A
	 * recv(... MSG_OOB)�ɂĔ񓯊��R�[�h���󂯎���āA
	 * �A�{�[�g���������Ă���A�|�[�g���N���[�Y���A
	 * �|�[�g�񃊃X�g����폜����
	 */
	for( i = 0; i <= width; i++ ) {	/* asynchroneous */

		if( FD_ISSET( i, &efds ) ) {
			err = recv( pds[i].s_pd->i_rfd, &cntlcode, 1, MSG_OOB); 

			if ( err != 1 ) {
				err = unixerr2neo();
				goto	err1;
			}

DBG_C( ("_p_ServerTCP: CNTL setting\n") );

			pds[i].s_pd->i_stat |= P_ID_CNTL | P_ID_EXCEPT;
			pds[i].s_pd->i_cntl = cntlcode;

			p_abort( pds[i].s_pd ) ;

			pds[i].s_pd->i_stat &= ~P_ID_CONNECTED;
			p_close( pds[i].s_pd );
			pds[i].s_pd = 0 ;

			FD_CLR( i, &rfds );

		}
	}

	/*
	 * �ڑ��|�[�g�ȊO�̉��ꂩ�̃|�[�g�Ŏ�M�C�x���g������
	 * ������A��M�p�P�b�g�̎�ނɂ���āA���ꂼ��̏������s��
	 */
	for( i = 0; i <= width; i++ ) {	/* receive data */
		if( FD_ISSET( i, &rfds ) ) {

			headlen = sizeof(head);

			/*
			 * ��M�p�P�b�g�̃w�b�_�����s�[�N����
			 */
			headlen = p_peek( pds[i].s_pd, &head, headlen, P_WAIT );

			if ( headlen < 0 ) {
				p_abort( pds[i].s_pd ) ;

				pds[i].s_pd->i_stat &= ~P_ID_CONNECTED;
DBG_C( ("_p_ServerTCP:_p_peek[%d]\n", err ));
				p_close(pds[i].s_pd);
				pds[i].s_pd = 0;
				continue;
			}

			/*
			 * RPC �����N�t�@�~���[�̐ؒf�v����������A
			 * p_close()�ɂă|�[�g���N���[�Y���āA
			 * �|�[�g�񃊃X�g����|�[�g���폜����
			 */
			if( (head.h_mode & P_PKT_REQUEST)
				&& (head.h_family == P_F_LINK ) ) {
				switch( head.h_cmd ) {
					case P_LINK_DIS:

						pds[i].s_pd->i_stat |= P_ID_DISCONNECT;
						p_close( pds[i].s_pd );
DBG_C( ("_p_ServerTCP:P_LINK_DIS p_close()\n"));

						pds[i].s_pd	= 0;
						break;
				}

			/*
			 * RPC �����N�t�@�~���[�ȊO�̗v���p�P�b�g
			 * ��������Ap_proc()�ŗv���������s��
			 */
			} else if( head.h_mode & P_PKT_REQUEST ) {
DBG_C( ("_p_ServerTCP:P_PKT_REQUEST\n"));

				proc_err = p_proc( pds[i].s_pd ) ;

				if ( neo_errno == E_LINK_CNTL
					|| neo_errno == E_UNIX_EPIPE 
					|| neo_errno == E_UNIX_ECONNRESET  ) {

DBG_C( ("_p_ServerTCP:P_PKT_REQUEST[%s]\n", neo_errsym(neo_errno) ));
					p_abort(pds[i].s_pd);
					pds[i].s_pd->i_stat &= ~P_ID_CONNECTED;
					p_close(pds[i].s_pd);
					pds[i].s_pd = 0;

				}

			}
		}
	}

	goto loop;

err1:
DBG_EXIT(err);
	return err;
}

#ifdef	STATICS_OUT

sum_up( pd, ap )
p_id_t	*pd;
p_id_t	*ap;
{
	p_id_t	*p;

	sum_item_up( &pd->i_spacket, &ap->i_spacket);
	sum_item_up( &pd->i_rpacket, &ap->i_rpacket);
//	sum_item_up( &pd->i_sframe, &ap->i_sframe);
//	sum_item_up( &pd->i_rframe, &ap->i_rframe);

	for ( p = (p_id_t *)_dl_head( &_neo_port );
			_dl_valid( &_neo_port, p );
				p = (p_id_t *)_dl_next(p) ) {
		
		if ( p->i_stat & P_ID_CONNECTED )
			goto	ret;
	}

	static_out( ap );
ret:
	return;

}

sum_item_up( b, r )
struct i_static *b;
struct i_static *r;
{
	r->s_number += b->s_number;

	r->s_bytes  += b->s_bytes;

	r->s_time   += b->s_time;

	if ( r->s_max_bytes < b->s_max_bytes )
		r->s_max_bytes = b->s_max_bytes;

	if ( r->s_min_bytes > b->s_min_bytes )
		r->s_min_bytes = b->s_min_bytes;

	if ( r->s_max_time < b->s_max_time )
		r->s_max_time = b->s_max_time;

	if ( r->s_min_time > b->s_min_time )
		r->s_min_time = b->s_min_time;
}
#endif //STATICS_OUT

#ifdef MEMORY

mem_out()
{
	p_id_t	*p;

	for ( p = (p_id_t *)_dl_head( &_neo_port );
			_dl_valid( &_neo_port, p );
				p = (p_id_t *)_dl_next(p) ) {
		
		if ( p->i_stat & P_ID_CONNECTED )
			goto	ret;
	}

	printf("After running Allocated memory (%d)\n", _neo_debug_halloc() );
ret:
	return;

}
#endif
