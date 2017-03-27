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
 *	neo_errno.h
 *
 *	����	�G���[�ԍ�
 *
 ******************************************************************************/
#ifndef _NEO_ERRNO_
#define _NEO_ERRNO_

#include	<sys/types.h>
#include	<sys/errno.h>

/*
 *	�m�d�n �e�N���X �G���[�ԍ�
 */
#include	"neo_unix_err.h"
#include	"neo_link_err.h"
#include	"neo_skl_err.h"
#include	"neo_db_err.h"

#include	"xj_sql_err.h"

/*
 *	�m�d�n �N���X�擾�}�N��
 */
#define	NEO_ERRCLASS(n)	(((n)>>24)&0xff)

/*
 *	�m�d�n �G���[�V���{���擾
 */
extern	char	*neo_errsym(void);

/*
 *	�t�m�h�w to �m�d�n �G���[�ԍ��ϊ�
 */
extern	int	unixerr2neo(void);

/*
 *	�m�d�n to �t�m�h�w �G���[�ԍ��ϊ�
 */
extern	int	neoerr2unix(void);

extern	int	neo_errno;
//extern	int	errno;

#endif  /* _NEO_ERRNO_ */


