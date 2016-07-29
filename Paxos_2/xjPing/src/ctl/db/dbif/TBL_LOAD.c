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
 *	TBL_LOAD.c
 *
 *	����		�f�[�^���[�h�ƃX�g�A 
 *
 ******************************************************************************/

#include	"TBL_RDB.h"

/*****************************************************************************
 *
 *  �֐���	TBL_LOAD()
 *
 *  �@�\	���R�[�h���t�@�C�����烍�[�h���� 
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
TBL_LOAD( td, s, n, bufp )
	r_tbl_t		*td;		/* �e�[�u���L�q�q		*/
	int		s;		/* ���[�h���郌�R�[�h�̊J�n�ԍ� */
	int		n;		/* ���[�h���郌�R�[�h�̐�	*/
	char		*bufp;		/* ���R�[�h�i�[�p�G���A		*/
{

DBG_ENT(M_RDB,"TBL_LOAD");
DBG_A(("s=%d,n=%d\n", s, n ));

	if( (neo_errno = my_tbl_load( TDTOMY( td ), s, n, bufp )) )
		goto err1;

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(-1);
	return( -1 );
}


/*****************************************************************************
 *
 *  �֐���	TBL_STORE()
 *
 *  �@�\	���R�[�h���t�@�C���փX�g�A���� 
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	-1
 *
 ******************************************************************************/
int
TBL_STORE( td, s, n, bufp )
	r_tbl_t		*td;
	int		s, n;
	char		*bufp;
{
DBG_ENT(M_RDB,"TBL_STORE");
DBG_A(("s=%d,n=%d\n", s, n ));

	if( (neo_errno = my_tbl_store( TDTOMY( td ), s, n, bufp )) )
		goto err1;

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(-1);
	return( -1 );
}
