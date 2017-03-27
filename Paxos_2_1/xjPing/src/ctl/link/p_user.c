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
 *      p_user.c
 *
 *      ����	���[�U I/F �v���O����
 *
 ******************************************************************************/

#include	<string.h>
#include	"neo_link.h"

/***
#pragma weak p_open
#pragma weak p_request
#pragma weak p_close
#pragma weak p_recv
#pragma weak p_reply
#pragma weak p_response
#pragma weak p_write
#pragma weak p_read
#pragma weak p_peek
#pragma weak p_cntl_send
#pragma weak p_cntl_recv
#pragma weak p_exit
***/

extern	int	neo_set_epilog();

int		_link_lock;

static	int	epilog_flg = 0;
static	int	packet_seqno = 0;

/*****************************************************************************
 * �֐���
 *		p_connect()
 * �@�\
 *		�ڑ��A���z��H�̊m��
 * �֐��l
 *     		����	0
 *		�ُ�	-1	
 * ����
 *		p_connect�O��p_open()�Ń|�[�g��
 *		�������Ȃ���΂Ȃ�Ȃ�
 ******************************************************************************/
int
p_connect( pd, raddr )
	p_id_t		*pd;		/* ���|�[�g�L�q�q	*/
	p_port_t	*raddr;		/* ����|�[�g�A�h���X	*/
{
	p_req_con_t	req;
	p_res_con_t	res;
	int		err;
	int		len;
//	int		fd;

DBG_ENT( M_LINK, "p_connect");

	_link_lock++ ;

	/*
	 * �T�[�o�ڑ��|�[�g�A�h���X������ɑ���|�[�g�A�h���X�Ƃ���
	 */
	bcopy( (char *)raddr, (char *)&pd->i_remote, sizeof(p_port_t) );

	ASSERT( pd->i_stat & P_ID_READY );

			if( connect( pd->i_sfd, (struct sockaddr*)&raddr->p_addr.a_pa,
				sizeof(raddr->p_addr.a_pa) ) < 0 ) {

				neo_errno	= unixerr2neo();
				goto err1;
			}
				len = sizeof(pd->i_local.p_addr.a_pa);
				getsockname( pd->i_sfd, 
					(struct sockaddr*)&pd->i_local.p_addr.a_pa, 
					(socklen_t*)&len );
//DBG_PRT("CONNECT\n");
	/*
	 * �ڑ��v���p�P�b�g���쐬����
	 */
	req.c_head.h_tag	= P_HEAD_TAG;
	req.c_head.h_seq	= packet_seqno ++;
	req.c_head.h_id		= getpid();
	req.c_head.h_err	= 0;
	req.c_head.h_mode	= P_PKT_REQUEST;
	req.c_head.h_len	= sizeof(p_req_con_t) - sizeof(p_head_t);
	req.c_head.h_family	= P_F_LINK;
	req.c_head.h_vers	= P_LINK_VER0;
	req.c_head.h_cmd	= P_LINK_CON;

	bcopy( (char *)&pd->i_local, (char *)&req.c_local, sizeof(p_port_t) );
	bcopy( (char *)raddr, (char *)&req.c_remote, sizeof(p_port_t) );

	/*
	 * _p_send()�ɂĐڑ��v���p�P�b�g�𑗐M����
	 */	
//DBG_PRT("LINK\n");
	if( (err = _p_send( pd, (p_head_t*)&req, sizeof(req) )) ) {
		neo_errno = err;
		goto err1;
	}

	/*
	 * _p_recv()�ɂĉ����p�P�b�g��M�҂�
	 */
	len	= sizeof(res);
	if( (err = _p_recv( pd, (char*)&res, &len , P_PKT_REPLY ) ) ) {
//DBG_PRT("LINK REPLY err=%d\n", err );
		neo_errno = err;
		goto err1;
	}
//DBG_PRT("LINK REPLY\n");

	if( res.c_head.h_err ) {
		neo_errno = res.c_head.h_err;
		goto err1;
	}

	/*
	 * �|�[�g��ڑ����|�[�g�Ƃ��āA�|�[�g�L�q�q
	 * �̑���|�[�g�A�h���X���X�V����
	 */
	pd->i_stat	|= P_ID_CONNECTED;
	pd->i_local.p_rtime = pd->i_local.p_stime = time( 0 );
	bcopy( &res.c_remote, &pd->i_remote, sizeof(p_addr_t) );

	/*
	 * �\�P�b�g�̑���M�o�b�t�@�T�C�Y�̐ݒ�
	 * ���M�o�b�t�@�T�C�Y�̓��[�J���̃o�b�t�@�T�C�Y�萔�ɂ�
	 * ��M�o�b�t�@�T�C�Y�͑���̃o�b�t�@�T�C�Y�萔�ɂ��
	 */
	len = pd->i_local.p_bufsize;
	if ( setsockopt(pd->i_sfd, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len)) ) {
		neo_errno = unixerr2neo();
		goto err1;
	}

	len = pd->i_remote.p_bufsize;
	if ( setsockopt(pd->i_rfd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len)) ) {
		neo_errno = unixerr2neo();
		goto err1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_disconnect()
 * �@�\
 *		���z��H�̐ؒf
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 ******************************************************************************/
int
p_disconnect( pd )
	p_id_t	*pd;		/* �ؒf���悤�Ƃ���|�[�g�L�q�q */
{
	p_req_dis_t	req;
	p_res_dis_t	res;
	int		err;
	int		len;

DBG_ENT( M_LINK, "p_disconnect");

	_link_lock++;

	/*
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED) ) {
		neo_errno = E_LINK_DOWN;
		goto err1;
	}

	/*
	 * �ڑ��v���p�P�b�g���쐬����
	 */
	req.d_head.h_tag	= P_HEAD_TAG;
	req.d_head.h_seq	= packet_seqno ++;
	req.d_head.h_id		= getpid();
	req.d_head.h_err	= 0;
	req.d_head.h_mode	= P_PKT_REQUEST;
	req.d_head.h_len	= sizeof(p_port_t) * 2 ;
	req.d_head.h_family	= P_F_LINK;
	req.d_head.h_vers	= P_LINK_VER0;
	req.d_head.h_cmd	= P_LINK_DIS;

	bcopy(&pd->i_local, &req.d_client, sizeof(p_port_t) );
	bcopy(&pd->i_remote, &req.d_remote, sizeof(p_port_t) );

	/*
	 * _p_send()�ɂĐڑ��v���p�P�b�g�𑗐M����
	 */
//DBG_PRT("UNLINK SEND\n");
	if( (err = _p_send( pd, (p_head_t*)&req, sizeof(req) )) ) {
		neo_errno	= err;
		goto err1;
	}

	/*
	 * _p_recv()�ɂăT�[�o����̉�������M�҂�
	 */
	len	= sizeof(res);
//DBG_PRT("UNLINK REPLY len=%d\n", len );
	if( (err = _p_recv( pd, (char*)&res, &len, P_PKT_REPLY )) ) {
		neo_errno	= err;
//DBG_PRT("UNLINK REPLY err=%d\n", err);
		goto err1;
	}
//DBG_PRT("UNLINK REPLY err=%d\n", err);

	if( res.d_head.h_err ) {
		neo_errno	= err;
		goto err1;
	}

				if ( pd->i_stat & P_ID_READY ) {
					close(pd->i_sfd);
					pd->i_stat &= ~P_ID_READY ;
				}

	/*
	 * �|�[�g��Ԑڑ����r�b�g���N���A����
	 */
	pd->i_stat	&= ~P_ID_CONNECTED;

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_open()
 * �@�\
 *		�|�[�g�L�q�q�I�[�v��
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 *		p_open�O��p_getaddrbyname()�Ń|�[�g
 *		���擾����͈̂�ʂł���
 ******************************************************************************/
p_id_t *
p_open( laddr )
	p_port_t	*laddr;		/* �|�[�g�A�h���X	*/
{
	p_id_t	*pd;
	int	fd;
	int	err;

DBG_ENT( M_LINK, "p_open");

	_link_lock++;

	/*
	 * �|�[�g�L�q�q�p�G���A����
	 */
	pd = (p_id_t *)_p_zalloc( sizeof(p_id_t) );
	if( !pd ) {
		err = E_LINK_NOMEM;
		goto err1;
	}

	/*
	 * �|�[�g�L�q�q�������ƃ��[�J�����ݒ�
	 */
	bcopy( laddr, &pd->i_local, sizeof(p_port_t) );

	pd->i_local.p_addr.a_pd	= pd;
	pd->i_local.p_cur_time	= pd->i_local.p_min_time;

	_dl_init(&pd->i_lnk);

	pd->i_spacket.s_min_bytes = pd->i_spacket.s_min_time = SOCK_BUF_MAX;
	pd->i_rpacket.s_min_bytes = pd->i_rpacket.s_min_time = SOCK_BUF_MAX;

	/* 
	 * �ʐM�^�C�v�ɂ�蕪�򂷂�
	 */
			fd = socket( AF_INET, SOCK_STREAM, 0 );
			if( fd < 0 ) {
				neo_errno = unixerr2neo();
				goto err2;
			}
	
	switch( pd->i_local.p_role ) {

		case P_CLIENT: 
			break;

		case P_SERVER:
			{
				int on = 1;
				if ( setsockopt( fd,SOL_SOCKET,SO_REUSEADDR,
					(char*)&on,sizeof(on) ) < 0 ){
					neo_errno = unixerr2neo();
					goto err2;
				}
			}

			if(bind(fd, (struct sockaddr *)&pd->i_local.p_addr.a_pa,
					sizeof(pd->i_local.p_addr.a_pa))< 0) {
				neo_errno = unixerr2neo();
				goto err2;
			}
/* XXX */
			/*
			 * �T�[�o���o�b�N���O�ݒ肳��Ă���
		 	 * �ꍇ�ɂ́Alisten()�𔭍s����
			 */
			if( pd->i_local.p_listen ) {
				if( listen( fd, pd->i_local.p_listen ) ) {
					neo_errno = unixerr2neo();
					goto err3;
				}
			}

			break;

		default:  
			neo_errno = E_LINK_INVAL;
			goto	err3;

	}

	/*
	 * READY ��Ԃ̗L���|�[�g�Ƃ��āA 
	 * _neo_port�փ����N����
	 */
	pd->i_sfd = pd->i_rfd = fd;
	pd->i_stat	= P_ID_READY;

	_dl_insert( pd, &_neo_port );
	pd->i_id	= P_PORT_EFFECTIVE;

#ifdef TEST
display();
#endif
	/*
	 * �G�s���[�O�������o�^����Ă��Ȃ��ꍇ�ɂ́A�o�^����
	 */
	if ( !epilog_flg ) {
		neo_set_epilog( M_LINK, p_exit );
		epilog_flg = 1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( pd );

err3:	close( fd );
err2: 	_p_free( pd );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( 0 ) ;
}


/*****************************************************************************
 * �֐���
 *		p_close()
 * �@�\
 *		�|�[�g�L�q�q�N���[�Y
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 ******************************************************************************/
int
p_close( pd )
	p_id_t	*pd;		/* �N���[�Y���悤�Ƃ���|�[�g�L�q�q*/
{
	p_req_dis_t	req;
	p_res_dis_t	res;
	int		err;
	p_id_t		*p;
	int		fd;
	int		siz = sizeof(req);

DBG_ENT( M_LINK, "p_close");

	_link_lock++;

	/*
	 * �|�[�g�̗L�����`�F�b�N�i��d�N���[�Y��h���j
	 */
	if ( pd->i_id != P_PORT_EFFECTIVE ) {
		neo_errno = E_LINK_PORT ;
		goto	err1;
	}

	/* 
	 * ����ؒf�v���ɂ��N���[�Y
	 */
	if( pd->i_stat & P_ID_DISCONNECT ) {

	discard:

		/*
		 * �ؒf�v���p�P�b�g��_p_recv()�Ŏ󂯎��
		 */
		if( (err = _p_recv( pd, (char*)&req, &siz, P_PKT_REQUEST )) ) {
			neo_errno	= err;
			goto err1;
		}

		if( req.d_head.h_family == P_F_LINK &&
			req.d_head.h_cmd == P_LINK_DIS ) {

			/*
			 * �����p�P�b�g���쐬���āA_p_send()�ɂ�
			 * ����ɑ��M����
			 */
			bcopy( &req.d_head, &res.d_head, sizeof(p_head_t) );
			res.d_head.h_mode = P_PKT_REPLY;
			res.d_head.h_len  =  0;

			if( (err = _p_send( pd, (p_head_t*)&res, sizeof(res) )) ) {
				neo_errno	= err;
				goto err1;
			}

			pd->i_stat	&= ~(P_ID_CONNECTED|P_ID_DISCONNECT);

		} else {
			goto discard;
		}
	}

	/* 
	 * �|�[�g��ڑ���ԃ`�F�b�N
	 */
	if( pd->i_stat & P_ID_CONNECTED ) {
		neo_errno = E_LINK_BUSY;
		goto err1;
	}

	/* 
	 * �|�[�g�L���t���O�N���A�A_neo_port����A�������N����
	 */
	pd->i_id = 0;
	_dl_remove( pd );

#ifdef TEST
display();
#endif

	if( !(pd->i_stat & P_ID_READY ) )
		goto  next1;

	/*
	 * �|�[�g�̃\�P�b�g�t�@�C���L�q�q�����̃|�[�g�ɋ��L
	 * ����Ă���ƁA�N���[�Y���Ȃ��A�����łȂ��ꍇ�́A
	 * �\�P�b�g�L�q�q���N���[�Y����
	 */
	fd = pd->i_sfd;
	for( p = (p_id_t *)_dl_head(&_neo_port); _dl_valid(&_neo_port,p); 
						p = (p_id_t *)_dl_next(p) ) {
		if( fd == p->i_sfd )	goto next;
		if( fd == p->i_rfd )	goto next;
	}
	if( fd != pd->i_rfd )
		close( fd );
next:
	fd = pd->i_rfd;
	for( p = (p_id_t *)_dl_head(&_neo_port); _dl_valid(&_neo_port,p); 
						p = (p_id_t *)_dl_next(p) ) {
		if( fd == p->i_sfd )	goto next1;
		if( fd == p->i_rfd )	goto next1;
	}

	close( fd );
next1:
	_p_free( pd );

	_link_lock--;
DBG_EXIT( 0 );
	return( 0 );
err1:

	_link_lock--;
DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_accept()
 * �@�\
 *		�ڑ��v���ɑ΂����Έ�̉��z��H���m������
 * �֐��l
 *     		����	�V�|�[�g�L�q�q�̃A�h���X
 *		�ُ�	0
 * ����
 *		�T�[�o���p�֐�
 ******************************************************************************/
p_id_t	*
p_accept( ap )
	p_id_t	*ap;		/* �ڑ��p�|�[�g�L�q�q	*/
{
	struct sockaddr_in peer;
	int		len = sizeof(peer);
	p_req_con_t	req;
	p_res_con_t	res;
	p_id_t		*np;
	int		fd;
	int		err;

DBG_ENT( M_LINK, "p_accept");

//DBG_PRT("ACCEPT\n");
	_link_lock++;

	/*
	 * �V�|�[�g�p�G���A��p�ӂ��āA����������
	 */
	if ( !(np = (p_id_t *)_p_zalloc( sizeof(p_id_t) ) ) ) {
		neo_errno = E_LINK_NOMEM;
		goto 	err1;
	}

	bcopy( ap, np, sizeof(p_id_t) );

	np->i_local.p_addr.a_pd	= np;
	np->i_local.p_addr.a_id	= -1;

	_dl_init( &np->i_lnk );

	np->i_err = 0 ;
	np->i_cntl = 0;

	bzero(&np->i_spacket, sizeof(np->i_spacket) );
	bzero(&np->i_rpacket, sizeof(np->i_rpacket) );

	np->i_spacket.s_min_bytes = np->i_spacket.s_min_time = SOCK_BUF_MAX;
	np->i_rpacket.s_min_bytes = np->i_rpacket.s_min_time = SOCK_BUF_MAX;

re_accept:
			fd = accept( ap->i_rfd, (struct sockaddr *)&peer,
										(socklen_t*)&len );
			if( fd < 0 ) {

				/*
				 * accept�� TERM �ȊO�̃V�O�i���𖳎�����
				 */
				if ( (errno == EINTR) && (!neo_isterm() ) )
					goto re_accept;
				neo_errno	= unixerr2neo();
				goto err1;
			}
			np->i_sfd = np->i_rfd = fd;
			ap = np ;

	/*
	 * �ڑ��v����_p_recv()�Ŏ�M����
	 */
//DBG_PRT("LINK\n");
	len = sizeof(p_req_con_t);
	if( (err = _p_recv( ap, (char*)&req, &len, P_PKT_REQUEST )) ) {

		neo_errno	= err;
		goto err1;
	}

	/*
	 * ����|�[�g����V�|�[�g�ɐݒ肷��
	 */
	bcopy( &req.c_local, &np->i_remote, sizeof(p_port_t) );

	/*
	 * �����p�P�b�g���쐬����
	 */
	bcopy( &req, &res, sizeof(p_head_t) );

	res.c_head.h_mode	= P_PKT_REPLY;
	res.c_head.h_len	= sizeof(res) - sizeof(p_head_t);

	bcopy( &np->i_local, &res.c_remote, sizeof(p_port_t) );

//DBG_PRT("LINK REPLY\n");
	/*
	 * _p_send()�ɂĉ����p�P�b�g�𑗐M����
	 */
	if( (err = _p_send( np, (p_head_t*)&res, sizeof(res) )) ) {
		neo_errno	= err;
		goto err1;
	}

	/*
	 * �V�|�[�g��_neo_port�Ƀ����N���A�|�[�g�L���t���O�ݒ肷��
	 */
	_dl_insert( np, &_neo_port );

	np->i_id    = P_PORT_EFFECTIVE;

	/* 
	 * �|�[�g��ڑ����|�[�g�Ƃ���
	 */
	np->i_stat |= (P_ID_CONNECTED | P_ID_READY ) ;
	np->i_local.p_rtime = np->i_local.p_stime = time( 0 );

#ifdef TEST
display();
#endif

		len = np->i_local.p_bufsize;
		if ( setsockopt(np->i_sfd, SOL_SOCKET, SO_SNDBUF, 
						&len, sizeof(len)) ) {
			neo_errno = unixerr2neo();
			goto err1;
		}

		len = np->i_remote.p_bufsize;
		if ( setsockopt(np->i_rfd, SOL_SOCKET, SO_RCVBUF, 
						&len, sizeof(len)) ) {
			neo_errno = unixerr2neo();
			goto err1;
		}

	_link_lock--;

DBG_EXIT( 0 );
	return( np );
err1:
	_p_free( np );
	_link_lock--;

DBG_EXIT( -1 );
	return( 0 );
}

/*****************************************************************************
 * �֐���
 *		p_request()
 * �@�\
 *		�T�[�r�X�v���𑗐M����
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 ******************************************************************************/
int
p_request( pd, reqp, len )
	p_id_t		*pd;		/* ���M�|�[�g�L�q�q	*/
	p_head_t	*reqp;		/* ���M�f�[�^		*/
	int		len;		/* ���M�f�[�^�T�C�Y	*/	
{
	int	err;

DBG_ENT( M_LINK, "p_request");

	_link_lock++;

	/*
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !( pd->i_stat & P_ID_CONNECTED ) || 
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno = E_LINK_DOWN;
		goto err1;
	}

	/*
	 * RPC �v���g�R���t�@�~���[�`�F�b�N
	 */
	if( reqp->h_family & P_F_MASK ) {
		neo_errno = E_LINK_FAMILY;
		goto err1;
	}

	/*
	 * ���M�p�P�b�g�T�C�Y�`�F�b�N
	 */
	if( reqp->h_len != ( len - sizeof(p_head_t) ) ) {
DBG_B(("send data size len[%d]reqp->h_len[%d]sizeof(p_head_t)[%d]\n",
	len, reqp->h_len, sizeof(p_head_t) ));
		neo_errno = E_LINK_PACKET;
		goto err1;
	}

	/*
	 * �p�P�b�g�w�b�_���t���O�Ɨv�����[�h�ݒ�
	 */
	reqp->h_tag	= P_HEAD_TAG;
	reqp->h_seq	= packet_seqno ++;
	reqp->h_id	= getpid();
	reqp->h_mode	= P_PKT_REQUEST;

	/* 
	 * _p_send()�ɂėv���p�P�b�g�𑗐M����
	 */
	if( (err = _p_send( pd, reqp, len )) ) {
		neo_errno	= err;
		goto err1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_recv()
 * �@�\
 *		�T�[�r�X�v������M����
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 *		lenp�����̓A�h���X�ł���A���͈����Ƃ��āA�v���f�[�^�T�C�Y��
 *		�ݒ肳��A�o�͈����Ƃ��āA���ێ�M�����f�[�^�T�C�Y���ݒ肳���
 ******************************************************************************/
Msg_t*
p_recvByMsg( p_id_t *pd )
{
	Msg_t		*pMsg;
	MsgVec_t	Vec;
	p_head_t	head;
	char		*pBuf;
	int			s, l;

	if( p_peek( pd, &head, sizeof(head), P_WAIT ) < 0 )	return( NULL );

	pBuf = neo_malloc( (s = head.h_len + sizeof(p_head_t)) );
	if( !pBuf ) {
		neo_errno = E_RDB_NOMEM;
		return( NULL );
	}
	l	= s;
	if( p_recv( pd, (p_head_t*)pBuf, &l ) ) return( NULL );	
	pMsg	= MsgCreate( 1, neo_malloc, neo_free );
	MsgVecSet( &Vec, VEC_MINE, pBuf, s, l );
	MsgAdd( pMsg, &Vec );

	return( pMsg );
}

int
p_recv( pd, reqp, lenp )
	p_id_t		*pd;		/* ��M�|�[�g�L�q�q	*/
	p_head_t	*reqp;		/* ��M�o�b�t�@		*/
	int		*lenp;		/* ��M�o�b�t�@�T�C�Y	*/
{
	int	err;

DBG_ENT( M_LINK, "p_recv");
	
	_link_lock++;

	/* 
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED ) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno = E_LINK_DOWN;
		goto err1;
	}

	/*
	 * _p_recv()�ɂėv���p�P�b�g����M����
	 */
	if( (err = _p_recv( pd, (char*)reqp, lenp, P_PKT_REQUEST )) ) {
		neo_errno	= err;
		goto err1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_reply()
 * �@�\
 *		�T�[�r�X���ʂ𑗐M����
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 ******************************************************************************/
int
p_reply_free( pd, resp, len )
	p_id_t		*pd;		/* ���M�|�[�g�L�q�q	*/
	p_head_t	*resp;		/* ���M�f�[�^		*/
	int		len;		/* ���M�f�[�^�T�C�Y	*/
{
	int	Ret;

	Ret = p_reply( pd, resp, len );

	neo_free( resp );
	return( Ret );
}

int
p_reply( pd, resp, len )
	p_id_t		*pd;		/* ���M�|�[�g�L�q�q	*/
	p_head_t	*resp;		/* ���M�f�[�^		*/
	int		len;		/* ���M�f�[�^�T�C�Y	*/
{
	int	err;

DBG_ENT( M_LINK, "p_reply");

	_link_lock++;

	/*
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno	= E_LINK_DOWN;
		goto err1;
	}

	/* 
	 * RPC �v���g�R���t�@�~���[�`�F�b�N
	 */
	if( resp->h_family & P_F_MASK ) {
		neo_errno	= E_LINK_FAMILY;
		goto err1;
	}

	/*
	 * ���M�f�[�^�T�C�Y�`�F�b�N
	 */
	if( resp->h_len != (len - sizeof(p_head_t) ) ) {
DBG_B(("send data size len[%d]resp->h_len[%d]sizeof(p_head_t)[%d]\n",
	len, resp->h_len, sizeof(p_head_t) ));
		neo_errno	= E_LINK_PACKET;
		goto err1;
	}

	/*
	 * �w�b�_���t���O���������[�h�ݒ�
	 */
	resp->h_tag	= P_HEAD_TAG;
	resp->h_mode	= P_PKT_REPLY;

	/* 
	 * _p_send()�ɂĉ����𑗐M����
	 */
	if( (err = _p_send( pd, (p_head_t*)resp, len )) ) {
		neo_errno	= err;
		goto err1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:

	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_response()
 * �@�\
 *		�T�[�r�X���ʂ���M����
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 *		lenp�����̓A�h���X�ł���A���͈����Ƃ��āA�v���f�[�^�T�C�Y��
 *		�ݒ肳��A�o�͈����Ƃ��āA���ێ�M�����f�[�^�T�C�Y���ݒ肳���
 ******************************************************************************/
Msg_t*
p_responseByMsg( p_id_t *pd )
{
	Msg_t		*pMsg;
	MsgVec_t	Vec;
	p_head_t	head;
	char		*pBuf;
	int			s, l;

	if( p_peek( pd, &head, sizeof(head), P_WAIT ) < 0 )	return( NULL );

	pBuf = neo_malloc( (s = head.h_len + sizeof(p_head_t)) );
	if( !pBuf ) {
		neo_errno = E_RDB_NOMEM;
		return( NULL );
	}
	l	= s;
	if( p_response( pd, (p_head_t*)pBuf, &l ) ) return( NULL );	
	pMsg	= MsgCreate( 1, neo_malloc, neo_free );
	MsgVecSet( &Vec, VEC_MINE, pBuf, s, l );
	MsgAdd( pMsg, &Vec );

	return( pMsg );
}

int
p_response( pd, resp, lenp )
	p_id_t		*pd;		/* ��M�|�[�g�L�q�q	*/
	p_head_t	*resp;		/* ��M�o�b�t�@		*/
	int		*lenp;		/* ��M�o�b�t�@�T�C�Y	*/
{
	int	err;

DBG_ENT( M_LINK, "p_response");

	_link_lock++;

	/*
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED ) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno	= E_LINK_DOWN;
		goto err1;
	}

	/*
	 * _p_recv()�ɂĉ����p�P�b�g����M����
	 */
	if( (err = _p_recv( pd, (char*)resp, lenp, P_PKT_REPLY )) ) {
		neo_errno	= err;
		goto err1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_write()
 * �@�\
 *		�f�[�^���M
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 ******************************************************************************/
int
p_write( pd, datap, len )
	p_id_t		*pd;		/* ���M�|�[�g�L�q�q	*/
	p_head_t	*datap;		/* ���M����f�[�^	*/
	int		len;		/* ���M����f�[�^�T�C�Y	*/
{
	int	err;

DBG_ENT( M_LINK, "p_write");

	_link_lock++;

	/*
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED ) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno	= E_LINK_DOWN;
		goto err1;
	}
	/* 
	 * ���M�f�[�^�T�C�Y�`�F�b�N
	 */
	if( datap->h_len != ( len - sizeof(p_head_t) ) ) {
DBG_B(("send data size len[%d]datap->h_len[%d]sizeof(p_head_t)[%d]\n",
	len, datap->h_len, sizeof(p_head_t) ));
		neo_errno	= E_LINK_PACKET;
		goto err1;
	}

	/*
	 * �w�b�_���t���O�ƃ��[�h�ݒ�
	 */
	datap->h_tag	= P_HEAD_TAG;
	datap->h_seq	= packet_seqno ++;
	datap->h_id	= getpid();
	datap->h_mode	= P_PKT_DATA;

	/*
	 * _p_send()�ɂđ��M����
	 */
	if( (err = _p_send( pd, datap, len )) ) {
		neo_errno	= err;
		goto err1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_read()
 * �@�\
 *		�f�[�^��M
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 *		lenp�����̓A�h���X�ł���A���͈����Ƃ��āA�v���f�[�^�T�C�Y��
 *		�ݒ肳��A�o�͈����Ƃ��āA���ێ�M�����f�[�^�T�C�Y���ݒ肳���
 ******************************************************************************/
int
p_read( pd, datap, lenp )
	p_id_t		*pd;		/* ��M�|�[�g�L�q�q	*/
	p_head_t	*datap;		/* ��M�o�b�t�@		*/
	int		*lenp;		/* ��M�o�b�t�@�T�C�Y	*/
{
	int	err;

DBG_ENT( M_LINK, "p_read");

	_link_lock++;

	/* 
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED ) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno	= E_LINK_DOWN;
		goto err1;
	}

	/*
	 * _p_recv()��p�����āAP_PKT_DATA��ނ̃p�P�b�g����M����
	 */	
	if( (err = _p_recv( pd, (char*)datap, lenp , P_PKT_DATA)) ) {
		neo_errno	= err;
		goto err1;
	}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_peek()
 * �@�\
 *		�f�[�^�s�[�N
 * �֐��l
 *     		����	�f�[�^�T�C�Y
 *		�ُ�	-1
 * ����
 *		NOWAIT���[�h�Ńf�[�^���Ȃ��ꍇ�ɂ́A�G���[�ƂȂ�
 ******************************************************************************/
int
p_peek( pd, datap, len, mode )
	p_id_t		*pd;		/* ��M�|�[�g�L�q�q	*/
	p_head_t	*datap;		/* ��M�o�b�t�@		*/
	int		len;		/* ��M�o�b�t�@�T�C�Y	*/
	int		mode;		/* �s�[�N����p�P�b�g���[�h*/
{
	int	err;

DBG_ENT( M_LINK, "p_peek");

	_link_lock++;

	/* 
	 * �|�[�g READY ��ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_READY ) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno	= E_LINK_DOWN;
		goto err1;
	}

	/*
	 * _p_peek()��p�����āA�w�肵�����[�h�Ńs�[�N����
	 */
	if( (err = _p_peek(pd, (char*)datap, &len, mode )) ) {
		neo_errno	= err;
		goto err1;
	}

	_link_lock--;

DBG_EXIT( len );

	return( len );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_cntl_send()
 * �@�\
 *		�񓯊��R�[�h���M
 * �֐��l
 *     		����	0
 *		�ُ�	-1
 * ����
 ******************************************************************************/
int
p_cntl_send( pd, code )
	p_id_t	*pd;		/* ���M�|�[�g�L�q�q	*/
	char	code;		/* ���M����񓯊��R�[�h	*/
{
DBG_ENT( M_LINK, "p_cntl_send" );

	_link_lock++;

	/* 
	 * �|�[�g�ڑ���ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED ) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno	= E_LINK_DOWN;
		goto err1;
	}

			if( send( pd->i_sfd, &code, 1, MSG_OOB ) < 0 ) {
				neo_errno	= unixerr2neo();
				goto err1;
			}

	_link_lock--;

DBG_EXIT( 0 );
	return( 0 );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}

/*****************************************************************************
 * �֐���
 *		p_cntl_recv()
 * �@�\
 *		�񓯊��R�[�h�擾
 * �֐��l
 *     		����	�񓯊��R�[�h
 *		�ُ�	-1
 * ����
 ******************************************************************************/
int
p_cntl_recv( pd )
	p_id_t	*pd;		/* �|�[�g�L�q�q		*/
{

DBG_ENT( M_LINK, "p_cntl_recv" );

	_link_lock++;

	/* 
	 * �|�[�g��ԃ`�F�b�N
	 */
	if( !(pd->i_stat & P_ID_CONNECTED ) ||
			( pd->i_id != P_PORT_EFFECTIVE ) ) {
		neo_errno	= E_LINK_DOWN;
		goto err1;
	}

	if( !(pd->i_stat & P_ID_CNTL ) ) {
		neo_errno	= E_LINK_INVAL;
		goto err1;
	}

	pd->i_stat &= ~P_ID_CNTL;

	_link_lock--;

DBG_EXIT( pd->i_cntl );
	return( pd->i_cntl );
err1:
	_link_lock--;

DBG_EXIT( -1 );
	return( -1 );
}


/*****************************************************************************
 * �֐���
 *		p_set_time()
 * �@�\
 *		��M�҂��^�C���A�E�g�ύX
 * �֐��l
 *     		�ύX�O�̃^�C���A�E�g�l
 * ����
 *		timeout�̓~���b�P�ʂł��邱��
 ******************************************************************************/
int
p_set_time( pd, timeout )
	p_id_t	*pd;		/* �|�[�g�L�q�q			*/
	long	timeout;	/* �^�C���A�E�g�l(�~���b)	*/
{
	int	ret;

	if ( timeout < 0 ) {
		neo_errno = E_LINK_INVAL;
		ret = -1 ;
	} else {
		ret = pd->i_local.p_rcv_time;
		pd->i_local.p_rcv_time = timeout ;
	}

	return ret;
}

/*****************************************************************************
 * �֐���
 *		p_exit()
 * �@�\
 *		�I������
 * �֐��l
 *     		����
 * ����
 ******************************************************************************/
int
p_exit()
{
	p_id_t	*pd;
	p_id_t	*wp;

DBG_ENT( M_LINK, "p_exit" ) ;

	/*
	 * _neo_port�Ɍq�����Ă���S�Ẵ|�[�g�ɑ΂��鏈������
	 */
	list_for_each_entry_safe( pd, wp, &_neo_port, i_lnk ) {

		/*
		 * �I���񓯊��R�[�h��������Ap_close()�Ń|�[�g���N���[�Y����
		 */
		if ( pd->i_stat & P_ID_CNTL &&
			((p_cntl_recv( pd )&0xff)  == P_CNTL_ABORT) ) {

				pd->i_stat &= ~P_ID_CONNECTED;
				p_close( pd );
		/*
		 * �ڑ����|�[�g��������Ap_cntl_send()�ŏI���񓯊�
		 * �R�[�h�𑗐M���āA�|�[�g���N���[�Y����
		 */
		}  else {

		 	if ( pd->i_stat & P_ID_CONNECTED )  {

				p_cntl_send( pd, P_CNTL_ABORT );
			}

			pd->i_stat &= ~P_ID_CONNECTED;
			p_close( pd );

		}
	}

DBG_EXIT( 0 );
	return 0 ;
}


#ifdef TEST
display() 
{
	static clear = 0 ;
	p_id_t	*pd;
	int	amount = 0 ;

	if ( !clear ) {
		system("clear");
		clear ++;
	}

	for ( pd = (p_id_t *)_dl_head(&_neo_port);
				_dl_valid(&_neo_port, pd);
					pd = (p_id_t *)_dl_next(pd) ) {

		amount ++ ;
/***
		printf("pd ADD == %x   pd STAT == %x\n", pd, pd->i_stat) ;
***/
	}

	printf("---------------------------------\n");

/******
	if ( amount % 100 == 0 ) {
****/
		printf(" Total Port Number == %d\n", amount);
/*****
		printf("---------------------------------\n");
	}
******/

}
#endif

#ifdef	STATICS_OUT
static_out( pd )
p_id_t	*pd;
{

DBG_ENT( M_LINK, "static_out");

	DBG_A( (" PORT STATIC DATA IS AS FOLLOWS\n") ) ;
	DBG_A( (" PACKET SENDED ::\n") );
	DBG_A( ("         Total Number 	=%d\n", pd->i_spacket.s_number) );
	DBG_A( ("         Total Bytes 	=%d\n", pd->i_spacket.s_bytes) );
	DBG_A( ("         Max  length 	=%d\n", pd->i_spacket.s_max_bytes) );
	DBG_A( ("         Min  length 	=%d\n", pd->i_spacket.s_min_bytes) );
	DBG_A( ("         Total Time   	=%d\n", pd->i_spacket.s_time) );
	DBG_A( ("         Max   Time 	=%d\n", pd->i_spacket.s_max_time) );
	DBG_A( ("         Min   Time 	=%d\n", pd->i_spacket.s_min_time) );
	DBG_A( (" PACKET RECEIVED ::\n") );
	DBG_A( ("         Total Number 	=%d\n", pd->i_rpacket.s_number) );
	DBG_A( ("         Total Bytes 	=%d\n", pd->i_rpacket.s_bytes) );
	DBG_A( ("         Max  length 	=%d\n", pd->i_rpacket.s_max_bytes) );
	DBG_A( ("         Min  length 	=%d\n", pd->i_rpacket.s_min_bytes) );
	DBG_A( ("         Total Time   	=%d\n", pd->i_rpacket.s_time) );
	DBG_A( ("         Max   Time 	=%d\n", pd->i_rpacket.s_max_time) );
	DBG_A( ("         Min   Time 	=%d\n", pd->i_rpacket.s_min_time) );

DBG_EXIT(0);

	return;
}
#endif //STATICS_OUT
