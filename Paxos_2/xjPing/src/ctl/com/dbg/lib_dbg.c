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
 *	����	�f�o�b�O�o�͗p�@���C�u�����[   (lib_dbg.c)
 * 
 ******************************************************************************/

#include <stdio.h>
#include <time.h>
/***
#include <varargs.h>
***/
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "lib_dbg.h"

/**********************/
/* �O���[�o���ϐ���` */
/**********************/

char	*_Dbg_filename;			/* �t�@�C����			*/
char	*_Dbg_func;			/* �֐���			*/
char	 _Dbg_level;			/* �o�̓��x��			*/
int	 _Dbg_line;			/* �o�̓��C��			*/
int	 _Dbg_class;			/* �o�̓N���X			*/

#define	D_MAX_TBL	256		/* �ő�e�[�u���T�C�Y		*/
static	char	 _Dbg_tbl[D_MAX_TBL];	/* �f�o�b�O���x���@�e�[�u��	*/
static	FILE	*_Dbg_fp = (FILE*)NULL;	/* �o�̓t�@�C���|�C���^�[	*/

/************/
/* �֐��錾 */
/************/

char	*strtok();
char	*getenv();
/***
int	printf();
char	*neo_malloc();
***/
char	*neo_malloc();
void	neo_free();

static	int	dbg_chk_arg();		/* �����̃`�F�b�N		*/
static	int	dbg_set_level();	/* �f�o�b�O���x���̐ݒ�		*/
static	int	dbg_set_tbl();		/* �e�[�u���̃Z�b�g		*/
static	int	dbg_set_tbl_all();	/* �e�[�u���̃Z�b�g		*/
static	int	dbg_get_class();	/* �N���X�C���f�b�N�X�̎擾	*/
static	char	dbg_get_level();	/* ���x���̎擾			*/
static	int	dbg_open_file();	/* �o�̓t�@�C���̃I�[�v��	*/
static	void	dbg_write_header();	/* �w�b�_�[�̏�������		*/
static	char	*dbg_get_time();	/* ���ݎ����̎擾		*/

/**************************/
/* �f�o�b�O�p�@�}�N����` */
/**************************/

#ifdef	DEBUG

#define	PRINTF				printf
#define	FUNC1				dbg_table
#define	FUNC2				dbg_find_classname
#define	FUNC3				dbg_find_levelname
#define	_printf(a) 			PRINTF a
#define	_dbg_table()			FUNC1()
#define	_dbg_find_classname(a,b)	FUNC2(a,b)
#define	_dbg_find_levelname(a,b)	FUNC3(a,b)

#else	// DEBUG

#define	_printf(a)
#define	_dbg_table()
#define	_dbg_find_classname(a,b)
#define	_dbg_find_levelname(a,b)

#endif	// DEBUG

/******************************************************************************
 * �֐���	_dbg_init
 * �@�\		�f�o�b�O�̏�����
 * �֐��l	�O�@�@�F�@����I��
 *		�|�P�@�F�@�G���[
 * ����
 ******************************************************************************/

extern	int
_dbg_init( ac, av )
int	*ac;
char	*av[];
{
	char	*dbg_level = (char*)NULL;	/* �f�o�b�O���x��������	*/
	char	*procname  = (char*)NULL;	/* �v���Z�X����		*/
	char	save_av[1024];			/* �����̃Z�[�u		*/
	int	ret_cd  = OK;			/* ���^�[���@�R�[�h	*/
	int	dbg_flg = 0;			/* �f�o�b�O�@�t���O	*/

	_printf(( "Enter %s(ac=%d,av[0]=%s)\n", __FUNCTION__, *ac, av[0] ));

	/******************/
	/* �����̃`�F�b�N */
	/******************/

	if ( dbg_chk_arg(ac, av, &dbg_level, &procname, save_av) == ERR ) {
		_printf(( "      %s : dbg_chk_arg ERR !\n", func ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : dbg_chk_arg OK !\n", func ));

	/************************/
	/* �f�o�b�O�e�[�u���쐬 */
	/************************/

	if ( dbg_set_level(dbg_level, &dbg_flg) == ERR ) {
		_printf(( "      %s : dbg_set_level ERR !\n", func ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : dbg_set_level OK !\n", func ));

	if ( dbg_flg == 0 ) {
		_printf(( "      %s : dbg_flg == 0 : goto ret !\n", func ));
		goto 	ret;
	}

	/**************************/
	/* �o�̓t�@�C���̃I�[�v�� */
	/**************************/

	if ( dbg_open_file(procname) == ERR ) {
		_printf(( "      %s : dbg_open_file(%s) ERR !\n",
			func, procname ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : dbg_open_file(%s) OK !\n", func, procname ));

	/**********************/
	/* �w�b�_�[�̏������� */
	/**********************/

	dbg_write_header( save_av );
	_printf(( "      %s : dbg_write_header OK !\n", func ));

	/********/
	/* �I�� */
	/********/
ret:
	if ( procname  ) {
		neo_free( procname );
		_printf(( "      %s : neo_free procname OK !\n", func ));
	}

	_printf(( "Exit  %s(ret=%d)\n", func, ret_cd ));
	return( ret_cd );
}

/******************************************************************************
 * �֐���	_dbg_term
 * �@�\		�f�o�b�O�̏I������
 * �֐��l	�Ȃ�
 * ����
 ******************************************************************************/

extern	void
_dbg_term()
{
	_printf(( "Enter %s\n", __FUNCTION__ ));

	/**********************/
	/* �I�������̏������� */
	/**********************/

	if ( _Dbg_fp  ) {
		_printf(( "      %s : write dbg_get_time OK !\n", func ));
		fprintf( _Dbg_fp, "\n%s\n", dbg_get_time() );
	}

	/********/
	/* �I�� */
	/********/

	if ( _Dbg_fp  ) {
		fclose( _Dbg_fp );
		_Dbg_fp = (FILE*)NULL;
		_printf(( "      %s : fclose OK !\n", func ));
	}

	_printf(( "Exit  (void)%s\n", func ));
}

/* VARARGS2 */
/******************************************************************************
 * �֐���	_dbg_printf
 * �@�\		�f�o�b�O�\��
 * �֐��l	�Ȃ�
 * ����
 ******************************************************************************/

extern	int
_dbg_print_check()
{
#ifdef	DEBUG
	va_list ap;
	char	classname[32];
	char	levelname[32];
#endif
	int	check = 0;

	_dbg_find_classname( _Dbg_class, classname );
	_dbg_find_levelname( _Dbg_level, levelname );
	_printf(( "Enter %s( %s, %s )\n", __FUNCTION__, classname, levelname ));

	/******************************************************/
	/* �t�@�C�����I�[�v������Ă��Ȃ����́A�����o�͂��Ȃ� */
	/******************************************************/

	if ( _Dbg_fp == (FILE*)NULL ) {
		_printf(( "      %s : _Dbg_fp == NULL!\n", func ));
		goto	ret;
	}

	/********************/
	/* �N���X  �`�F�b�N */
	/********************/

	if ( _Dbg_class < 0 || _Dbg_class > D_MAX_TBL ) {
		_printf(( "      %s : class check ERR ! : class = %d\n",
			func, _Dbg_class ));
		goto	ret;
	}

	_printf(( "      %s : class check OK ! : class = %d\n",
			func, _Dbg_class ));

	/************/
	/* �o�͔��� */
	/************/

	if ( (_Dbg_level & _Dbg_tbl[_Dbg_class]) == 0 ) {
		_printf(( "      %s : �o�͂Ȃ�\n", func ));
		goto	ret;
	}
	check = 1;
	fprintf(  _Dbg_fp, "%-12s %4d  ", _Dbg_filename, _Dbg_line );
ret:
	return( check );
}

#ifdef XXX
extern	void
_dbg_printf1( va_alist )
va_dcl
{
	va_list ap;
	char	*format;
	static	char	buf[BUFSIZ];

	if ( _Dbg_fp == (FILE*)NULL ) {
		return;
	}
	va_start(ap);
	format = va_arg( ap, char* );
	vsprintf( buf, format, ap );
	fprintf( _Dbg_fp, buf );
}

#else

extern	void
_dbg_printf1( char *format, ... )
{
	va_list ap;
	static	char	buf[BUFSIZ];

	if ( _Dbg_fp == (FILE*)NULL ) {
		return;
	}
	va_start(ap, format );
	vsprintf( buf, format, ap );
	fprintf( _Dbg_fp, "%s", buf );
	va_end(ap);
}
#endif

#ifdef XXX
extern	void
_dbg_printf( va_alist )
va_dcl
{
	va_list ap;
	neo_level_sym_t	*ptr;
	char	*func = "_dbg_printf";		/* �֐���		*/
	char	*format;
	int	length;
	int	index;
	int	idx;
	int	err    = errno;
	int	lv_flg = 0;
	char	classname[32];
	char	levelname[32];
	static	char	buf[BUFSIZ];

	_dbg_find_classname( _Dbg_class, classname );
	_dbg_find_levelname( _Dbg_level, levelname );
	_printf(( "Enter %s( %s, %s )\n", func, classname, levelname ));

	/******************************************************/
	/* �t�@�C�����I�[�v������Ă��Ȃ����́A�����o�͂��Ȃ� */
	/******************************************************/

	if ( _Dbg_fp == (FILE*)NULL ) {
		_printf(( "      %s : _Dbg_fp == NULL!\n", func ));
		goto	ret;
	}

	/********************/
	/* �N���X  �`�F�b�N */
	/********************/

	if ( _Dbg_class < 0 || _Dbg_class > D_MAX_TBL ) {
		_printf(( "      %s : class check ERR ! : class = %d\n",
			func, _Dbg_class ));
		goto	ret;
	}

	_printf(( "      %s : class check OK ! : class = %d\n",
			func, _Dbg_class ));

	/************/
	/* �o�͔��� */
	/************/

	if ( (_Dbg_level & _Dbg_tbl[_Dbg_class]) == 0 ) {
		_printf(( "      %s : �o�͂Ȃ�\n", func ));
		goto	ret;
	}

	_printf(( "      %s : �o��\n", func ));

	/**************************/
	/* �t�H�[�}�b�g�@�`�F�b�N */
	/**************************/

	va_start(ap);
	format = va_arg( ap, char* );

	if ( format == NULL ) {
		_printf(( "      %s : format ERR !\n", func ));
		goto	ret;
	}

	_printf(( "      %s : format OK !\n", func ));

	/****************/
	/* �t�@�C���o�� */
	/****************/

	fprintf(  _Dbg_fp, "%-12s %4d  ", _Dbg_filename, _Dbg_line );

	if ( !(_Dbg_level & _LEV_T) ) {
		fprintf(  _Dbg_fp, "   " );
	}

	vsprintf( buf, format, ap );
	fprintf( _Dbg_fp, buf );
	_printf(( "      %s : fprintf OK ! : buf = \"%s\"\n", func, buf ));

	errno = err;

	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  (void)%s\n", func ));
	return;
}
#else
extern	void
_dbg_printf( char *format, ... )
{
	va_list ap;
#ifdef	DEBUG
	neo_level_sym_t	*ptr;
	int	length;
	int	index;
	int	idx;
	int	lv_flg = 0;
	char	classname[32];
	char	levelname[32];
#endif
	int	err    = errno;
	static	char	buf[BUFSIZ];

	_dbg_find_classname( _Dbg_class, classname );
	_dbg_find_levelname( _Dbg_level, levelname );
	_printf(( "Enter %s( %s, %s )\n", __FUNCTION__, classname, levelname ));

	/******************************************************/
	/* �t�@�C�����I�[�v������Ă��Ȃ����́A�����o�͂��Ȃ� */
	/******************************************************/

	if ( _Dbg_fp == (FILE*)NULL ) {
		_printf(( "      %s : _Dbg_fp == NULL!\n", func ));
		goto	ret;
	}

	/********************/
	/* �N���X  �`�F�b�N */
	/********************/

	if ( _Dbg_class < 0 || _Dbg_class > D_MAX_TBL ) {
		_printf(( "      %s : class check ERR ! : class = %d\n",
			func, _Dbg_class ));
		goto	ret;
	}

	_printf(( "      %s : class check OK ! : class = %d\n",
			func, _Dbg_class ));

	/************/
	/* �o�͔��� */
	/************/

	if ( (_Dbg_level & _Dbg_tbl[_Dbg_class]) == 0 ) {
		_printf(( "      %s : �o�͂Ȃ�\n", __FUNCTION__ ));
		goto	ret;
	}

	_printf(( "      %s : �o��\n", __FUNCTION__ ));

	/**************************/
	/* �t�H�[�}�b�g�@�`�F�b�N */
	/**************************/

	va_start(ap,format);

	if ( format == NULL ) {
		_printf(( "      %s : format ERR !\n", __FUNCTION__ ));
		goto	ret;
	}

	_printf(( "      %s : format OK !\n", __FUNCTION__ ));

	/****************/
	/* �t�@�C���o�� */
	/****************/

	fprintf(  _Dbg_fp, "%-12s %4d  ", _Dbg_filename, _Dbg_line );

	if ( !(_Dbg_level & _LEV_T) ) {
		fprintf(  _Dbg_fp, "   " );
	}

	vsprintf( buf, format, ap );
	fprintf( _Dbg_fp, "%s", buf );
	_printf(( "      %s : fprintf OK ! : buf = \"%s\"\n", __FUNCTION__, buf ));

	errno = err;

	/********/
	/* �I�� */
	/********/
ret:
	va_end( ap );
	_printf(( "Exit  (void)%s\n", __FUNCTION__ ));
	return;
}
#endif

/******************************************************************************
 * �֐���	dbg_chk_arg
 * �@�\		�����̃`�F�b�N
 * �֐��l	�O�@�@�F�@����I��
 *		�|�P�@�F�@�G���[
 * ����
 ******************************************************************************/

static	int
dbg_chk_arg( ac, av, dbg_level, procname, save_av )
int	*ac;
char	*av[];
char	**dbg_level;
char	**procname;
char	*save_av;
{
	int	ret_cd = OK;			/* ���^�[���@�R�[�h	*/
	int	size;
	int	idx, i, j;
	char	*cp;

	_printf(( "Enter %s(ac=%d,av[0]=%s)\n", __FUNCTION__, *ac, av[0] ));

	/******************/
	/* �G���[�`�F�b�N */
	/******************/

	if ( ac == NULL || *ac <= 0 ) {
		_printf(( "      %s : ac ERR ! : ac = %d\n", __FUNCTION__, ac ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : ac, av OK !\n", func ));

	/********************/
	/* �v���Z�X���@�擾 */
	/********************/

	size = strlen( av[0] );

	if ( (*procname = neo_malloc(size+1)) == NULL ) {
		_printf(( "      %s : neo_malloc(%d) ERR !\n", func, size+1 ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : neo_malloc(%d) OK !\n", func, size + 1 ));

	if ( (cp = strrchr(av[0], '/')) == NULL ) {
		strcpy( *procname, av[0] );
	} else {
		strcpy( *procname, (cp+1) );
	}

	_printf(( "      %s : procname = %s\n", func, *procname ));

	/****************/
	/* �����̃Z�[�u */
	/****************/

	strcpy( save_av, *procname );
	strcat( save_av, " " );

	for (idx = 1; idx < *ac; idx++) {

		strcat( save_av, av[idx] );
		strcat( save_av, " " );
	}

	_printf(( "      %s : save av OK !\n", func ));

	/******************************/
	/* �f�o�b�O�@�I�v�V�����@���� */
	/******************************/

	for (idx = 1; idx < *ac; idx++) {

		if ( strcmp(av[idx], D_OPT_ARG) == 0 ) {

			if ( idx + 1 == *ac || av[idx+1] == NULL ) {
				_printf(( "      %s : av ERR !\n", func ));
				ret_cd = ERR;
				goto	ret;
			}

			size = strlen( av[idx+1] );
			if ( (*dbg_level = neo_malloc(size+1)) == NULL ) {
				_printf(( "      %s : neo_malloc ERR !\n",
					func ));
				ret_cd = ERR;
				goto	ret;
			}
			strcpy( *dbg_level, av[idx+1] );
			av[idx]   = NULL;
			av[idx+1] = NULL;
			_printf(( "      %s : dbg_level = \"%s\" !\n",
				func, *dbg_level ));
			break;
		}
	}

	/****************************/
	/* �f�o�b�O�I�v�V�����̍폜 */
	/****************************/

	for (i = j = 1; i < *ac; i++) {
		if (av[i])
			av[j++] = av[i];
	}
	av[j] = NULL;
	*ac = j;

	_printf(( "      %s : delete options OK !\n", func ));

	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  %s(ret=%d)\n", func, ret_cd ));
	return( ret_cd );
}

/******************************************************************************
 * �֐���	dbg_set_level
 * �@�\		�f�o�b�O���x���̐ݒ�
 * �֐��l	�O�@�@�F�@����I��
 *		�|�P�@�F�@�G���[
 * ����
 ******************************************************************************/

static	int
dbg_set_level( dbg_level, dbg_flg )
char	*dbg_level;
int	*dbg_flg;
{
	int	 ret_cd = OK;			/* ���^�[���@�R�[�h	*/
	char	*class, *level;
	char	*cp;
	int	length;
	int	idx;

	if ( dbg_level ) {
		_printf(( "Enter %s(lev=%s,flg=%d)\n",
			__FUNCTION__, dbg_level, *dbg_flg ));
	} else {
		_printf(( "Enter %s(lev=null,flg=%d)\n", __FUNCTION__, *dbg_flg ));
	}

	/**************************/
	/* �f�o�b�O�t���O�@������ */
	/**************************/

	*dbg_flg = 0;

	/********************************/
	/* �f�o�b�O���x�����ϐ��@�擾 */
	/********************************/

	if ( dbg_level == NULL ) {	/* 1993�N10��07�� (��) */
					/* 1993�N10��27�� (��) */

		/******************/
		/* ���ϐ��@�擾 */
		/******************/

		if ( (cp = getenv(D_OPT_ENV)) == NULL ) {
			_printf(( "      %s : getenv == NULL\n", func ));
			goto	ret;
		}

		_printf(( "      %s : getenv(%s) OK !\n", func, D_OPT_ENV ));

		/****************************/
		/* �������[�@�A���P�[�V���� */
		/****************************/

		length = strlen( cp ) + 1;

		if ( (dbg_level = neo_malloc(length)) == NULL ) {

			_printf(( "      %s : neo_malloc ERR !\n", func ));
			ret_cd = ERR;
			goto	ret;
		}

		_printf(( "      %s : neo_malloc(%d) OK !\n", func, length ));

		/********************/
		/* ���ϐ��@�R�s�[ */
		/********************/

		strcpy( dbg_level, cp );
		_printf(( "      %s : strcpy OK !\n", func ));
	}

	_printf(( "      %s : dbg_level = %s\n", func, dbg_level ));

	/**********************************************/
	/* �`�k�k�̎��́A�S�N���X�Ƀ��x�����Z�b�g���� */
	/**********************************************/

	length = strlen( D_OPT_ALL );

	if ( strncmp(dbg_level, D_OPT_ALL, length) == 0 ) {

		if ( dbg_level[length] == '\0' ) {
			_printf(("      %s : dbg_level(%s) ERR !\n",
				func, dbg_level ));
			ret_cd = ERR;
			goto	ret;
		}

		level = &(dbg_level[length+1]);

		if ( dbg_set_tbl_all(level) == ERR ) {
			_printf(("      %s : dbg_set_tbl_all ERR !\n", func ));
			ret_cd = ERR;
			goto	ret;
		}

		_printf(("      %s : dbg_set_tbl_all(%s) OK !\n", func, level));

		/**************************/
		/* �f�o�b�O�t���O�@�Z�b�g */
		/**************************/

		*dbg_flg = 1;
		_dbg_table();
		goto	ret;
	}

	/********************/
	/* �e�[�u���̏����� */
	/********************/

	for ( idx = 0; idx < D_MAX_TBL; idx++ ) {
		_Dbg_tbl[idx] = 0;
	}

	_printf(( "      %s : init _Dbg_tbl OK !\n", func ));

	/****************************************/
	/* �f�o�b�O���x�����e�[�u���փZ�b�g���� */
	/****************************************/

	for (class = dbg_level; (class=strtok(class, D_OPT_DEL)); class = NULL){

		/* 1993�N10��07�� (��) */

		if ( (level = strchr(class, D_OPT_LEV)) == NULL ) {
			_printf(( "      %s : strchr ERR ! : class = %s\n",
				func, class ));
			ret_cd = ERR;
			goto	ret;
		}

		*level = '\0';

		if ( *(++level) == '\0' ) {
			_printf(( "      %s : level = NULL !\n", func ));
			ret_cd = ERR;
			goto	ret;
		}

		_printf(( "      %s : strchr OK ! : class = %s : level = %s\n",
			func, class, level ));

		if ( dbg_set_tbl(class, level) == ERR ) {
			_printf(( "      %s : dbg_set_tbl ERR !\n", func ));
			ret_cd = ERR;
			goto	ret;
		}
	}

	_printf(( "      %s : set _Dbg_tbl OK !\n", func ));

	/**************************/
	/* �f�o�b�O�t���O�@�Z�b�g */
	/**************************/

	*dbg_flg = 1;
	_dbg_table();

	/********/
	/* �I�� */
	/********/
ret:
	if ( *dbg_flg ) {
		_printf(( "      %s : �f�o�b�O�@�n�m\n", func ));
	} else {
		_printf(( "      %s : �f�o�b�O�@�n�e�e\n", func ));
	}

	_printf(( "Exit  %s(ret=%d)\n", func, ret_cd ));
	return( ret_cd );
}

/******************************************************************************
 * �֐���	dbg_set_tbl
 * �@�\		�f�o�b�O���x�����e�[�u���փZ�b�g����
 * �֐��l	�O�@�@�F�@����I��
 *		�|�P�@�F�@�G���[
 * ����
 ******************************************************************************/

static	int
dbg_set_tbl( class, level_str )
char	*class;
char	*level_str;
{
	int	ret_cd = OK;			/* ���^�[���@�R�[�h	*/
	char	level;
	int	index;

	_printf(( "Enter %s(class=%s,level=%s)\n", __FUNCTION__, class, level_str ));

	/********************/
	/* �N���X�@�`�F�b�N */
	/********************/

	if ( (index = dbg_get_class(class)) == ERR ) {
		_printf(( "      %s : dbg_get_class ERR !\n", func ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(("      %s : dbg_get_class OK ! class = %d\n", func, class));

	/********************/
	/* ���x���@�`�F�b�N */
	/********************/

	if ( level_str == NULL ) {
		_printf(( "      %s : check level ERR !\n", func ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : check level(%s) OK !\n", func, level_str ));

	if ( (level = dbg_get_level(level_str)) == 0 ) {
		_printf(( "      %s : dbg_get_level ERR !\n", func ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : dbg_get_level OK !\n", func ));

	/********************************/
	/* ���x�����e�[�u���փZ�b�g���� */
	/********************************/

	_Dbg_tbl[index] = level;
	_printf(( "      %s : set level OK !\n", func ));

	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  %s(ret=%d)\n", func, ret_cd ));
	return( ret_cd );
}

/******************************************************************************
 * �֐���	dbg_set_tbl_all
 * �@�\		�f�o�b�O���x����S�ẴN���X�փZ�b�g����
 * �֐��l	�O�@�@�F�@����I��
 *		�|�P�@�F�@�G���[
 * ����
 ******************************************************************************/

static	int
dbg_set_tbl_all( level_str )
char	*level_str;
{
	int	ret_cd = OK;			/* ���^�[���@�R�[�h	*/
	char	level;
	int	idx;

	_printf(( "Enter %s(level=%s)\n", __FUNCTION__, level_str ));

	/********************/
	/* ���x���@�`�F�b�N */
	/********************/

	if ( level_str == NULL || level_str[0] == '\0' ) {
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : level OK !\n", func ));

	if ( (level = dbg_get_level(level_str)) == 0 ) {
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "      %s : dbg_get_level OK ! : level = %s\n",
		func, level_str ));

	/********************************/
	/* ���x�����e�[�u���փZ�b�g���� */
	/********************************/

	for ( idx = 0; idx < D_MAX_TBL; idx++ ) {
		_Dbg_tbl[idx] = level;
	}

	_printf(( "      %s : set level OK !\n", func ));


	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  %s(ret=%d)\n", func, ret_cd ));
	return( ret_cd );
}

/******************************************************************************
 * �֐���	dbg_get_class
 * �@�\		�N���X�e�[�u������������
 * �֐��l	�e�[�u���̃C���f�b�N�X
 *		�|�P�@�F�@�G���[
 * ����
 ******************************************************************************/

static	int
dbg_get_class( class )
char	*class;
{
	int	index = ERR;			/* ���^�[���@�R�[�h	*/
	neo_class_sym_t	*ptr;			/* �׽  ð���  �߲���	*/

	_printf(( "Enter %s(class=%s)\n", __FUNCTION__, class ));

	/******************/
	/* �G���[�`�F�b�N */
	/******************/

	if ( class == NULL ) {
		_printf(( "      %s : class ERR !\n", func ));
		goto	ret;
	}

	_printf(( "      %s : class OK !\n", func ));

	/************************/
	/* �N���X�e�[�u���@���� */
	/************************/

	for ( ptr = _neo_class_tbl; ptr->c_sym; ptr++ ) {

		if ( strcmp(ptr->c_sym, class) == 0 ) {
			index = ptr->c_class;
			goto	ret;
		}
	}

	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  %s(index=%d)\n", func, index ));
	return( index );
}

/******************************************************************************
 * �֐���	dbg_get_level
 * �@�\		���x���e�[�u�����������A���x���l���쐬����
 * �֐��l	���x���l
 *		�O�@�F�@�G���[
 * ����
 ******************************************************************************/

static	char
dbg_get_level( level_str )
char	*level_str;
{
	char	level = 0;			/* ���^�[���@���x��	*/
	neo_level_sym_t	*ptr;			/* ����  ð���  �߲���	*/
	int	size;
	int	idx;

	_printf(( "Enter %s(level=%s)\n", __FUNCTION__, level_str ));

	/******************/
	/* �G���[�`�F�b�N */
	/******************/

	if ( level_str == NULL ) {
		goto	ret;
	}

	_printf(( "      %s : level_str OK !\n", func ));

	/************************/
	/* ���x���e�[�u���@���� */
	/************************/

	size = strlen( level_str );

	for ( level = idx = 0; idx < size; idx++ ) {

		for ( ptr = _neo_level_tbl; ptr->c_sym; ptr++ ) {

			if ( ptr->c_sym == level_str[idx] ) {
				level |= ptr->c_level;
				_printf(( "      %s : search OK !\n", func ));
				break;
			}
		}
	}

	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  %s(level=%d)\n", func, level ));
	return( level );
}

/******************************************************************************
 * �֐���	dbg_open_file
 * �@�\		�o�̓t�@�C���̃I�[�v��
 * �֐��l	�O�@�@�F�@����I��
 *		�|�P�@�F�@�G���[
 * ����
 ******************************************************************************/

static	int
dbg_open_file( procname )
char	*procname;
{
	int	ret_cd = OK;			/* ���^�[���@�R�[�h	*/
	char	*dbg_dir;			/* �f�B�N�g���[		*/
	char	outfile[2048];			/* �o�̓t�@�C����	*/

	_printf(( "Enter %s(procname=%s)\n", __FUNCTION__, procname ));

	/**********************************/
	/* �f�o�b�O�@�f�B���N�g���[�@�擾 */
	/**********************************/

	dbg_dir = getenv( D_OPT_DIR );

	if ( dbg_dir == NULL ) {
		_printf(( "     %s : getenv(%s) = null\n", func, D_OPT_DIR ));
		dbg_dir = ".";
	}

	_printf(( "     %s : dbg_dir OK ! : dbg_dir = %s\n", func, dbg_dir ));

	/**************************/
	/* �o�̓t�@�C���@�I�[�v�� */
	/**************************/

	/* 1993�N06��11��(��) */
	sprintf(outfile, "%s/%s.%s", dbg_dir, D_PREFIX, procname );

	if ( ! (_Dbg_fp = fopen(outfile, "w")) ){
		_printf(( "     %s : fopen(%s) ERR !\n", func, outfile ));
		ret_cd = ERR;
		goto	ret;
	}

	_printf(( "     %s : fopen(%s) OK !\n", func, outfile ));
	setbuf( _Dbg_fp, NULL );

	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  %s(ret=%d)\n", func, ret_cd ));
	return( ret_cd );
}

/******************************************************************************
 * �֐���	dbg_write_header
 * �@�\		�w�b�_�[�̏�������
 * �֐��l	�Ȃ�
 * ����
 ******************************************************************************/

static	void
dbg_write_header( save_av )
char	*save_av;
{
	_printf(( "Enter %s(save_av=%s)\n", __FUNCTION__, save_av ));

	/**********************/
	/* �w�b�_�[�@�������� */
	/**********************/

	fprintf( _Dbg_fp, "%s : %s\n\n", dbg_get_time(), save_av );
	_printf(( "     %s : write header OK !\n", func ));

	/********/
	/* �I�� */
	/********/

	_printf(( "Exit  (void)%s\n", func ));
}

/******************************************************************************
 * �֐���	dbg_get_time
 * �@�\		�V�X�e�������𕶎���Ŏ擾����B
 * �֐��l	���ݎ��� (char*)
 * ����
 ******************************************************************************/

static	char*
dbg_get_time()
{
	time_t	time();
	char	*ctime();
	long	t;
	static char	buf[32];

	_printf(( "Enter %s(void)\n", __FUNCTION__ ));

	buf[0] = '\0';

	if ( time(&t) == -1 ) {
		_printf(( "     %s : time() ERR ! : err = %d\n", func, errno ));
		goto	ret;
	}

	strcpy(buf, ctime(&t));
	buf[strlen(buf) - 1] = 0;

	/********/
	/* �I�� */
	/********/
ret:
	_printf(( "Exit  %s(ret=%s)\n", func, buf ));
	return buf;
}

/******************************************************************************
 * �֐���	_dbg_err_printf
 * �@�\		�f�o�b�O�\��
 * �֐��l	�Ȃ�
 * ����
 ******************************************************************************/

extern	char*	neo_errsym();

extern	void
_dbg_err_printf( str )
char	*str;
{
	if (str && *str) {
		_dbg_printf( "%s : %s\n", str, neo_errsym() );
	} else {
		_dbg_printf( "%s\n", neo_errsym() );
	}
}

#ifdef	DEBUG

/******************************************************************************
 * �֐���	debug_level
 * �@�\		���x���e�[�u���@�f�o�b�O 
 * �֐��l	�Ȃ�
 * ����
 ******************************************************************************/

static	void
dbg_table()
{
	int	idx;
	char	classname[16];
	char	levelname[16];

	_printf(( "\n" ));
	_printf(( "/********************/\n" ));
	_printf(( "/* ���x���@�e�[�u�� */\n" ));
	_printf(( "/********************/\n" ));

	for ( idx = 0; idx < D_MAX_TBL; idx++ ) {

		if ( _Dbg_tbl[idx] != 0 ) {
			_dbg_find_classname( idx, classname );

			if ( classname[0] != '\0' ) {
				_dbg_find_levelname( _Dbg_tbl[idx], levelname );
				_printf(("%-16s  %s\n", classname, levelname));
			}
		}
	}
	_printf(( "/********************/\n\n" ));
}

static	void
dbg_find_classname( class, name )
int	class;
char	*name;
{
	neo_class_sym_t	*ptr;

	name[0] = '\0';

	for ( ptr = _neo_class_tbl; ptr->c_sym; ptr++ ) {

		if ( class == ptr->c_class ) {
			strcpy( name, ptr->c_sym );
			break;
		}
	}
}

static	void
dbg_find_levelname( level, name )
char	level;
char	*name;
{
	neo_level_sym_t	*ptr;
	char	*cp;

	cp = name;

	for ( ptr = _neo_level_tbl; ptr->c_sym; ptr++ ) {

		if ( level & ptr->c_level ) {
			*cp = ptr->c_sym;
			cp++;
		}
	}
	*cp = '\0';
}

#endif	// DEBUG
