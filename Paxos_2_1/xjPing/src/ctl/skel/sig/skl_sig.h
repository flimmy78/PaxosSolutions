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
 *	skl_sig.h
 *
 *	����	�X�P���g�� �V�O�i������
 *
 ******************************************************************************/
#include	<signal.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<sys/resource.h>
#include	"neo_skl.h"
#include	"neo_system.h"


/*
 *	�V�O�i������ �Ǘ��\����
 */
typedef struct _neo_signal {
	int	sig;			/* �V�O�i���ԍ�(UNIX)		*/
	int	neo_sig;		/* �V�O�i���ԍ�(NEO)		*/
	int	set_flag;		/* �V�O�i���n���h���o�^�t���O	*/
	void	(*_neo_hndl)();		/* �V�O�i���n���h��		*/
	int	(*_neo_prhndl)();	/* �V�O�i���n���h��(�O����)	*/
	int	(*_neo_pthndl)();	/* �V�O�i���n���h��(�㏈��)	*/
	void	(*_usr_hndl)();		/* �V�O�i���n���h��(���[�U��`)	*/
} _neo_signal_t;


/*
 *	�V�O�i���}�X�N �Ǘ��\����
 */
typedef struct _neo_sigmask {
	int	neo_sig;		/* �V�O�i���ԍ�(NEO)	*/
	int	mask;			/* �V�O�i���}�X�N	*/
} _neo_sigmask_t;


extern void	_neo_epilogue();
extern void	_neo_child_remove();
extern void	_neo_isterm_set();
extern void	_hndl_itm();

static void	_hndl_dfl();
static void	_hndl_cld();
static int	_prhndl_trm();
static int	_prhndl_cld();
static int	_pthndl_dfl();
static int	_pthndl_flt();
static _neo_signal_t	*_tbl_search();
static void	_neo_sigmask_init();

void		_neo_init_signal();
void		_neo_sigflag_init();
void		_neo_sigflag_set();
int		_neo_sigflag_get();
