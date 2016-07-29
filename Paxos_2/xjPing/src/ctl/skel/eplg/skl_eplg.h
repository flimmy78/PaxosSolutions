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
 *	skl_eplg.h
 *
 *	����	�G�s���[�O�����i�o�^�^�폜�j
 *
 ******************************************************************************/

#include	"neo_dlnk.h"
#include	"neo_skl.h"
#include	"neo_system.h"

/*
 *	�G�s���[�O�����Ǘ��\����
 */
typedef struct _neo_epilog {
        _dlnk_t		lnk;		/* �o���������N	*/
	int		class;		/* �N���X	*/
	int		(*func)();	/* �����֐�	*/
} _neo_epilog_t;


void	_neo_init_epilog();
void	_neo_exec_epilog();
