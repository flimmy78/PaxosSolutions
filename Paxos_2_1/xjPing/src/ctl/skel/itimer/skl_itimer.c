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
 *	skl_itimer.c
 *
 *	����	�X�P���g�� �C���^�o���^�C�}
 * 
 ******************************************************************************/

#include	"skl_itimer.h"


/* 
 *	�C���^�o���^�C�}�Ǘ����X�g �G���g��
 */
static _dlnk_t	_itimer_entry;


/******************************************************************************
 * �֐���
 *		_neo_init_itimer()
 * �@�\        
 *		�C���^�o���^�C�} ������
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_init_itimer()
{
DBG_ENT( M_SKL, "_neo_init_itimer" );

	_dl_init( &_itimer_entry );

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		neo_setitimer()
 * �@�\        
 *		�C���^�o���^�C�} �o�^
 * �֐��l
 *      ����: !NULL (�C���^�o���^�C�}�ԍ�)
 *      �ُ�: NULL
 * ����
 ******************************************************************************/
void *
neo_setitimer( func, arg, msec, mode )
void	(*func)();	/* I ���[�U�o�^�֐�		*/
void	*arg;		/* I ���[�U�o�^�֐����s���̈���	*/
long	msec;		/* I �C���^�o���l		*/
int	mode;		/* I �N�����[�h			*/
{
	_neo_itimer_t	*newt;
	long		sec, usec;

DBG_ENT( M_SKL, "neo_setitimer" );
DBG_C(( "func(%#x) arg(%#x) msec(%d) mode(%d)\n", func, arg, msec, mode ));

	/*
	 *	�����`�F�b�N
	 */
	if( (msec <= 0) || ((mode != NEO_ONESHOT) && (mode != NEO_CYCLE)) ) {
		neo_errno = E_SKL_INVARG;
		goto err;
	}

	if( !(newt = (_neo_itimer_t *)neo_malloc( sizeof(_neo_itimer_t) )) ) {
		goto err;
	}

	/*
	 *	���s���̃^�C�}��~
	 */
	if( _dl_head(&_itimer_entry) ) {
		_it_timer( 0, 0, 0, 0 );
	}

	/*
	 *	�o�^���ݒ�
	 */
	msec = ( (msec + 99)/100*100 );
	sec  = msec/1000;
	usec = msec - sec*1000;

	newt->id		= (void *)newt;
	newt->mode		= mode;
	newt->itime.tv_sec	= sec;
	newt->itime.tv_usec	= usec;
	newt->func		= func;
	newt->arg		= arg;

	/*
	 *	�^�C�}�Ǘ��\���̂̓o�^
	 */
	_it_entlist( newt );

	/*
	 *	�^�C�}���s
	 */
	_it_alarm();

DBG_EXIT(newt);
	return( (void *)newt );
err:;
DBG_EXIT(0);
	return( (void *)0 );
}


/******************************************************************************
 * �֐���
 *		neo_delitimer()
 * �@�\        
 *		�C���^�o���^�C�} �폜
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_delitimer( id )
void	*id;	/* I �C���^�o���^�C�}�ԍ�	*/
{
	int	ret = -1;

DBG_ENT( M_SKL, "neo_delitimer" );
DBG_C(( "id(%#x) \n", id ));

	/*
	 *	�����`�F�b�N
	 */
	if( _dl_head(&_itimer_entry) ) {
		/*
		 *	���s���̃^�C�}��~
		 */
		_it_timer( 0, 0, 0, 0 );
	} else {
		neo_errno = E_SKL_INVARG;
		goto end;
	}

	/*
	 *	�^�C�}�Ǘ��\���̂̍폜
	 */
	ret = _it_delist( (_neo_itimer_t *)id );

	/*
	 *	�^�C�}���s
	 */
	_it_alarm();

end:;
DBG_EXIT(ret);
	return( ret );
}

/******************************************************************************
 * �֐���
 *		neo_chgitimer()
 * �@�\        
 *		�C���^�o���^�C�} �C���^�o���l�̕ύX
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_chgitimer( pt, msec )
_neo_itimer_t	*pt;	/* I �C���^�o���^�C�}�ԍ�	*/
long		msec;	/* I �C���^�o���l		*/
{
	int	before;
	long	sec, usec;

	DBG_ENT( M_SKL, "neo_chgitimer" );
	DBG_B(( "### pt(%#x),msec(%ld)\n", pt,msec ));

	before = sigblock( sigmask(SIGALRM) );

	/*
	 *	�����`�F�b�N
	 */
	if( (pt->mode != NEO_CYCLE) || (msec <= 0) || (pt->id != (void*)pt)) {
		neo_errno = E_SKL_INVARG;
		goto err;
	}

	/*
	 *	�o�^���ݒ�
	 */
	msec = ( (msec + 99)/100*100 );
	sec  = msec/1000;
	usec = msec - sec*1000;

	pt->itime.tv_sec	= sec;
	pt->itime.tv_usec	= usec;

	sigsetmask(before);
	DBG_EXIT(0);
	return( 0 );

err:;
	sigsetmask(before);
	DBG_EXIT(-1);
	return( -1 );
}

/******************************************************************************
 * �֐���
 *		neo_sleep()
 * �@�\        
 *		�X���[�v
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_sleep( sec )
unsigned int	sec;	/* I �X���[�v�b��	*/
{
DBG_ENT( M_SKL, "neo_sleep" );
DBG_C(( "sec(%d) \n", sec ));

	/*
	 *	�V�O�i���t���O�̏�����
	 */
	_neo_sigflag_init();

	/*
	 *	�^�C�}���s
	 */
	if( neo_setitimer( _skl_sleep, 0, (sec * 1000), NEO_ONESHOT ) == 0 ) {
		goto err;
	}

	while( 1 ) {
		pause();
		if( _neo_sigflag_get() )
			break;
	}


DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}


/******************************************************************************
 * �֐���
 *		_skl_sleep()
 * �@�\        
 *		�V�O�i���t���O(SIGALRM)�̐ݒ�
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static void
_skl_sleep()
{
DBG_ENT( M_SKL, "_it_sleep" );

	_neo_sigflag_set( SIGALRM );

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_hndl_itm()
 * �@�\        
 *		�V�O�i���n���h��(SIGALRM)
 * �֐��l
 *	����
 * ����
 *	  ���[�U��`�����֐��̒���, �C���^�o���^�C�}�̍폜���s�Ȃ�ꂽ��,
 *	���X�g�̐擪���ς���Ă���\�������邽�߁A���[�U��`�����֐���
 *	���s��A������x���X�g�̐擪�����o���B
 *	  ���̒l�����āA�o�^�����o�̍폜�^�ēo�^���s�Ȃ�
 ******************************************************************************/
void
_hndl_itm()
{
	_neo_itimer_t	*pt, *pt2;
#ifdef CHKTEST
static int prev = 0;
int s,t;
#endif
DBG_ENT( M_SKL, "_hndl_itm" );
#ifdef CHKTEST
t = time(0);
if( (s = t - prev) != 72 )
	DBG_A(("**************** HANDLER in(%d) from(%d) = (%d)\n",t,prev,s));
prev = t;
#endif

	if( !(pt = (_neo_itimer_t *)_dl_head(&_itimer_entry)) ) {
		goto end;
	}

	/*
	 *	���[�U��`�����֐�
	 */
	(*pt->func)(pt->arg);

	if( !(pt2 = (_neo_itimer_t *)_dl_head(&_itimer_entry)) ) {
		goto end;
	}

	/*
	 *	��񂾂����s
	 */
	if( pt == pt2 && pt->mode == NEO_ONESHOT ) {
		_it_delist( pt );

	/*
	 *	�J��Ԃ����s
	 */
	} else if( pt == pt2 && pt->mode == NEO_CYCLE ) {
		_dl_remove( pt );
		_it_entlist( pt );
	}

	/*
	 *	�^�C�}���s
	 */
	_it_alarm();

end:;
DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_it_entlist()
 * �@�\        
 *		�V�O�i���\���̂̃��X�g�ւ̓o�^
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
static int
_it_entlist( newt )
_neo_itimer_t	*newt;	/* I �V�O�i���\����	*/
{
	struct timeval	tp;
	struct timezone	tzp;
	long		sec, usec;
	_neo_itimer_t	*pt;

DBG_ENT( M_SKL, "_it_entlist" );

	/*
	 *	���ݎ����̎擾
	 */
	gettimeofday(&tp, &tzp);


	/*
	 *	�N�������̌v�Z
	 */
	usec = tp.tv_usec + newt->itime.tv_usec;

	if( usec/1000000 ) {
		usec = usec - 1000000;
		sec = tp.tv_sec + newt->itime.tv_sec + 1;
	} else {
		sec = tp.tv_sec + newt->itime.tv_sec;
	}

	newt->stime.tv_sec	= sec;
	newt->stime.tv_usec	= usec;


	/*
	 *	���X�g�ւ̓o�^
	 */
	for( pt = (_neo_itimer_t *)_dl_head( &_itimer_entry ); 
			_dl_valid( &_itimer_entry, pt );
			pt = (_neo_itimer_t *)_dl_next( pt ) ) {

		if( (pt->stime.tv_sec > sec) || 
		    ((pt->stime.tv_sec == sec) && (pt->stime.tv_usec > usec)) ){
			_dl_insert( newt, pt );
			goto ok;
		}
	}

	_dl_insert( newt, &_itimer_entry );

ok:;
DBG_EXIT(0);
	return( 0 );
}


/******************************************************************************
 * �֐���
 *		_it_dellist()
 * �@�\        
 *		�V�O�i���\���̂̃��X�g����̍폜
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
static int	
_it_delist( del )
_neo_itimer_t	*del;	/* I �V�O�i���\����	*/
{
	_neo_itimer_t		*pt;

DBG_ENT( M_SKL, "_it_delist" );

	for( pt = (_neo_itimer_t *)_dl_head( &_itimer_entry ); 
			_dl_valid( &_itimer_entry, pt ); 
			pt = (_neo_itimer_t *)_dl_next( pt ) ) {

		if( pt == del ) {
			_dl_remove( pt );
			pt->mode = -1;
			neo_free( pt );
			goto ok;
		}
	}

	neo_errno = E_SKL_INVARG;
	goto err;

ok:;
DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}


/******************************************************************************
 * �֐���
 *		_it_alarm()
 * �@�\        
 *		�C���^�o���^�C�}�̔��s
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
static int	
_it_alarm()
{
	_neo_itimer_t		*pt;
	struct timeval		tp;
	struct timezone		tzp;
	long			ssec, susec;

DBG_ENT( M_SKL, "_it_alarm" );

	if( !(pt = (_neo_itimer_t *)_dl_head( &_itimer_entry )) ) {
		goto err;
	}

	/*
	 *	���ݎ����̎擾
	 */
	gettimeofday(&tp, &tzp);


	/*
	 *	�N�������̌v�Z
	 */
	ssec  = pt->stime.tv_sec - tp.tv_sec;
	susec = pt->stime.tv_usec - tp.tv_usec;


	/*
	 *	 ���ݎ����� �N�������� �߂��Ă��鎞
	 */
	if( ssec < 0 || ( ssec == 0 && susec <= 0 ) ) {
		DBG_A(("next handler time-over(%d.%d)(%d)\n",ssec,susec,time(0)));
		_hndl_itm();

	/*
	 *	 �N�������� ���ݎ������ �x����
	 */
	} else {
		if( susec < 0 ) {
			susec += 1000000;
			ssec--;
		}
		DBG_A(("next handler (%d.%d)(%d)\n",ssec,susec,time(0)));
		_it_timer( 0, 0, ssec, susec );
	}


DBG_EXIT(0);
	return( 0 );
err:;
DBG_EXIT(-1);
	return( -1 );
}


/******************************************************************************
 * �֐���
 *		_it_timer()
 * �@�\        
 *		setitimer() �̔��s
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
static int
_it_timer( its, itus, vls, vlus )
long	its;	/* �^�C�}�̊����������Ƃ��̍ă��[�h�Ɏg�p����l(�b)	*/
long	itus;	/* �^�C�}�̊����������Ƃ��̍ă��[�h�Ɏg�p����l(�}�C�N���b) */
long	vls;	/* ���̃^�C�}�����܂ł̎���(�b)		*/
long	vlus;	/* ���̃^�C�}�����܂ł̎���(�}�C�N���b)	*/
{
	struct itimerval	vl, ovl;

DBG_ENT( M_SKL, "_it_timer" );
DBG_C(( "interval_sec(%d) interval_usec(%d)\n", vls, vlus ));

	vl.it_interval.tv_sec   = its;
	vl.it_interval.tv_usec  = itus;
	vl.it_value.tv_sec      = vls;
	vl.it_value.tv_usec     = vlus;

	setitimer( ITIMER_REAL, &vl, &ovl );

DBG_EXIT(0);
	return( 0 );
}
