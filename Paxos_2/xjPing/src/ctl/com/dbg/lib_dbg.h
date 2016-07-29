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

/************************************************************************/
/*									*/
/*									*/
/*	�@�\	�f�o�b�O�p�̏o�͂��s�Ȃ��B				*/
/*	����	�N���X�ƃ��x���ɂ��o�͂𐧌䂷��B			*/
/*									*/
/*	�֐�								*/
/*									*/
/************************************************************************/
#ifndef	_LIB_DBG_
#define	_LIB_DBG_

#include "NWGadget.h"

#include "neo_class.h"
#include "neo_debug.h"

/**************/
/* �o�̓��x�� */
/**************/

typedef	struct	neo_level_sym		/* ���x���E�V���{���@�\����	*/
{
	char	c_level;		/* �o�̓��x���ԍ�		*/
	char	c_sym;			/* �o�̓��x��������		*/
} neo_level_sym_t;

neo_level_sym_t	_neo_level_tbl[] = {	/* ���x���E�V���{���@�e�[�u��	*/

	{ _LEV_A,	'A'  },		/* �o�̓��x���@�`		*/
	{ _LEV_B,	'B'  },		/* �o�̓��x���@�a		*/
	{ _LEV_C,	'C'  },		/* �o�̓��x���@�b		*/
	{ _LEV_T,	'T'  },		/* �o�̓��x���@�g���[�X		*/
	{ '\0',		'\0' }
};

/**************/
/* �I�v�V���� */
/**************/

#define	D_PREFIX	"DBG"		/* �o�̓t�@�C���̃v���t�B�b�N�X	*/
#define	D_OPT_DIR	"NEO_DBGDIR"	/* ���ޯ��̧�ٗp�f�B���N�g���[	*/
#define	D_OPT_ENV	"NEO_DBGOPT"	/* ���ϐ�����̃I�v�V�����w��	*/
#define	D_OPT_ARG	"-d" 		/* ��������̃I�v�V�����w��	*/
#define	D_OPT_ALL	"ALL"		/* �S�N���X�w��I�v�V����	*/
#define	D_OPT_DEL	","		/* �I�v�V�����p�f���~�b�^�[	*/
#define	D_OPT_LEV	'='		/* ���x���p�f���~�b�^�[		*/

#define	OK		0
#define	ERR		-1

#endif	// _LIB_DBG_
/************************************************************************/
/*			End of the Header				*/
/************************************************************************/
