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
 *      p_raw.c
 *
 *      ����    RAW I/F �v���O����
 *
 ******************************************************************************/

#include	<string.h>
#include	"neo_link.h"
#include	<limits.h>
#include	<sys/uio.h>


/*****************************************************************************
 * �֐���
 *		_p_send()
 * �@�\
 *		�p�P�b�g���M�y��ACK��M
 * �֐��l
 *		0	����
 *		0�ȊO   �G���[�ԍ�
 * ����
 ****************************************************************************/
int
_p_send( pd, packet, len )
	p_id_t		*pd;	 	/* �|�[�g�L�q�q		*/
	p_head_t	*packet;	/* ���M�����p�P�b�g	*/
	int		len;		/* �p�P�b�g�T�C�Y	*/
{
	int		err;

#ifdef	STATICS
	long		begin;
	long		time_cost;
	int		length;
#endif

DBG_ENT( M_LINK, "_p_send" );
//DBG_PRT( "_p_send\n" );

	/*
	 * �|�[�g��ԃ`�F�b�N
	 */
	if( pd->i_stat & P_ID_DOWN ) {
		err = E_LINK_DOWN;
		goto err1;
	}
	if( pd->i_stat & P_ID_CNTL ) {
		err = E_LINK_CNTL;
		goto err1;
	}

	/*
	 * �p�P�b�g���M
	 */
	if( (err = SendStream( pd->i_rfd, (char*)packet, len ) ) ) {
		goto err1;
	}

#ifdef	STATICS
	begin	= time(0);
#endif

#ifdef STATICS

	time_cost	= time(0) - begin ;
	length		= sizeof(head);
	pd->i_rpacket.s_number ++;
	pd->i_rpacket.s_bytes += length;

	if (length > pd->i_rpacket.s_max_bytes )
		pd->i_rpacket.s_max_bytes = length;

	if (length < pd->i_rpacket.s_min_bytes )
		pd->i_rpacket.s_min_bytes = length;

	pd->i_rpacket.s_time += time_cost;

	if ( time_cost > pd->i_rpacket.s_max_time )
		pd->i_rpacket.s_max_time = time_cost ;

	if ( time_cost < pd->i_rpacket.s_min_time )
		pd->i_rpacket.s_min_time = time_cost ;

#endif // STATICS

err1:

DBG_EXIT( err );
	return( err );
}


/*****************************************************************************
 * �֐���
 *		_p_recv()
 * �@�\
 *		�p�P�b�g��M�y��ACK���M
 * �֐��l
 *		0	����
 *		0�ȊO   �G���[�ԍ�
 * ����
 *		�ڑ��v���t���[���̑��d��M�͂����Ń`�F�b�N����
 ****************************************************************************/
int
_p_recv( pd, packet, lenp, mode )
	p_id_t		*pd;		/* �|�[�g�L�q�q		*/
	char		*packet;	/* �o�b�t�@		*/
	int		*lenp;		/* �o�b�t�@�T�C�Y	*/
	int		mode;		/* ��M�p�P�b�g���[�h	*/
{
	int		err ,error = 0 ;

#ifdef STATICS
	long		begin;
	long		time_cost;
	int		length;
#endif // STATICS

DBG_ENT( M_LINK, "_p_recv") ;
//DBG_PRT( "_p_recv\n" );

#ifdef STATICS
	begin = time(0);
#endif // STATICS
	/*
	 * �|�[�g��ԃ`�F�b�N
	 */
	if( pd->i_stat & P_ID_DOWN ) {
		err = E_LINK_DOWN;
		goto err1;
	}

	/*
	 * �p�P�b�g��M�҂�
	 */
	p_head_t	head;

	if( pd->i_stat & P_ID_CNTL ) {
		error = E_LINK_CNTL;
		goto err1;
	}
	err = PeekStream( pd->i_rfd, (char*)&head, sizeof(head) );
//DBG_PRT("PEEK err=%d\n", err );
	if( err < 0 ) {
		err = errno;
		goto err1;
	}
	if( *lenp < sizeof(head) + head.h_len ) {
		err = E_LINK_BUFSIZE ;
//DBG_PRT("BUFF *lenp=%d h_len=%d\n", *lenp, head.h_len );
		goto err1;
	}
	err = RecvStream( pd->i_rfd, packet, sizeof(head) + head.h_len );
//DBG_PRT("RECV err=%d\n", err );
	if( err < 0 ) {
		err = errno;
		goto err1;
	}

#ifdef STATICS

	time_cost	= time(0) - begin ;

	pd->i_rpacket.s_number ++;
	pd->i_rpacket.s_bytes += length;

	if (length > pd->i_rpacket.s_max_bytes )
		pd->i_rpacket.s_max_bytes = length;

	if (length < pd->i_rpacket.s_min_bytes )
		pd->i_rpacket.s_min_bytes = length;

	pd->i_rpacket.s_time += time_cost;

	if ( time_cost > pd->i_rpacket.s_max_time )
		pd->i_rpacket.s_max_time = time_cost ;

	if ( time_cost < pd->i_rpacket.s_min_time )
		pd->i_rpacket.s_min_time = time_cost ;

#endif // STATICS

	err = error ;
err1:

DBG_EXIT( err );
	return( err );
}


/*****************************************************************************
 * �֐���
 *		_p_peek()
 * �@�\
 *		�p�P�b�g�s�[�N
 * �֐��l
 *		0	����
 *		0�ȊO   �G���[�ԍ�
 * ����
 ****************************************************************************/
int
_p_peek( pd, packet, lenp, mode )
	p_id_t	*pd;		/* �|�[�g�L�q�q		*/
	char	*packet;	/* �o�b�t�@		*/
	int	*lenp;		/* �o�b�t�@�T�C�Y	*/
	int	mode;		/* �s�[�N����p�P�b�g���[�h*/
{
	int		err;

DBG_ENT( M_LINK, "_p_peek");
//DBG_PRT( "_p_peek\n" );

	/*
	 * �|�[�g��ԃ`�F�b�N
	 */
	if( pd->i_stat & P_ID_DOWN ) {
		err = E_LINK_DOWN;
		goto err1;
	}

	/*
	 * ��M�L���[�`�F�b�N�y��P_WAIT���[�h��
	 * �̎�M�҂�
	 */
	int	len = *lenp;

	if( pd->i_stat & P_ID_CNTL ) {
		err = E_LINK_CNTL;
		goto err1;
	}
	err = PeekStream( pd->i_rfd, packet, len );
//DBG_PRT("len=%d errno=%d\n", len, errno );
	if( err < 0 ) {
		err = errno;
		goto err1;
	}

DBG_EXIT( 0 );
	return( 0 );
err1:

DBG_EXIT( err );
	return( err );
}


