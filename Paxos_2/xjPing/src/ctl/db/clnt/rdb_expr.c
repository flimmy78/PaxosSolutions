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
 *		rdb_expr.c
 *
 *	����	�\����͖ؗp �T�u���[�`��
 * 
 ******************************************************************************/

#include	"neo_db.h"
#include	"y.tab.h"

r_expr_t	*_e_alloc();
int			_e_free();
int			rdb_item_value();

/******************************************************************************
 * �֐���
 *		rdb_string_op()
 * �@�\        
 *		������f�[�^�p�� ��͖�LEAF �쐬
 * �֐��l
 *      ����: ��͖؂̃m�[�h
 *      �ُ�: 0
 * ����
 ******************************************************************************/
r_expr_t	*
rdb_string_op( item, op, str )
	char	*item;	/* �A�C�e����	*/
	long long	op;	/* ��r���Z�q	*/
	char	*str;	/* ����������	*/
{
	r_item_t	*ip;
	int		i;
	r_expr_t	*lv, *rv, *e;

DBG_ENT( M_RDB, "rdb_string_op" );
DBG_A(("item=[%s],op=%d,str%s=\n", item, op, str ));

	/*
	 *	�A�C�e������
	 */
	for( ip = &_search_tbl->t_scm->s_item[0], i = 0;
			i < _search_tbl->t_scm->s_n; ip++, i++ ) {
		if( !strcmp( ip->i_name, item ) )
			goto find;
	}
	goto err1;

find:
	/* 
	 *	��͖ؗp�\���̂̃������A���b�N 
	 */
	lv	= _e_alloc( sizeof(r_expr_t) );
	if( !lv )
		goto err1;
	rv	= _e_alloc( sizeof(r_expr_t) );
	if( !rv ) {
		_e_free( lv );
		goto err1;
	}
	e	= _e_alloc( sizeof(r_expr_t) );
	if( !e ) {
		_e_free( lv );
		_e_free( rv );
		goto err1;
	}

	/* 
	 *	���ݒ� 
	 */
	lv->e_op				= TLEAF;
	lv->e_tree.t_unary.v_type		= VITEM;
	lv->e_tree.t_unary.v_value.v_item	= ip;

	rv->e_op				= TLEAF;
	rv->e_tree.t_unary.v_type		= VSTRING;
	rv->e_tree.t_unary.v_value.v_str.s_len	= strlen( str );
	rv->e_tree.t_unary.v_value.v_str.s_str	= str;

	e->e_op				= op;
	e->e_tree.t_binary.b_l		= lv;
	e->e_tree.t_binary.b_r		= rv;

DBG_EXIT(e);
	return( e );
err1:
DBG_EXIT(0);
	return( 0 );
}

/******************************************************************************
 * �֐���
 *		rdb_numerical_op()
 * �@�\        
 *		�����f�[�^�p�� ��͖�LEAF �쐬
 * �֐��l
 *      ����: ��͖؂̃m�[�h
 *      �ُ�: 0
 * ����
 ******************************************************************************/
r_expr_t	*
rdb_numerical_op( item, op, num )
	char	*item;	/* �A�C�e����	*/
	long long	op;	/* ��r���Z�q	*/
	long	num;	/* ������	*/
{
	r_item_t	*ip;
	int		i;
	r_expr_t	*lv, *rv, *e;

DBG_ENT( M_RDB, "rdb_numerical_op" );
DBG_A(("item=[%s],op=%d,num=0x%xL\n", item, op, num ));

	/*
	 *	�A�C�e������
	 */
	for( ip = &_search_tbl->t_scm->s_item[0], i = 0;
			i < _search_tbl->t_scm->s_n; ip++, i++ ) {
		if( !strcmp( ip->i_name, item ) )
			goto find;
	}
	goto err1;

find:
	/* 
	 *	��͖ؗp�\���̂̃������A���b�N 
	 */
	lv	= _e_alloc( sizeof(r_expr_t) );
	if( !lv )
		goto err1;
	rv	= _e_alloc( sizeof(r_expr_t) );
	if( !rv ) {
		_e_free( lv );
		goto err1;
	}
	e	= _e_alloc( sizeof(r_expr_t) );
	if( !e ) {
		_e_free( lv );
		_e_free( rv );
		goto err1;
	}

	/* 
	 *	���ݒ� 
	 */
	lv->e_op				= TLEAF;
	lv->e_tree.t_unary.v_type		= VITEM;
	lv->e_tree.t_unary.v_value.v_item	= ip;

	rv->e_op				= TLEAF;
	rv->e_tree.t_unary.v_type		= VBIN;
	rv->e_tree.t_unary.v_value.v_int	= num;

	e->e_op				= op;
	e->e_tree.t_binary.b_l		= lv;
	e->e_tree.t_binary.b_r		= rv;

DBG_EXIT(e);
	return( e );
err1:
DBG_EXIT(0);
	return( 0 );
}

/******************************************************************************
 * �֐���
 *		rdb_logical_op()
 * �@�\        
 *		�_�����Z�p�� ��͖�LEAF �쐬
 * �֐��l
 *      ����: ��͖؂̃m�[�h
 *      �ُ�: 0
 * ����
 ******************************************************************************/
r_expr_t	*
rdb_logical_op( epl, op, epr )
	r_expr_t	*epl;	/* ��͖؂̃m�[�h	*/
	long long		op;	/* �_�����Z�q		*/
	r_expr_t	*epr;	/* ��͖؂̃m�[�h	*/
{
	r_expr_t	*e;

DBG_ENT( M_RDB, "rdb_logical_op" );

	/* 
	 *	��͖ؗp�\���̂̃������A���b�N 
	 */
	if( (e = _e_alloc(sizeof(r_expr_t) )) ) {
		e->e_op			= op;
		e->e_tree.t_binary.b_l	= epl;
		e->e_tree.t_binary.b_r	= epr;
	} else {
		goto err1;
	}

DBG_EXIT(e);
	return( e );
err1:
DBG_EXIT(0);
	return( 0 );
}

/******************************************************************************
 * �֐���
 *		rdb_evaluate()
 * �@�\        
 *		�ݒ肳�ꂽ���������ŁA���R�[�h��]������
 * �֐��l
 *      ����: 0
 *      �ُ�: -1
 * ����
 *	���ʂ� �^�Ȃ�� 1,  �U�Ȃ�� 0 �� vp->v_value.v_int �ɐݒ�
 *	�܂��A vp->v_type �� VBOOL(�_���l) ���ݒ肳��Ă��Ȃ����̓G���[
 ******************************************************************************/
int
rdb_evaluate( tp, rec, vp )
	r_expr_t	*tp;	/* I ��͖؂̃��[�g	*/
	char		*rec;	/* I ���R�[�h�f�[�^	*/
	r_value_t	*vp;	/* O �]������		*/
{
	r_value_t	vl, vr;

DBG_ENT( M_RDB, "rdb_evaluate" );

	/*
	 *	�m�[�h�̃^�C�v���`�F�b�N
	 */
	switch( tp->e_op ) {
		/*
		 *	�m�[�h�� �t(TLEAF)
		 *		vp �Ƀf�[�^��ݒ肷��
		 */
		case TLEAF:	/* terminal */
			switch( tp->e_tree.t_unary.v_type ) {
				case VITEM:
					rdb_item_value(
					  tp->e_tree.t_unary.v_value.v_item,
					  rec,
					  vp );
					break;
				case VSTRING:
DBG_A(("VSTRING(%s)\n", tp->e_tree.t_unary.v_value.v_str.s_str ));
					bcopy( &tp->e_tree.t_unary, vp,
								sizeof(*vp) );
					break;
				case VBIN:
DBG_A(("VBIN(0x%x)\n", tp->e_tree.t_unary.v_value.v_int ));
					bcopy( &tp->e_tree.t_unary, vp,
								sizeof(*vp) );
					break;
					
			}
DBG_EXIT(0);
			return( 0 );

		/*
		 *	�m�[�h�� ���Z�q
		 *		�ċA���J��Ԃ��� �t(TLEAF)�̉��Z�ɂȂ�����]��
		 *		���ʂ� �^�Ȃ�� 1,  �U�Ȃ�� 0 �� v_int �ɐݒ�
		 *		���̍ۂ� v_type �� VBOOL(�_���l) ��ݒ肷��
		 */
		default:
			/*
			 *	���ċA ( vl �ݒ� )
			 */
			if( rdb_evaluate( tp->e_tree.t_binary.b_l, 
							rec, &vl ) < 0 )
				goto err1;
			/*
			 *	�E�ċA ( vr �ݒ� )
			 */
			if( rdb_evaluate( tp->e_tree.t_binary.b_r, 
							rec, &vr ) < 0 )
				goto err1;

			/*
			 *	�]��
			 */
			switch( tp->e_op ) {

				/* logical operator */
				case SOR:	
					if( (vl.v_type != VBOOL) 
						|| (vr.v_type != VBOOL)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int 
							|| vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;
				case SAND:
					if( (vl.v_type != VBOOL) 
						|| (vr.v_type != VBOOL)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int 
							&& vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;

				/* arithmatic operator */
				case SLT:	
					if( (vl.v_type != VBIN) 
						|| (vr.v_type != VBIN)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int <
							vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;
				case SGT:
					if( (vl.v_type != VBIN) 
						|| (vr.v_type != VBIN)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int >
							vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;
				case SLE:
					if( (vl.v_type != VBIN) 
						|| (vr.v_type != VBIN)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int <=
							vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;
				case SGE:
					if( (vl.v_type != VBIN) 
						|| (vr.v_type != VBIN)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int >=
							vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;
				case SEQ:
					if( (vl.v_type != VBIN) 
						|| (vr.v_type != VBIN)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int ==
							vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;
				case SNE:
					if( (vl.v_type != VBIN) 
						|| (vr.v_type != VBIN)) {
							goto err1;
					} else {
						vp->v_type	= VBOOL;
						if( vl.v_value.v_int !=
							vr.v_value.v_int )
							vp->v_value.v_int = 1;
						else
							vp->v_value.v_int = 0;
					}
					break;
				case STR_EQ:
					if( (vl.v_type != VSTRING) 
						|| (vr.v_type != VSTRING)) {
							goto err1;
					} else {
					  vp->v_type	= VBOOL;
					  if(!strncmp( vl.v_value.v_str.s_str,
					  	       vr.v_value.v_str.s_str,
					  	       vl.v_value.v_str.s_len))
						vp->v_value.v_int = 1;
					  else
						vp->v_value.v_int = 0;
					}
					break;
				case STR_NE:
					if( (vl.v_type != VSTRING) 
						|| (vr.v_type != VSTRING)) {
							goto err1;
					} else {
					  vp->v_type	= VBOOL;
					  if( strncmp( vl.v_value.v_str.s_str,
					  	       vr.v_value.v_str.s_str,
					  	       vl.v_value.v_str.s_len))
						vp->v_value.v_int = 1;
					  else
						vp->v_value.v_int = 0;
					}
					break;
				case STR_GT:
					if( (vl.v_type != VSTRING) 
						|| (vr.v_type != VSTRING)) {
							goto err1;
					} else {
					  vp->v_type	= VBOOL;
					  if( strncmp( vl.v_value.v_str.s_str,
					  	   vr.v_value.v_str.s_str,
					  	   vl.v_value.v_str.s_len) > 0)
						vp->v_value.v_int = 1;
					  else
						vp->v_value.v_int = 0;
					}
					break;
				case STR_LT:
					if( (vl.v_type != VSTRING) 
						|| (vr.v_type != VSTRING)) {
							goto err1;
					} else {
					  vp->v_type	= VBOOL;
					  if( strncmp( vl.v_value.v_str.s_str,
					  	   vr.v_value.v_str.s_str,
					  	   vl.v_value.v_str.s_len) < 0)
						vp->v_value.v_int = 1;
					  else
						vp->v_value.v_int = 0;
					}
					break;
				case STR_GE:
					if( (vl.v_type != VSTRING) 
						|| (vr.v_type != VSTRING)) {
							goto err1;
					} else {
					  vp->v_type	= VBOOL;
					  if( strncmp( vl.v_value.v_str.s_str,
					  	   vr.v_value.v_str.s_str,
					  	   vl.v_value.v_str.s_len) >= 0)
						vp->v_value.v_int = 1;
					  else
						vp->v_value.v_int = 0;
					}
					break;
				case STR_LE:
					if( (vl.v_type != VSTRING) 
						|| (vr.v_type != VSTRING)) {
							goto err1;
					} else {
					  vp->v_type	= VBOOL;
					  if( strncmp( vl.v_value.v_str.s_str,
					  	   vr.v_value.v_str.s_str,
					  	   vl.v_value.v_str.s_len) <= 0)
						vp->v_value.v_int = 1;
					  else
						vp->v_value.v_int = 0;
					}
					break;

			}
			break;
	}

DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(-1);
	return( -1 );
}

/******************************************************************************
 * �֐���
 *		rdb_item_value()
 * �@�\        
 *		��͖؂� �t(VITEM) �ɁA���R�[�h�̃f�[�^��ݒ肷��
 * �֐��l
 *      �O�݂̂ŁA�Ӗ����Ȃ�
 * ����
 ******************************************************************************/
int
rdb_item_value( ip, rec, vp )
	r_item_t	*ip;	/* I �A�C�e���\����	*/
	char		*rec;	/* I ���R�[�h�f�[�^	*/
	r_value_t	*vp;	/* O ��͖؂� �t(VITEM)	*/
{
	int	len;

DBG_ENT( M_RDB, "rdb_item_value" );

	switch( ip->i_attr ) {
		case R_HEX:
		case R_INT:
		case R_UINT:
		case R_FLOAT:
		case R_LONG:
		case R_ULONG:
			vp->v_type	= VBIN;
			len = ip->i_len < sizeof(vp->v_value) ?
					ip->i_len : sizeof(vp->v_value);
			bcopy( rec + ip->i_off, &vp->v_value, len );
			break;
		case R_STR:
			vp->v_type		= VSTRING;
			vp->v_value.v_str.s_len	= ip->i_len;
			vp->v_value.v_str.s_str	= (char *)(rec+ip->i_off);
			break;
		case R_BYTES:
		    vp->v_type		= VSTRING;
			vp->v_value.v_str.s_len	= ip->i_len;
			vp->v_value.v_str.s_str	= (char *)(rec+ip->i_off);
			break;

	}

DBG_EXIT(0);
	return( 0 );
}

/******************************************************************************
 * �֐���
 *		_e_alloc()
 * �@�\        
 *		��͖ؗp�\���̂̃������A���b�N
 * �֐��l
 *      ����: �m�[�h�ւ̃|�C���^
 *      �ُ�: 0
 * ����
 *	���݂́A�Œ�̈悩�珇�ԂɎ擾���Ă��邾��
 ******************************************************************************/
r_expr_t	*
_e_alloc( n )
	int	n;	/* ���݁A���g�p	*/
{
	int	i;

DBG_ENT(M_RDB,"e_alloc");

#ifdef NODEF
	if( !_search_usr->u_search.s_ebuff ) {
		if( !(_search_usr->u_search.s_ebuff = 
			(r_expr_t *)neo_malloc(R_EXPR_MAX*sizeof(r_expr_t))) ) {
			goto err1;
		} else {
			_search_usr->u_search.s_max	= R_EXPR_MAX;
			_search_usr->u_search.s_cnt	= 0;
		}
		if( _search_usr->u_search.s_cnt 
				>= _search_usr->u_search.s_max )
			goto err1;
	}
	DBG_EXIT(_search_usr->u_search.s_ebuff+_search_usr->u_search.s_cnt++ );
	return( _search_usr->u_search.s_ebuff+_search_usr->u_search.s_cnt++ );
#endif

	if( _search_usr->u_search.s_cnt 
			>= _search_usr->u_search.s_max )
		goto err1;

	i = _search_usr->u_search.s_cnt++;

DBG_EXIT(i);
	return( &_search_usr->u_search.s_elm[i] );
err1:
DBG_EXIT(0);
	return( 0 );
}

/******************************************************************************
 * �֐���
 *		_e_free()
 * �@�\        
 *		��͖ؗp�\���̂̃��������
 * �֐��l
 *      �O�݂̂ŁA�Ӗ����Ȃ�
 * ����
 *	���݂́A�������Ȃ�
 ******************************************************************************/
int
_e_free( ep )
	r_expr_t	*ep;	/* ���݁A���g�p */
{
	return( 0 );
}
