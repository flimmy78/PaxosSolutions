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
 *	TBL_RDB.h
 *
 *	����	DB i/f header
 *
 ******************************************************************************/
#ifndef	_TBL_RDB_
#define	_TBL_RDB_

#include	"neo_db.h"
#include	<fcntl.h>
#include	"../../../../../cache/h/FileCache.h"

extern	BlockCacheCtrl_t	BCNTRL;
#define	pBC	(&BCNTRL)

#define	MY_FD_MAX	40	/* max fds */

#define	MY_FD	0x01		/* fd attached */

/*
 * DB table descriptor
 */
typedef struct my_tbl {
	r_tbl_t		*m_td;		/* DB_MAN table descriptor */
	int		m_stat;		/* status */
	int		m_fd;		/* file descriptor */
	int		m_data_off;	/* data offset */
	_dlnk_t		m_fd_active;	/* fd active list */
} my_tbl_t;

#define	MYTOTD( p )	((p)->m_td)
#define	TDTOMY( p )	((my_tbl_t *)(p)->t_tag)
#define	FDTOMY( p ) \
		(my_tbl_t*)((char*)(p)-(size_t)&((my_tbl_t*)0)->m_fd_active)

/*
 * function
 */
extern	void	fd_init( char * pPath );
extern	int		fd_drop();
extern	int		fd_create( r_tbl_name_t );
extern	int		fd_delete( int );
extern	int 	fd_open( my_tbl_t* );
extern	int 	fd_close( my_tbl_t* );
extern	int		fd_unlink( r_tbl_name_t );

extern	my_tbl_t	*my_tbl_alloc();
extern	void		my_tbl_free( my_tbl_t* );
extern	int 		my_tbl_store( my_tbl_t*, int, int, char* );
extern	int 		my_tbl_load( my_tbl_t*, int, int, char* );

extern	int		my_head_write( char*, int, long, int, 
								int, r_item_t[], uint64_t *pOff );
extern	int		my_head_update( my_tbl_t* );
extern	int		my_head_read( my_tbl_t*, uint64_t *pOff );
extern	int		my_item_write( char*, int, r_item_t[], uint64_t *pOff );
extern	int		my_item_read( my_tbl_t*, uint64_t *pOff );
extern	int		my_depend_write( char*, int, r_tbl_mod_t[], uint64_t *pOff );
extern	int		my_depend_read( my_tbl_t*, uint64_t *pOff );
extern	int64_t	my_data_off( my_tbl_t* );
extern	int64_t	my_depend_off( my_tbl_t* );

/*----------------------------------------------------------------------------

  ��  �m�d�n�^�c�a �t�@�C���t�H�[�}�b�g

  +----------------------------------------+
  |             ���R�[�h��                 |	4 �o�C�g�i�o�C�i���j
  +----------------------------------------+
  |             �L�����R�[�h��             |	4 �o�C�g�i�o�C�i���j
  +----------------------------------------+
  |             ���R�[�h��                 |	4 �o�C�g�i�o�C�i���j 
  +----------------------------------------+
  |             �A�C�e����                 |	4 �o�C�g�i�o�C�i���j
  +----+-----------------------------------+
  | �A |        �A�C�e����                 |	32�o�C�g�i������j
  + �C +-----------------------------------+
  | �e |        ����                       |	4 �o�C�g�i�o�C�i���j
  + �� +-----------------------------------+
  | �� |        �L���A�C�e����             |	4 �o�C�g�i�o�C�i���j
  + �� +-----------------------------------+
  | �� |        �A�C�e����                 |	4 �o�C�g�i�o�C�i���j
  + �� +-----------------------------------+
  |    |        ���R�[�h���I�t�Z�b�g�l     |	4 �o�C�g�i�o�C�i���j
  +----+----+------------------------------+
  | �O | �w |   ���Δԍ�                   |	4 �o�C�g�i�o�C�i���j
  + �� + �b +------------------------------+
  | �� | �_ |   ��Δԍ�                   |	4 �o�C�g�i�o�C�i���j
  + �� + �� +------------------------------+
  | �� |    |   ���                       |	4 �o�C�g�i�o�C�i���j
  + �R +----+------------------------------+
  | �b | �A |   0 �Ԗڂ̃A�C�e���f�[�^     |	�A�C�e���� �o�C�g�j
  | �h | �C |                              |
  + �f + �e +------------------------------+
  | �b | �� |   1 �Ԗڂ̃A�C�e���f�[�^     |     .....
  | �^ | �f |                              |
  +    + �b +------------------------------+
  |    | �^ |                              |
  |    |    |    ........                  |
  +----+----+------------------------------+
                 ........



  ��  �m�d�n�^�c�a  �C���^�t�F�[�X

  (1)  �f�[�^�x�[�X�̃I�[�v��
	
	TBL_DBOPEN( char *�f�[�^�x�[�X�� )


  (2)  �f�[�^�x�[�X�̃N���[�Y
	
	TBL_DBCLOSE( char *�f�[�^�x�[�X�� )


  (3)  �e�[�u���̐���
	
	TBL_CREATE( r_tbl_name_t , int rec_num, int item_num, r_item_t* )


  (4)  �e�[�u���̍폜
	
	TBL_DROP( r_tbl_name_t )


  (5)  �e�[�u���̃I�[�v��
	
	TBL_OPEN( r_tbl_t*, r_tbl_name_t )


  (6)  �e�[�u���̃N���[�Y
	
	TBL_CLOSE( r_tbl_t* )


  (7)  �e�[�u���̃��[�h
	
	TBL_LOAD( r_tbl_t*, int start_abs, int num, char* )


  (8)  �e�[�u���̃X�g�A
	
	TBL_STORE( r_tbl_t*, int start_abs, int num, char* )


  (9)  �e�[�u���̃w�b�_���̕ύX
	
	TBL_CNTL( r_tbl_t*, int cmd, char *arg )

----------------------------------------------------------------------------*/

#endif /* _TBL_RDB_ */
