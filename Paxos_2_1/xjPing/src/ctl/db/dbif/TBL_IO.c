/*******************************************************************************
 * 
 *  TBL_IO.c --- xjPing (On Memory DB) db I/F Library programs
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

#include	"TBL_RDB.h"

#ifdef	ZZZ

int	MyFdMax = MY_FD_MAX;
static int		fd_cnt;
static _dlnk_t	fd_active;
static char		PathDB[R_PATH_NAME_MAX];

char*
F( char *name )
{
	static char _f[R_PATH_NAME_MAX];

	if( PathDB[0] ) {strcpy(_f,PathDB);strcat(_f,"/");strcat(_f,name);}
	else	strcpy(_f,name );
	return( _f );
}

/*****************************************************************************
 *
 *  �֐���	fd_init()
 *
 *  �@�\	DB�e�[�u���L�q�q���X�g������
 * 
 *  �֐��l
 *     		����
 *
 ******************************************************************************/
void
fd_init( char *pPath )
{
	_dl_init(&fd_active);
	strcpy( PathDB, pPath );
}


/*****************************************************************************
 *
 *  �֐���	fd_drop()
 *
 *  �@�\	�Â�DB�e�[�u�����N���[�Y����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
fd_drop()
{
	long		past;
	_dlnk_t		*dlp;
	my_tbl_t	*md;
	my_tbl_t	*find;
	r_tbl_t		*td;

DBG_ENT(M_RDB,"fd_drop");

	find	= 0;
	past	= time(0);

	/*
	 * �Â�DB�e�[�u����{���o��
	 */
	for( dlp = _dl_head(&fd_active); _dl_valid(&fd_active,dlp);
			dlp = _dl_next(dlp) ) {
		md = FDTOMY(dlp);
		td = MYTOTD(md);
		if( td->t_access < past ) {
			past 	= td->t_access;
			find	= md;
		}
	}

	if( !find ) {
		neo_errno	= E_RDB_FD;
		goto err1;
	}

	/*
	 * �Y��DB�e�[�u�����N���[�Y����
	 */
	if( (neo_errno = fd_close( find )) )
		goto err1;

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	fd_creat()
 *
 *  �@�\	�t�@�C���𐶐�����
 * 
 *  �֐��l
 *     		����	���������t�@�C���̋L�q�q
 *		�ُ�	-1
 *
 ******************************************************************************/
int
fd_create( name )
	r_tbl_name_t	name;		/* ��������t�@�C����	*/
{
	int	fd;

DBG_ENT(M_RDB,"fd_create");

	/*
	 * �v���Z�X���̃t�@�C���L�q�q�������ȏゾ������A
	 * fd_drop()�ɂĂ���L�q�q���N���[�Y����
	 */
	if( fd_cnt > MyFdMax ) {
		if( fd_drop() )
			goto err1;
	}

	/*
	 * �t�@�C���𐶐�����
     *
	 * O_RDWR  : �ǂݏ����p
     * O_CREAT : �K�v�ɉ����Amode �w��̃p�[�~�b�V�����Ńt�@�C�����쐬����
     * O_EXCL  : O_CREAT�Ƌ��Ɏw�肳�ꂽ�ꍇ�A���łɃt�@�C�������݂���ꍇ�͎��s����B
     *           �����t�@�C�����Q�x�쐬���Ȃ����߂̃I�v�V�����B
	 */
	if( (fd = open( F(name), O_RDWR|O_CREAT|O_EXCL, 0666 )) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}

DBG_A(("fd = %d\n", fd ));
	fd_cnt++;

DBG_EXIT(fd);
	return( fd );
err1:
DBG_EXIT(-1);
	return( -1 );
}


/*****************************************************************************
 *
 *  �֐���	fd_delete()
 *
 *  �@�\	�t�@�C�����N���[�Y����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
fd_delete( fd )
	int	fd;
{

DBG_ENT(M_RDB,"fd_delete");
DBG_A(("fd=%d\n", fd ));

	if( close( fd ) ) {
		neo_errno = unixerr2neo();
		goto err1;
	}

	fd_cnt--;

DBG_EXIT( 0 );
	return( 0 );

err1:
DBG_EXIT(-1);
	return( -1 );
}

/*****************************************************************************
 *
 *  �֐���	fd_open()
 *
 *  �@�\	DB�e�[�u���L�q�q���I�[�v������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
fd_open( md )
	my_tbl_t	*md;		/* DB�e�[�u���L�q�q	*/
{
	int	fd;
DBG_ENT(M_RDB,"fd_open");

	/*
	 * �t�@�C���L�q�q������DB�e�[�u���L�q�q
	 * �ɃA�^�b�`����Ă���ꍇ�́A�������Ȃ�
	 */
	if( md->m_stat & MY_FD )
		goto exit;

	/*
	 * �v���Z�X���̃t�@�C���L�q�q�������ȏゾ������A
	 * fd_drop()�ɂĂ���L�q�q���N���[�Y����
	 */
	if( fd_cnt > MyFdMax ) {
		if( fd_drop() )
			goto err1;
	}

	/*
	 * �t�@�C�����I�[�v������
     *
	 * O_RDWR  : �ǂݏ����p
     * O_BINARY : Windows�n�Ńo�C�i���A�e�L�X�g�̃��[�h�w�������̂Ɏg�p����
	 */
	if( (fd = open( F(MYTOTD(md)->t_name), O_RDWR )) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
DBG_A(("fd=%d\n", fd ));

	/*
	 * DB�e�[�u���L�q�q�̃t�@�C���A�^�b�`�Ϗ��
	 * �����A�t�@�C���L�q�q�𒆂ɐݒ肷��
	 */
	fd_cnt++;
	md->m_fd	= fd;
	md->m_stat	|= MY_FD;

	/*
	 * �Y��DB�e�[�u���L�q�q�����X�g�ɓo�^����
	 */
	_dl_insert( &md->m_fd_active, &fd_active );

exit:
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(-1);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	fd_close()
 *
 *  �@�\	DB�e�[�u���L�q�q���N���[�Y����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
fd_close( md )
	my_tbl_t	*md;		/* DB�e�[�u���L�q�q	*/
{

DBG_ENT(M_RDB,"fd_close");

	/*
	 * �t�@�C���L�q�q������DB�e�[�u���L�q�q����
	 * �f�B�^�b�`���ꂽ�ꍇ�ɂ́A�������Ȃ�
	 */
	if( !(md->m_stat & MY_FD ) )
		goto exit;
DBG_A(("fd=%d\n", md->m_fd ));

	/*
	 * �t�@�C�����N���[�Y����
	 */
	if( close( md->m_fd ) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}

	/*
	 * DB�e�[�u���L�q�q�����X�g����O���āA
	 * �A�^�b�`�σr�b�g���N���A����
	 */
	_dl_remove( &md->m_fd_active );
	md->m_stat &= ~MY_FD;

	fd_cnt--;
DBG_A(("fd=%d,fd_cnt=%d\n", md->m_fd, fd_cnt ));
exit:
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT( neo_errno );
	return( neo_errno );
}

int
fd_unlink( r_tbl_name_t name )
{
	int	Ret;

	Ret = unlink( F(name) );
	return( Ret );
}

#endif	// ZZZ

/*****************************************************************************
 *
 *  �֐���	my_tbl_alloc()
 *
 *  �@�\	DB�e�[�u���L�q�q�p�G���A�𐶐�����
 * 
 *  �֐��l
 *     		����	���������G���A�̃A�h���X
 *		�ُ�	NULL
 *
 ******************************************************************************/
my_tbl_t	*
my_tbl_alloc()
{
	my_tbl_t *md;
DBG_ENT(M_RDB,"my_tbl_alloc");

	md = (my_tbl_t *)neo_zalloc(sizeof(my_tbl_t) );
DBG_EXIT(md);
	return( md );
}


/*****************************************************************************
 *
 *  �֐���	my_tbl_free()
 *
 *  �@�\	DB�e�[�u���L�q�q�p�G���A���������
 * 
 *  �֐��l
 *     		����
 *
 ******************************************************************************/
void
my_tbl_free( md )
	my_tbl_t	*md;		/* DB�e�[�u���L�q�q */
{
	r_tbl_t	*td;

DBG_ENT(M_RDB,"my_tbl_free");
DBG_A(("md=0x%x\n",md));

	/*
	 * �X�L�[�}�����Ă���΁A�������
	 */
	td = MYTOTD( md );
	if( td ) {
		if( td->t_scm ) {
			neo_free( td->t_scm );
			td->t_scm = NULL;
		}
		if( td->t_depend ) {
			neo_free( td->t_depend );
			td->t_depend = NULL;
		}
	}
	neo_free( md );

DBG_EXIT(0);
}


/*****************************************************************************
 *
 *  �֐���	my_head_write()
 *
 *  �@�\	���R�[�h�����t�@�C���w�b�_��
 *		�ɏ�������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
my_head_write( pName, type, modified, rec_num, item_num, items, pOff )
	char *pName;
	int		type;
	long		modified;
	int		rec_num;	/* ���R�[�h�̐�		*/
	int		item_num;	/* �A�C�e���̐�		*/
	r_item_t	items[];	/* �A�C�e�����G���A	*/
	uint64_t	*pOff;
{
	int	size;
	int	i;
	int	used = 0;
	uint64_t	Off;

DBG_ENT(M_RDB,"my_head_write");

	/*
	 * ���R�[�h�T�C�Y���Z�o����
	 */
	size	= sizeof(r_head_t);
	for( i = 0; i < item_num; i++ ) {
		size += items[i].i_size;
	}
DBG_A(("rec_num=%d,item_num=%d,size=%d\n", rec_num, item_num, size ));

/***
	if( lseek( fd, (off_t)0, 0 ) < 0 ) {
		goto err1;
	}
***/
	Off	= 0;

/***
	if( write( fd, &type, sizeof(type) ) < 0  )
		goto err1;
	if( write( fd, &modified, sizeof(modified) ) < 0  )
		goto err1;
***/

	if( FileCacheWrite( pBC, pName, Off, 
			sizeof(type), (char*)&type ) < 0 )
		goto err1;
	Off	+= sizeof(type);
	if( FileCacheWrite( pBC, pName, Off, 
			sizeof(modified), (char*)&modified ) < 0 )
		goto err1;
	Off	+= sizeof(modified);
	/*
	 * ���R�[�h�����AUSED���R�[�h���A���R�[�h�T�C�Y
	 * �����t�@�C���w�b�_���ɏ�������
	 */
/***
	if( write( fd, &rec_num, sizeof(rec_num) ) < 0  )
		goto err1;
	if( write( fd, &used, sizeof(used) ) < 0  )
		goto err1;
	if( write( fd, &size, sizeof(size) ) < 0  )
		goto err1;
***/

	if( FileCacheWrite( pBC, pName, Off, 
				sizeof(rec_num), (char*)&rec_num ) < 0 )
		goto err1;
	Off	+= sizeof(rec_num);
	if( FileCacheWrite( pBC, pName, Off, 
				sizeof(used), (char*)&used ) < 0 )
		goto err1;
	Off	+= sizeof(used);
	if( FileCacheWrite( pBC, pName, Off, 
				sizeof(size), (char*)&size ) < 0 )
		goto err1;
	Off	+= sizeof(size);

	*pOff	= Off;
DBG_EXIT(0);
	return( 0 );

err1:
	neo_errno	= unixerr2neo();
DBG_EXIT(neo_errno);
	return( neo_errno );
}



/*****************************************************************************
 *
 *  �֐���	my_head_update()
 *
 *  �@�\	�t�@�C���w�b�_���̏����X�V����
 *		(modified,used)
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
my_head_update( md )
	my_tbl_t	*md;		/* DB�e�[�u���L�q�q	*/
{
	r_tbl_t	*td;
	uint64_t	len;
	uint64_t	Off;

DBG_ENT(M_RDB,"my_head_update");

	td = MYTOTD( md );

/***
	if( (neo_errno = fd_open( md )) )
		goto err1;

	if( lseek( md->m_fd, (off_t)sizeof(int), 0 ) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
***/
	Off	= sizeof(int);

/****
	if( write( md->m_fd, &td->t_modified, sizeof(td->t_modified) ) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}

	if( write(md->m_fd, &td->t_rec_num, sizeof(td->t_rec_num)) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
DBG_A( (" file's used record information updated to %d\n", td->t_rec_used ) );

	if( write(md->m_fd, &td->t_rec_ori_used, sizeof(td->t_rec_used)) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
***/
	if( FileCacheWrite( pBC, td->t_name, Off, 
			sizeof(td->t_modified), (char*)&td->t_modified ) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
	Off	+= sizeof(td->t_modified);

	if( FileCacheWrite( pBC, td->t_name, Off,
			sizeof(td->t_rec_num), (char*)&td->t_rec_num ) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
	Off	+= sizeof(td->t_rec_num);

	if( FileCacheWrite( pBC, td->t_name, Off,
			sizeof(td->t_rec_used), (char*)&td->t_rec_ori_used ) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
	Off	+= sizeof(td->t_rec_used);

	if( td->t_depend ) {
/***
		if( lseek( md->m_fd, (off_t)my_depend_off( md ), 0 ) < 0 ) {
			neo_errno = unixerr2neo();
			goto err1;
		}
		if( write(md->m_fd, td->t_depend, 
			sizeof(r_tbl_list_t)
			+ sizeof(r_tbl_mod_t)*(td->t_depend->l_n - 1 ) ) < 0 ) {
			neo_errno = unixerr2neo();
			goto err1;
		}
***/
		Off	= my_depend_off( md );
		len	= sizeof(r_tbl_list_t)+sizeof(r_tbl_mod_t)*(td->t_depend->l_n-1);

		if( FileCacheWrite( pBC, td->t_name, Off, 
					len, (char*)td->t_depend) < 0 ) {
			neo_errno = unixerr2neo();
			goto err1;
		}
	}

	len	= my_data_off( md ) + td->t_rec_size*td->t_rec_num;
/***
	if( ftruncate(md->m_fd, len ) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
***/
	if( FileCacheTruncate( pBC, td->t_name, len ) < 0 ) {
		neo_errno = unixerr2neo();
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
 *  �֐���	my_head_read()
 *
 *  �@�\	DB�t�@�C���̃w�b�_���ǂݏo��
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 *  (��)	DB�t�@�C���̃t�H�[�}�b�g���Q�Ƃ��邱��
 ******************************************************************************/
int
my_head_read( myd, pOff )
	my_tbl_t	*myd;		/* DB�e�[�u���L�q�q	*/
	uint64_t	*pOff;
{
	int	type;
	long	modified;
	int	rec_num;
	int	size;
	int	used;
	r_tbl_t	*td;
	uint64_t	Off;

DBG_ENT(M_RDB,"my_head_read");

	td = MYTOTD( myd );
	/*
	 * DB�e�[�u�����I�[�v������
	 */
/***
	if( (neo_errno = fd_open( myd )) )
		goto err1;
***/

	/*
	 * �t�@�C���w�b�_�ɂ��郌�R�[�h�����AUSED���R�[�h��,
	 * ���R�[�h�T�C�Y����ǂݏo��
	 */
/***
	if( lseek( myd->m_fd, (off_t)0, 0 ) < 0 ) 
		goto err2;
	if( read( myd->m_fd, &type, sizeof(type) ) < 0  )
		goto err2;
	if( read( myd->m_fd, &modified, sizeof(modified) ) < 0  )
		goto err2;
	if( read( myd->m_fd, &rec_num, sizeof(rec_num) ) < 0  )
		goto err2;
	if( read( myd->m_fd, &used, sizeof(used) ) < 0  )
		goto err2;

	if ( used > rec_num ) {
		neo_errno = E_RDB_FILE;
		goto	err1;
	}

	if( read( myd->m_fd, &size, sizeof(size) ) < 0  )
		goto err2;
***/
	Off	= 0;

	if( FileCacheRead( pBC, td->t_name, Off,
				sizeof(type), (char*)&type ) < 0  )
		goto err2;
	Off	+= sizeof(type);

	if( FileCacheRead( pBC, td->t_name, Off,
			sizeof(modified), (char*)&modified ) < 0  )
		goto err2;
	Off	+= sizeof(modified);

	if( FileCacheRead( pBC, td->t_name, Off,
			sizeof(rec_num), (char*)&rec_num ) < 0  )
		goto err2;
	Off	+= sizeof(rec_num);

	if( FileCacheRead( pBC, td->t_name, Off,
			sizeof(used), (char*)&used ) < 0  )
		goto err2;
	Off	+= sizeof(used);

	if ( used > rec_num ) {
		neo_errno = E_RDB_FILE;
		goto	err1;
	}

	if( FileCacheRead( pBC, td->t_name, Off,
			sizeof(size), (char*)&size ) < 0  )
		goto err2;
	Off	+= sizeof(size);

	/*
	 * ��L�����e�[�u���L�q�q�ɐݒ肷��
	 */
	td->t_type		= type;
	td->t_modified		= modified;
	td->t_rec_size		= size;
	td->t_rec_num		= rec_num;
	td->t_rec_used		= used;
	td->t_rec_ori_used	= used;

	*pOff	= Off;

DBG_A(("rec_num=%d,used=%d,size=%d\n", rec_num, used, size ));
DBG_EXIT(0);
	return( 0 );
err2: DBG_T("err2");
	neo_errno = unixerr2neo();
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}



/*****************************************************************************
 *
 *  �֐���	my_item_write()
 *
 *  �@�\	�A�C�e�������t�@�C���֏�������
 * 
 *  �֐��l
 *     		����	���|�[�g�L�q�q�̃A�h���X
 *		�ُ�	0
 *
 ******************************************************************************/
int
my_item_write( pName, item_num, items, pOff )
	char	*pName;
	int		item_num;	/* �A�C�e���̐�		  */
	r_item_t	items[];	/* �A�C�e�����i�[�G���A */
	uint64_t	*pOff;
{
	int		off;
	int		i;
	uint64_t	Off, Len;

DBG_ENT(M_RDB,"my_item_write");

	/*
	 * �e�A�C�e���̃��R�[�h���̃I�t�Z�b�g���Z�o����
	 */
	for( i = 0, off = sizeof(r_head_t); i < item_num; i++ ) {
		items[i].i_off	= off;
		off		+= items[i].i_size;
DBG_A(("i=%d,off=%d,size=%d\n", i, items[i].i_off, items[i].i_size));
	}

	/*
	 * �A�C�e�������A�e�A�C�e�������t�@�C���w�b�_���ɏ�������
	 */
/***
	if( write( fd, &item_num, sizeof(item_num) ) < 0 ) {
		goto err1;
	}
	if( write( fd, items, item_num*sizeof(r_item_t) ) < 0 ) {
		goto err1;
	}
***/
	Off	= *pOff;

	if( FileCacheWrite( pBC, pName, Off,
			sizeof(item_num), (char*)&item_num ) < 0 ) {
		goto err1;
	}
	Off	+= sizeof(item_num);

	Len = item_num*sizeof(r_item_t);
	if( FileCacheWrite( pBC, pName, Off, Len, (char*)items ) < 0 ) {
		goto err1;
	}
	Off	+= Len;

	*pOff	= Off;

DBG_EXIT(0);
	return( 0 );
err1:
		neo_errno	= unixerr2neo();
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	my_item_read()
 *
 *  �@�\	DB�t�@�C���w�b�_���ɂ���A�C�e��
 *		����ǂݏo��
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 *  (��)	DB�t�@�C���̃t�H�[�}�b�g���Q�Ƃ��邱��
 *
 ******************************************************************************/
int
my_item_read( myd, pOff )
	my_tbl_t	*myd;		/* DB�e�[�u���L�q�q	*/
	uint64_t	*pOff;
{
	r_scm_t		*sp;
	int		item_num;
	struct	stat	buf;
	int		file_size;
	int		off_set;
	int		i;
	r_tbl_t	*td;
	uint64_t	Off, Len;

DBG_ENT(M_RDB,"my_item_read");

	td = MYTOTD( myd );

	/*
	 * DB�e�[�u���L�q�q���I�[�v������
	 */
/***
	if( (neo_errno = fd_open( myd )) )
		goto err1;
***/


	/*
	 * �t�@�C���w�b�_���̃A�C�e��������ǂݏo��
	 */
/***
	if( read( myd->m_fd, &item_num, sizeof(item_num) ) < 0 ) {
		neo_errno	= unixerr2neo();
		goto err1;
	}
***/
	Off	= *pOff;

	if( FileCacheRead( pBC, td->t_name, Off,
			sizeof(item_num), (char*)&item_num ) < 0 ) {
		neo_errno	= unixerr2neo();
		goto err1;
	}
	Off	+= sizeof(item_num);

	/*
	 * �t�@�C���T�C�Y�`�F�b�N
	 */
/***
	 if ( fstat( myd->m_fd, &buf ) ) {
		neo_errno = unixerr2neo();
		goto	err1;
	}
****/
	 if ( FileCacheStat( pBC, td->t_name, &buf ) ) {
		neo_errno = unixerr2neo();
		goto	err1;
	}

	file_size = td->t_rec_size * td->t_rec_used ;
	file_size += ( 16 + item_num * sizeof(r_item_t) );

	if ( (file_size > buf.st_size) || ( file_size <= 0 ) ) {
		neo_errno = E_RDB_FILE;
		goto	err1;
	}

	/*
	 * �X�L�[�}�p�G���A�𐶐�����
	 */
	if( !(sp = (r_scm_t*)neo_malloc(sizeof(r_scm_t)
				+ (item_num - 1 )*sizeof(r_item_t)) ) ) {
		neo_errno = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 * �X�L�[�}���e�[�u���L�q�q�ɐݒ肷��
	 */
	sp->s_n	= item_num;
	td->t_scm = sp;

	/*
	 * �t�@�C���w�b�_���̃A�C�e��������L
	 * �X�L�[�}�G���A�ɓǂݍ���
	 */
/***
	if( read( myd->m_fd, sp->s_item, item_num*sizeof(r_item_t) ) < 0 ) {
		neo_errno	= unixerr2neo();
		goto err1;
	}
***/
	Len	= item_num*sizeof(r_item_t);
	if( FileCacheRead( pBC, td->t_name, Off,
			Len, (char*)sp->s_item ) < 0 ) {
		neo_errno	= unixerr2neo();
		goto err1;
	}
	Off	+= Len;

	/*
	 * �A�C�e�����`�F�b�N
	 */
/***
	 off_set = 12 ;
***/
	 off_set = sizeof(r_head_t) ;
	 for ( i = 0; i < item_num; i ++ ) {

		if ( sp->s_item[i].i_off != off_set ) {
			neo_errno = E_RDB_FILE;
			goto	err1;
		}

		off_set += sp->s_item[i].i_size;
	 }

	 if ( off_set != td->t_rec_size ) {
		neo_errno = E_RDB_FILE;
		goto	err1;
	}

	*pOff	= Off;

DBG_A(("item_num=%d\n", item_num));
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}

int
my_depend_write( pName, num, depends, pOff )
	char	*pName;
	int		num;
	r_tbl_mod_t	depends[];
	uint64_t	*pOff;
{
	uint64_t	Off, Len;

DBG_ENT(M_RDB,"my_depend_write");

	Off	= *pOff;

/***
	if( write( fd, &num, sizeof(num) ) < 0 ) {
		goto err1;
	}
	if( num > 0 ) {
		if( write( fd, depends, sizeof(r_tbl_mod_t)*num ) < 0 ) {
			goto err1;
		}
	}
***/
	if( FileCacheWrite( pBC, pName, Off,
			sizeof(num), (char*)&num ) < 0 ) {
		goto err1;
	}
	Off	+= sizeof(num);

	if( num > 0 ) {
/***
		if( write( fd, depends, sizeof(r_tbl_mod_t)*num ) < 0 ) {
			goto err1;
***/
		Len	= sizeof(r_tbl_mod_t)*num;
		if( FileCacheWrite( pBC, pName, Off,
				Len, (char*)depends ) < 0 ) {
			goto err1;
		}
		Off	+= Len;
	}
	*pOff	= Off;

DBG_EXIT( 0 );
	return( 0 );
err1:
	neo_errno	= unixerr2neo();
DBG_EXIT(neo_errno);
	return( neo_errno );
}

int
my_depend_read( myd, pOff )
	my_tbl_t	*myd;		/* DB�e�[�u���L�q�q	*/
	uint64_t	*pOff;
{
	int		num;
	r_tbl_list_t	*dp;
	r_tbl_t	*td;
	uint64_t	Off, Len;

DBG_ENT(M_RDB,"my_depend_write");

	td = MYTOTD( myd );

/***
	if( (neo_errno = fd_open( myd )) )
		goto err1;
***/
	Off	= *pOff;

/***
	if( read( myd->m_fd, &num, sizeof(num) ) < 0 ) {
		neo_errno	= unixerr2neo();
		goto err1;
	}
***/
	if( FileCacheRead( pBC, td->t_name, Off,
			sizeof(num), (char*)&num ) < 0 ) {
		neo_errno	= unixerr2neo();
		goto err1;
	}
	Off	+= sizeof(num);

	if( num > 0 ) {
		if( !(dp = (r_tbl_list_t*)neo_malloc(sizeof(r_tbl_list_t)
				+ (num - 1 )*sizeof(r_tbl_mod_t)) ) ) {
			neo_errno = E_RDB_NOMEM;
			goto err1;
		}
/***
		if( read( myd->m_fd, dp->l_tbl, sizeof(r_tbl_mod_t)*num ) < 0 ) {
			neo_errno	= unixerr2neo();
			goto err1;
		}
***/
		Len	= sizeof(r_tbl_mod_t)*num;
		if( FileCacheRead( pBC, td->t_name, Off,
				Len, (char*)dp->l_tbl ) < 0 ) {
			neo_errno	= unixerr2neo();
			goto err1;
		}
		Off	+= Len;

		dp->l_n	= num;
		td->t_depend = dp;
	}
DBG_EXIT( 0 );
	return( 0 );
err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}

/*****************************************************************************
 *
 *  �֐���	my_data_off()
 *
 *  �@�\	�t�@�C���f�[�^���̐擪�I�t�Z�b�g���Z�o����
 * 
 *  �֐��l
 *     		�I�t�Z�b�g�l
 *
 ******************************************************************************/
int64_t
my_data_off( myd )
	my_tbl_t	*myd;		/* DB�e�[�u���L�q�q	*/
{
	int64_t	off;

DBG_ENT(M_RDB,"my_data_off");

	off 	= my_depend_off( myd );
	off	+= sizeof(int);
	if( MYTOTD(myd)->t_depend ) {
		off	+= MYTOTD(myd)->t_depend->l_n*sizeof(r_tbl_mod_t);
	}
DBG_EXIT(off);
	return( off );
}

int64_t
my_depend_off( myd )
	my_tbl_t	*myd;		/* DB�e�[�u���L�q�q	*/
{
	int64_t	off;

DBG_ENT(M_RDB,"my_data_off");

	off	= sizeof(MYTOTD(myd)->t_type);
	off	+= sizeof(MYTOTD(myd)->t_modified);
	off	+= sizeof(MYTOTD(myd)->t_rec_num);
	off	+= sizeof(MYTOTD(myd)->t_rec_used);
	off	+= sizeof(MYTOTD(myd)->t_rec_size);
	off	+= sizeof(MYTOTD(myd)->t_scm->s_n);
	off	+= MYTOTD(myd)->t_scm->s_n * sizeof(r_item_t);

DBG_EXIT(off);
	return( off );
}


/*****************************************************************************
 *
 *  �֐���	my_tbl_store()
 *
 *  �@�\	�e�[�u�����t�@�C���֏�������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
my_tbl_store( md, s, n, bufp )
	my_tbl_t	*md;		/* DB�e�[�u���L�q�q	*/
	int		s;		/* �J�n���R�[�h�ԍ�	*/
	int		n;		/* ���R�[�h��		*/
	char		*bufp;		/* ���R�[�h�i�[�G���A	*/
{
	r_tbl_t	*td;
	uint64_t	Off, Len;

DBG_ENT(M_RDB,"my_tbl_store");

	td = MYTOTD( md );

	/* 
	 * DB�e�[�u�����I�[�v������
	 */
/***
	if( (neo_errno = fd_open( md )) )
		goto err1;
***/

	/*
	 * �t�@�C���L�q�q�|�C���^�𒲐����āA�������݂��s��
	 */
/***
	if( lseek( md->m_fd, (off_t)(md->m_data_off+s*td->t_rec_size), 0) < 0 )
		goto err2;;

	if( write( md->m_fd, bufp, n*td->t_rec_size ) < 0 )
		goto err2;;
***/
	Off	= md->m_data_off + s*td->t_rec_size;
	Len	= n*td->t_rec_size;

	if( FileCacheWrite( pBC, td->t_name, Off, Len, bufp ) < 0 )
		goto err2;

DBG_EXIT(0);
	return( 0 );
err2: DBG_T("err2");
	neo_errno = unixerr2neo();
//err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}


/*****************************************************************************
 *
 *  �֐���	my_tbl_load()
 *
 *  �@�\	�t�@�C������e�[�u���Ƀ��R�[�h�����[�h����
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
my_tbl_load( md, s, n, bufp )
	my_tbl_t	*md;		/* DB�e�[�u���L�q�q	*/
	int		s;		/* �J�n���R�[�h�ԍ�	*/
	int		n;		/* �ǂݏo�����R�[�h��	*/
	char		*bufp;		/* ���R�[�h�i�[�p�G���A */
{
	r_tbl_t	*td;
	int	len;
	uint64_t	Off, Len;
DBG_ENT(M_RDB,"my_tbl_load");

	td = MYTOTD( md );

	/* 
	 * DB�e�[�u���L�q�q���I�[�v������
	 */
/***
	if( (neo_errno = fd_open( md )) )
		goto err1;
***/

	/*
	 * �t�@�C���L�q�q�̃V�[�N�|�C���^�𒲐����āA
	 * �ǂݏo�����s��
	 */
/***
	if( lseek( md->m_fd, (off_t)(md->m_data_off+s*td->t_rec_size), 0 ) < 0 )
		goto err2;

	len = read( md->m_fd, bufp, n*td->t_rec_size );
***/
	Off	= md->m_data_off+s*td->t_rec_size;
	Len	= n*td->t_rec_size;

	len	= FileCacheRead( pBC, td->t_name, Off, Len, bufp );

	if ( len < 0 )
		goto err2;
	else if ( len < n*td->t_rec_size )
		bzero( bufp + len , n*td->t_rec_size - len ) ;

DBG_EXIT(0);
	return( 0 );
err2: DBG_T("err2");
	neo_errno	= unixerr2neo();
//err1:
DBG_EXIT(neo_errno);
	return( neo_errno );
}

char*
F( char *name )
{
	static char _f[R_PATH_NAME_MAX];

	sprintf( _f, "%s/%s", pBC->bc_FdCtrl.fc_Root, name );
	return( _f );
}

int
f_create( char* pFile )
{
	int	fd;

	if( (fd = open( F(pFile), O_RDWR|O_CREAT|O_EXCL|O_TRUNC, 0666 )) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
	return( fd );
err1:
	return( -1 );
}

int
f_open( char* pFile )
{
	int	fd;

	if( (fd = open( F(pFile), O_RDWR )) < 0 ) {
		neo_errno = unixerr2neo();
		goto err1;
	}
	return( fd );
err1:
	return( -1 );
}

int
f_unlink( char* pFile )
{
	int	Ret;

	Ret = unlink( F(pFile) );
	return( Ret );
}

#ifdef	ZZZ
DIR*
f_opendir()
{
	DIR	*pDir;

	pDir	= opendir( PathDB );
	return( pDir );
}

int
f_closedir( DIR *pDir )
{
	return( closedir( pDir ) );
}

struct dirent*
f_readdir( DIR *pDir )
{
	return( readdir( pDir ) );
}

void
f_rewinddir( DIR *pDir )
{
	return( rewinddir( pDir ) );
}

int
f_stat( char *pName, struct stat *pStat )
{
	return( stat( F(pName), pStat ) );
}
#endif	// ZZZ
