/*******************************************************************************
 * 
 *  skl_sig.c --- xjPing (On Memory DB) skelton (signal Handring) Library programs
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

#include	"skl_sig.h"

#ifdef DEBUG
extern void neo_exit();
#endif

/*
 *	�V�O�i������ �Ǘ��e�[�u��
 */
static _neo_signal_t	_sig_tbl[] = {
{SIGHUP,    0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
#ifdef DEBUG
{SIGINT,    NEO_SIGINT,	  1, _hndl_dfl,(int(*)())neo_exit,_pthndl_dfl, 0},
#else
{SIGINT,    NEO_SIGINT,	  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
#endif
{SIGQUIT,   NEO_SIGINT,	  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGILL,    NEO_SIGFAULT, 1, _hndl_dfl, 0,           _pthndl_flt, 0},
{SIGTRAP,   0,            0, 0,		0,           0,		  0},
{SIGABRT,   NEO_SIGINT,	  0, _hndl_dfl, 0,           _pthndl_dfl, 0},
/*{SIGEMT,    NEO_SIGFAULT, 1, _hndl_dfl, 0,           _pthndl_flt, 0},*/
{SIGFPE,    NEO_SIGFAULT, 1, _hndl_dfl, 0,           _pthndl_flt, 0},
{SIGKILL,   0,            0, 0,		0,           0,		  0},
{SIGBUS,    NEO_SIGFAULT, 1, _hndl_dfl, 0,           _pthndl_flt, 0},
{SIGSEGV,   0,            0, 0,         0,           0,           0},
{SIGSYS,    NEO_SIGFAULT, 1, _hndl_dfl, 0,           _pthndl_flt, 0},
{SIGPIPE,   NEO_SIGPIPE,  1, _hndl_dfl,	0,           _pthndl_dfl, 0},
{SIGALRM,   0,            1, _hndl_itm,	0,           0,		  0},
{SIGTERM,   NEO_SIGTERM,  1, _hndl_dfl,	_prhndl_trm, _pthndl_dfl, 0},
{SIGURG,    0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGSTOP,   0,            0, 0,		0,           0,		  0},

{SIGTSTP,   0,         	  0, 0,         0,           0,           0},
{SIGCONT,   0,         	  0, 0,         0,           0,           0},

{SIGCLD,    NEO_SIGCLD,   1, _hndl_cld,	_prhndl_cld, _pthndl_dfl, 0},
{SIGTTIN,   0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGTTOU,   0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGIO,     0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGXCPU,   0,            0, 0,		0,           0,		  0},
{SIGXFSZ,   0,            0, 0,		0,           0,		  0},
{SIGVTALRM, 0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGPROF,   0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGWINCH,  0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGUSR1,   NEO_SIGUSR1,  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
{SIGUSR2,   NEO_SIGUSR2,  1, _hndl_dfl, 0,           _pthndl_dfl, 0},

#ifdef SIGLOST
{SIGLOST,   NEO_SIGFAULT, 1, _hndl_dfl, 0,           _pthndl_flt, 0},
#endif
#ifdef SIGPWR
{SIGPWR,    0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
#endif
#ifdef SIGWAITING
{SIGWAITING,0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
#endif
#ifdef SIGLWP
{SIGLWP,    0,		  1, _hndl_dfl, 0,           _pthndl_dfl, 0},
#endif

{0, 0, 0, 0, 0, 0, 0},
};


/*
 *	�V�O�i���}�X�N �Ǘ��e�[�u��
 */
static _neo_sigmask_t	_sig_mask[] = {
{0, 0},
{NEO_SIGFAULT, 0},
{NEO_SIGINT,   0},
{NEO_SIGPIPE,  0},
{NEO_SIGTERM,  0},
{NEO_SIGCLD,   0},
{NEO_SIGUSR1,  0},
{NEO_SIGUSR2,  0},
{0, 0},
};

/*
 *	�V�O�i���t���O
 */
static int	_signal_flag;

/*
 *	�V�O�i���}�X�N
 */
static int	_signal_mask;
static int	_signal_pending;

#undef sigmask
/********* undef sigmask �͉��L�̎���ɂ��
	/usr/include/signal.h �̒���
	#ifdef __USE_BSD
	/ * Compute mask for signal SIG.  * /
	# define sigmask(sig)   __sigmask(sig)
    ......................................
	#endif / * Use BSD.  * /
�ƒ�`����Ă��邽��
************************* ********************/

int
sigmask( sig )
	int	sig;
{
DBG_ENT( M_SKL, "sigmask" );
	return( 1<<sig );
}

int
sigblock( mask )
	int	mask;
{
	int 	prev;

DBG_ENT( M_SKL, "sigblock" );
	prev = _signal_mask;

	_signal_mask |= mask;

DBG_EXIT( prev );
	return( prev );
}

int
sigsetmask( mask )
	int	mask;
{
	int 	prev;
	int pended;
	_neo_signal_t	*ps;

DBG_ENT( M_SKL, "sigsetmask" );

	prev = _signal_mask;
	_signal_mask = mask;

	pended = _signal_pending & ~mask;
	if( pended ) {
		for( ps = _sig_tbl; ps->neo_sig; ps++ ) {
			if( pended & _sig_mask[ps->neo_sig].mask ) {

				(*ps->_neo_hndl)( ps->sig );

				_signal_pending 
					&= ~_sig_mask[ps->neo_sig].mask;
			}
		}
	}
DBG_EXIT( prev );
	return( prev );
}


/******************************************************************************
 * �֐���
 *		_neo_init_signal()
 * �@�\        
 *		�V�O�i������ ������
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_init_signal()
{
	_neo_signal_t	*ps;

DBG_ENT( M_SKL, "_neo_init_signal" );

	for( ps = _sig_tbl; ps->sig; ps++ ) {

		/*
		 *	�V�O�i���n���h�� �o�^
		 */
		if( ps->set_flag )
			signal( ps->sig, ps->_neo_hndl );

		/*
		 *	�V�O�i���}�X�N �o�^
		 */
		if( ps->neo_sig )
			_sig_mask[ps->neo_sig].mask |= sigmask(ps->sig);
	}

	/*
	 *	�V�O�i���t���O ������
	 */
	_neo_sigflag_init();

	/*
	 *	�V�O�i���}�X�N ������
	 */
	_neo_sigmask_init();

/***
	siginterrupt( SIGTERM, 1 );
***/

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		neo_sighold()
 * �@�\        
 *		�V�O�i���̃z�[���h
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_sighold( neo_sig )
int	neo_sig;	/* I �V�O�i���ԍ�(NEO)	*/
{
	int	ret = 0;

DBG_ENT( M_SKL, "neo_sighold" );

	/*
	 *	�����`�F�b�N
	 */
	if( neo_sig <= 0 || neo_sig > NEO_SIGUSR2 ) {
		neo_errno = E_SKL_INVARG;
		goto err;
	}

	/*
	 *	�V�O�i���u���b�N
	 */
	sigblock( _sig_mask[neo_sig].mask );

end:;
DBG_EXIT(ret);
	return( ret );
err:;
	ret = -1;
	goto end;
}

/******************************************************************************
 * �֐���
 *		neo_sigrelse()
 * �@�\        
 *		�V�O�i���̃����[�X
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 ******************************************************************************/
int
neo_sigrelse( neo_sig )
int	neo_sig;	/* I �V�O�i���ԍ�(NEO)	*/
{
	int	ret = 0;

DBG_ENT( M_SKL, "neo_sigrelse" );

	/*
	 *	�����`�F�b�N
	 */
	if( neo_sig <= 0 || neo_sig > NEO_SIGUSR2 ) {
		neo_errno = E_SKL_INVARG;
		goto err;
	}

	_signal_mask &= ~_sig_mask[neo_sig].mask;

end:;
DBG_EXIT(ret);
	return( ret );
err:;
	ret = -1;
	goto end;
}


/******************************************************************************
 * �֐���
 *		neo_signal()
 * �@�\        
 *		�V�O�i������ �o�^�^�폜
 * �֐��l
 *      ����: !-1 (�ȑO�̓o�^�֐�)
 *      �ُ�: -1
 * ����
 ******************************************************************************/
void
(*neo_signal( neo_sig, func ))()
int	neo_sig;	/* I �V�O�i���ԍ�(NEO)	*/
void	(*func)();	/* I ���[�U��`�֐� �A�h���X	*/
{
	int		before;
	_neo_signal_t	*ps;
	void		(*prefunc)() = (void (*)())-1;

DBG_ENT( M_SKL, "neo_signal" );
DBG_C(( "neo_sig=%d, func=%#x\n", neo_sig, func ));

	/*
	 *	�����`�F�b�N
	 */
	if( neo_sig <= 0 || neo_sig > NEO_SIGUSR2 ) {
		neo_errno = E_SKL_INVARG;
		goto end;
	}

	/*
	 *	�V�O�i���u���b�N
	 */
	before = sigblock( _sig_mask[neo_sig].mask );

	/*
	 *	�o�^
	 */
	for( ps = _sig_tbl; ps->sig; ps++ ) {

		if( (ps->neo_sig) && (ps->neo_sig == neo_sig) ) {
			prefunc = ps->_usr_hndl;
			ps->_usr_hndl = func;
		}
	}

	/*
	 *	�u���b�N����
	 */
	sigsetmask( before );

end:;
DBG_EXIT(prefunc);
	return( prefunc );
}


/******************************************************************************
 * �֐���
 *		_hndl_dfl()
 * �@�\        
 *		�V�O�i���n���h�� �i�f�t�H���g�j
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static void
_hndl_dfl( sig, code, scp, addr )
int	sig;		/* I �V�O�i���ԍ�	*/
int	code;		/* I �p�����[�^	*/
struct sigcontext *scp;	/* I �R���e�L�X�g�𕜌����邽�߂ɗp����\���� */
char	*addr;		/* I �A�h���X�Ɋւ���t�����	*/
{
	_neo_signal_t	*ps;

DBG_ENT( M_SKL, "_hndl_dfl" );
DBG_C(( "signal no=%d \n", sig ));

	/*
	 *	�\���̂̌���
	 */
	ps = _tbl_search( sig );
	
	if( _sig_mask[ps->neo_sig].mask & _signal_mask ) {

		_signal_pending |= _sig_mask[ps->neo_sig].mask;

		return;
	}
	/*
	 *	�O����
	 */
	if( ps->_neo_prhndl )
		(*ps->_neo_prhndl)();

	/*
	 *	���[�U��`����
	 */
	if( ps->_usr_hndl )
		(*ps->_usr_hndl)( ps->neo_sig );

	/*
	 *	�㏈��
	 */
	if( ps->_neo_pthndl )
		(*ps->_neo_pthndl)( sig );

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_hndl_cld()
 * �@�\        
 *		�V�O�i���n���h�� �iSIGCLD�j
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static void
_hndl_cld( sig, code, scp, addr )
int	sig;		/* I �V�O�i���ԍ�	*/
int	code;		/* I �p�����[�^	*/
struct sigcontext *scp;	/* I �R���e�L�X�g�𕜌����邽�߂ɗp����\���� */
char	*addr;		/* I �A�h���X�Ɋւ���t�����	*/
{
	_neo_signal_t	*ps;
	int		ret, pid, status;

DBG_ENT( M_SKL, "_hndl_cld" );
DBG_C(( "signal no=%d \n", sig ));

	/*
	 *	�\���̂̌���
	 */
	ps = _tbl_search( sig );
	
	if( _sig_mask[ps->neo_sig].mask & _signal_mask ) {

		_signal_pending |= _sig_mask[ps->neo_sig].mask;

		return;
	}

	while( 1 ) {
		/*
		 *	�O����
		 */
		ret = (*ps->_neo_prhndl)( &pid, &status );

		switch( ret ) {
		case -1:
		case  0:
			goto post;

		default:
DBG_C(( "Die child process pid=%d, status=%#x \n", pid, status ));
			/*
			 *	���[�U��`����
			 */
			if( ps->_usr_hndl ) {
				(*ps->_usr_hndl)( ps->neo_sig, pid, status );
			}
			continue;
		}
	}
post:;
	/*
	 *	�㏈��
	 */
	if( ps->_neo_pthndl )
		(*ps->_neo_pthndl)( sig );

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_prhndl_trm()
 * �@�\        
 *		�V�O�i���n���h�� �O�����iSIGTERM)
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static int
_prhndl_trm()
{
DBG_ENT( M_SKL, "_prhndl_trm" );

	_neo_isterm_set();

DBG_EXIT(0);
	return( 0 );
}


/******************************************************************************
 * �֐���
 *		_prhndl_cld()
 * �@�\        
 *		�V�O�i���n���h�� �O�����iSIGCLD)
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static int
_prhndl_cld( pid, status )
int	*pid;		/* O �v���Z�X����	*/
int	*status;	/* O �I���X�e�[�^�X	*/
{
DBG_ENT( M_SKL, "_prhndl_cld" );

	while( 1 ) {
		*pid = wait3( status, WNOHANG, 0 );

		switch( *pid ) {
		/*
		 *	�G���[
		 */
		case -1:
			if( errno == EINTR )
				continue;
			goto end;

		/*
		 *	�I���q�v���Z�X����
		 */
		case  0:
			goto end;

		/*
		 *	�I���q�v���Z�X�L��
		 */
		default:
			_neo_child_remove( *pid );
			goto end;
		}
	}
end:;
DBG_EXIT(*pid);
	return( *pid );
}


/******************************************************************************
 * �֐���
 *		_pthndl_dfl()
 * �@�\        
 *		�V�O�i���n���h�� �㏈���i�f�t�H���g)
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static int
_pthndl_dfl( sig )
int	sig;	/* I �V�O�i���ԍ�	*/
{
DBG_ENT( M_SKL, "_pthndl_dfl" );

	_neo_sigflag_set( sig );

DBG_EXIT(sig);
	return( 0 );
}


/******************************************************************************
 * �֐���
 *		_pthndl_flt()
 * �@�\        
 *		�V�O�i���n���h�� �㏈���iNEO_SIGFAULT)
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static int
_pthndl_flt( sig )
int	sig;	/* I �V�O�i���ԍ�	*/
{
DBG_ENT( M_SKL, "_pthndl_flt" );
DBG_C(( "signal no=%d \n", sig ));

	_neo_epilogue( 1 );

	/*
	 *	�R�A�_���v
	 */
	signal( sig, SIG_DFL );
	kill( getpid(), sig );

DBG_EXIT(1);
	return( 1 );
}


/******************************************************************************
 * �֐���
 *		_tbl_search()
 * �@�\        
 *		�V�O�i���Ǘ��\���� ����
 * �֐��l
 *	�V�O�i���Ǘ��\���̂ւ̃|�C���^
 * ����
 ******************************************************************************/
static	_neo_signal_t *
_tbl_search( sig )
int	sig;	/* I �V�O�i���ԍ�	*/
{
	_neo_signal_t	*ps;

DBG_ENT( M_SKL, "_tbl_search" );
DBG_C(( "signal no=%d \n", sig ));

	for( ps = _sig_tbl; ps->sig; ps++ ) {
		if( sig == ps->sig )
			break;
	}

DBG_EXIT(ps);
	return( ps );
}


/******************************************************************************
 * �֐���
 *		_neo_sigflag_set()
 * �@�\        
 *		�V�O�i���t���O �ݒ�
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_sigflag_set( sig )
int	sig;	/* I �V�O�i���ԍ�	*/
{
DBG_ENT( M_SKL, "_neo_sigflag_set" );
DBG_C(( "signal no=%d \n", sig ));

	_signal_flag = sig;

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_neo_sigflag_init()
 * �@�\        
 *		�V�O�i���t���O ������
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
void
_neo_sigflag_init()
{
DBG_ENT( M_SKL, "_neo_sigflag_init" );

	_signal_flag = 0;

DBG_EXIT(0);
}


/******************************************************************************
 * �֐���
 *		_neo_sigflag_get()
 * �@�\        
 *		�V�O�i���t���O �擾
 * �֐��l
 *	�V�O�i���t���O �ԍ�
 * ����
 ******************************************************************************/
int
_neo_sigflag_get()
{
DBG_ENT( M_SKL, "_neo_sigflag_get" );
DBG_EXIT(_signal_flag);

	return( _signal_flag );
}


/******************************************************************************
 * �֐���
 *		_neo_sigmask_init()
 * �@�\        
 *		�V�O�i���}�X�N ������
 * �֐��l
 *	����
 * ����
 ******************************************************************************/
static void
_neo_sigmask_init()
{
DBG_ENT( M_SKL, "_neo_sigmask_init" );

	_signal_mask = 0;

DBG_EXIT(0);
}
