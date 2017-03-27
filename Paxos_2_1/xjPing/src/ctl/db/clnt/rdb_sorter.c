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
 *	rdb_sorter.c
 *
 *	����	R_CMD_SORT
 *			 	 �e�[�u�����\�[�g����A���S���Y��
 *
 *	(��)  �`�o�ɂ͊J������Ȃ�
 *
 ******************************************************************************/

#include	"neo_db.h"

static int	db_compare(), db_down(), db_up(), db_sdown(), db_sup(), 
		db_fdown(), db_fup();
static int	db_ldown(), db_lup();
static int	db_udown(), db_uup();
static int	db_uldown(), db_ulup();

/* �\�[�g���͊�\���� */
typedef	struct	r_sorter {
	int	s_offset;	/* �t�B�[���h�̃��R�[�h���̃I�t�Z�b�g�n�l */
	int	s_length;	/* �t�B�[���h�̗L���o�C�g��		  */
	int	(*s_comp)();	/* �Y���t�B�[���h��Ή�����\�[�g�֐�	  */
} r_sorter_t;

static int		sorters;
static r_sorter_t	*sorter;

/*****************************************************************************
 *
 *  �֐���	_rdb_sort_open()
 *
 *  �@�\	�\�[�g�������
 * 
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
_rdb_sort_open( td, n, sort )
	r_tbl_t		*td;	 	/* �\�[�g�Ώۃe�[�u���L�q�q */
	int		n;		/* �\�[�g�L�[�̐�	    */
	r_sort_t	*sort;		/* �\�[�g�L�[		    */
{
	int		i, j;
	int		n_item;
	r_item_t	*item;
	int		found;
	int		err;
DBG_ENT(M_RDB,"_rdb_sanal");

	/*
	 * �e�L�[�p�̃\�[�g���͊�G���A��p�ӂ���
	 */
	sorters	= n;
	if( !(sorter = (r_sorter_t *)neo_malloc(sorters*sizeof(r_sorter_t))) ) {
		err = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 * ��L���������e�L�[�̕��͊�ɓ��e��ݒ肷��
	 */
	n_item	= td->t_scm->s_n;
	for( i = 0; i < n; i++ ) {

		/*
		 * �L�[�ƈ�v����t�B�[���h��{���o���āA�Y���t�B�[���h
		 * �̑������A�L�[�̃\�[�g���͊��ݒ肷��
		 */
		found = 0;
		for( j = 0; j < n_item; j++ ) {
			item = &td->t_scm->s_item[j];
			if( !(strncmp( item->i_name, sort[i].s_name,
						sizeof(r_item_name_t)) ) ) {

				/*
				 * �L���o�C�g���ƃI�t�Z�b�g�l�ݒ�
				 */
				sorter[i].s_offset	= item->i_off;
				sorter[i].s_length	= item->i_len;

				/*
				 */
				if ( item->i_attr == R_INT ) { 

					if( sort[i].s_order == R_DESCENDANT )
						sorter[i].s_comp = db_down;
					else
						sorter[i].s_comp = db_up;

				}

				else if ( item->i_attr == R_HEX 
				  || item->i_attr == R_UINT ) {

					if( sort[i].s_order == R_DESCENDANT )
						sorter[i].s_comp = db_udown;
					else
						sorter[i].s_comp = db_uup;

				}

				 else if ( item->i_attr == R_LONG ) {

					if( sort[i].s_order == R_DESCENDANT )
						sorter[i].s_comp = db_ldown;
					else
						sorter[i].s_comp = db_lup;

				} 

				 else if ( item->i_attr == R_ULONG ) {

					if( sort[i].s_order == R_DESCENDANT )
						sorter[i].s_comp = db_uldown;
					else
						sorter[i].s_comp = db_ulup;

				} 

				/*
				 * �����̏ꍇ�ɂ́Adb_fdown ���邢��
				 * db_fup �\�[�g�֐���p����
				 */
				 else if ( item->i_attr == R_FLOAT ) {

					if( sort[i].s_order == R_DESCENDANT )
						sorter[i].s_comp = db_fdown;
					else
						sorter[i].s_comp = db_fup;

				} 

				/*
				 * ������̏ꍇ�ɂ� db_sdown ���邢��
				 * db_sup�\�[�g�֐���p����
				 */
				else if ( item->i_attr == R_STR ) {

					if( sort[i].s_order == R_DESCENDANT )
						sorter[i].s_comp = db_sdown;
					else
						sorter[i].s_comp = db_sup;
				}
				else if ( item->i_attr == R_BYTES ) {
                    //ignore R_BYTES sort
				}

				found = 1;
				goto next;
			}
		}
next:
		
		/*
		 * �L�[�ƈ�v����t�B�[���h�����A�\�[�g�L�[�w��G���[
		 */
		if ( !found ) {
			err = E_RDB_NOITEM;
			goto err2;
		}
	}

DBG_EXIT(0);
	return( 0 );

err2: DBG_T("err2");
	neo_free( sorter );
	sorter = 0;
err1:
DBG_EXIT(err);
	return( err );

}


/*****************************************************************************
 *
 *  �֐���	_rdb_sort_close()
 *
 *  �@�\	�\�[�g�̌㏈��
 * 
 *  �֐��l
 *     		0
 *
 ******************************************************************************/
int
_rdb_sort_close()
{
DBG_ENT(M_RDB,"_rdb_sort_close");

	/*
	 * �L�[�̃\�[�g���͊�G���A���������
	 */
	neo_free( sorter );
	sorter = 0;

DBG_EXIT(0);
	return( 0 );
}


/*****************************************************************************
 *
 *  �֐���	_rdb_sort_start()
 *
 *  �@�\	�\�[�g�����{����
 * 
 *  �֐��l	0
 *
 ******************************************************************************/
int
_rdb_sort_start( start, num )
	r_sort_index_t	*start;		/* �\�[�g�p Index �G���A */
	int		num;		/* �\�[�g�Ώۃ��R�[�h��	 */
{
DBG_ENT(M_RDB,"_rdb_sort_start");

	/*
	 * �V�X�e�����C�u�����֐�qsort
	 * (�N�C�b�N�\�[�g)�ɂă\�[�g������
	 */
	qsort( start, num, sizeof(r_sort_index_t), db_compare );
DBG_EXIT(0);
	return( 0 );
}


/*****************************************************************************
 *
 *  �֐���	_rdb_arrange_data()
 *
 *  �@�\	�\�[�g��A���ۃ�������̃��R�[�h�̈ʒu�𒲐�����
 * 		(�\�[�g�́AIndex �e�[�u���ɑ΂��čs�����̂�)
 *
 *  �֐��l
 *     		����	0
 *		�ُ�	�G���[�ԍ�
 *
 ******************************************************************************/
int
_rdb_arrange_data( abs, rel, n, size )
	r_sort_index_t	*abs,		/* �\�[�g�O Index ���	*/
			*rel;		/* �\�[�g�� Index ���	*/
	int		n;		/* ���R�[�h��		*/
	int		size;		/* ���R�[�h�T�C�Y	*/
{
	int		i, j, k, l;
	int		is;
	r_head_t	*wp;
	int		err;
	int		reln, absn;

DBG_ENT(M_RDB,"_rdb_arrange_data");

	/*
	 * �ꃌ�R�[�h�T�C�Y���̗Վ��G���A��p�ӂ���
	 */
	if( !(wp = (r_head_t *)neo_malloc( size )) ) {
		err = E_RDB_NOMEM;
		goto err1;
	}

	/*
	 * �S���R�[�h�����`�F�b�N����
	 */
	for( i = 0; i < n; i++ ) {

		/*
		 * �\�[�g�ヌ�R�[�h�̈ʒu���ύX���Ă��Ȃ��A���邢��
		 * �O�̃��[�v�Ŋ��ɓK���ȏ��Ɉڂ����̏ꍇ�ɂ́A�������Ȃ�
		 */
		if( rel[i].i_id == i )
			continue;

		/*
		 * �ړ��́A�e�z���X�g���ōs���āAis ���z���X�g��
		 * �擪�ԍ��Ƃ��āA�擪���R�[�h��Վ��G���A�ɑޔ�������
		 * [�z���X�g(1,3,6)�Ƃ́A1��3�ŏ�������(1,3), (3,6)
		 *  (6,1)�Ƃ����u�����X�g]
		 */ 
		is = j = i;

		bcopy( abs[is].i_head, wp, size );

		for( k = 0; k < n; k++ )  {

			/*
			 * ���R�[�h�����������A�A���A���R�[�h�̐�΁A����
			 * �ԍ��͌����R�[�h�̂Ƃ���
			 */
			reln	= abs[j].i_head->r_rel;
			absn	= abs[j].i_head->r_abs;

			bcopy( rel[j].i_head, abs[j].i_head, size );

			abs[j].i_head->r_rel	= reln;
			abs[j].i_head->r_abs	= absn;

			/*
			 * �z���X�g�����̔ԍ���􂢏o��
			 */
			l	= rel[j].i_id;	/* next */

			rel[j].i_id	= j;

			j	= l;

			/*
			 * �z���X�g���Ō�̃��R�[�h��������A
			 * ������������ƈႤ
			 */
			if( rel[j].i_id == is )
				goto term;
		}
DBG_A(("SORTER ERROR\n"));
		err = E_RDB_CONFUSED;
		goto err1;
	term:
		/*
		 * �z���X�g���Ō�̃��R�[�h�̏ꍇ�ɂ́A
		 * �O�Վ��G���A�ɑޔ��������z���X�g���擪���R�[�h��
		 * ����������
		 */
		reln	= abs[j].i_head->r_rel;
		absn	= abs[j].i_head->r_abs;

		bcopy( wp, abs[j].i_head, size );

		abs[j].i_head->r_rel	= reln;
		abs[j].i_head->r_abs	= absn;

		rel[j].i_id		= j;
	}

	/*
	 * �Վ��G���A���������
	 */
	neo_free( wp );
DBG_EXIT(0);
	return( 0 );
err1:
DBG_EXIT(err);
	return( err );
}


/*****************************************************************************
 *
 *  �֐���	db_compare()
 *
 *  �@�\	qsort()����R�[��������r�֐�
 * 
 *  �֐��l
 *     		�[����������
 *		�[��
 *		�[���ُ퐮��
 *
 ******************************************************************************/
static int
db_compare( rec1, rec2 )
	r_sort_index_t	*rec1;		/* ��r���郌�R�[�h�P	*/
	r_sort_index_t	*rec2;		/* ��r���郌�R�[�h�Q	*/
{
	register int	i, off;
	int	ret;

	/*
	 * ��r�����Ƃ��āA�L�[�̕��׏��ɂ���āA����L�[�œ��
	 * ���R�[�h�̑召�֌W���ł���܂ŁA�e�L�[����r����
	 */
	ret	= 0;
	for( i = 0; i< sorters; i++ ) {
		off	= sorter[i].s_offset;
		if( ( ret = (*sorter[i].s_comp)( (char*)rec1->i_head+off, 
			      (char*)rec2->i_head+off, sorter[i].s_length ) ) ){
			break;
		}
	}

	return( ret );
}


/*****************************************************************************
 *
 *  �֐���	db_up()
 *
 *  �@�\	�������ڂɂ��Ă̏����]���֐�
 * 
 *  �֐��l
 *     		������
 *		0
 *		������
 *
 ******************************************************************************/
static int
db_up( data1, data2, len ) 
	register int	*data1, *data2, len;
{
	return( *data1 - *data2 );
}

static int
db_uup( data1, data2, len ) 
	register unsigned int	*data1, *data2, len;
{
	return( *data1 - *data2 );
}

static int
db_lup( data1, data2, len ) 
	register long long	*data1, *data2;
	register int		len;
{
	if( *data1 == *data2 )		return( 0 );
	else if( *data1 > *data2 )	return( 1 );
	else				return( -1 );
}

static int
db_ulup( data1, data2, len ) 
	register unsigned long long	*data1, *data2;
	register int		len;
{
	if( *data1 == *data2 )		return( 0 );
	else if( *data1 > *data2 )	return( 1 );
	else				return( -1 );
}

/*****************************************************************************
 *
 *  �֐���	db_down()
 *
 *  �@�\	�������ڂɂ��Ă̍~���]���֐�
 * 
 *  �֐��l
 *     		������
 *		0
 *		������
 *
 ******************************************************************************/
static int
db_down( data1, data2, len ) 
	register int	*data1, *data2, len;
{
	return( *data2 - *data1 );
}

static int
db_udown( data1, data2, len ) 
	register unsigned int	*data1, *data2, len;
{
	return( *data2 - *data1 );
}


static int
db_ldown( data1, data2, len ) 
	register long long	*data1, *data2;
	register int		len;
{
	if( *data2 == *data1 )		return( 0 );
	else if( *data2 > *data1 )	return( 1 );
	else				return( -1 );
}

static int
db_uldown( data1, data2, len ) 
	register unsigned long long	*data1, *data2;
	register int		len;
{
	if( *data2 == *data1 )		return( 0 );
	else if( *data2 > *data1 )	return( 1 );
	else				return( -1 );
}

/*****************************************************************************
 *
 *  �֐���	db_fup()
 *
 *  �@�\	�������ڂɂ��Ă̏����]���֐�
 * 
 *  �֐��l
 *     		-1
 *		0
 *		1
 *
 ******************************************************************************/
static int
db_fup( data1, data2, len ) 
	register float	*data1, *data2;
	int	len;
{
	if ( *data1 > *data2 ) 
		return( 1 );
	else if ( *data1 == *data2 )
		return( 0 );
	else 
		return( -1 );
}


/*****************************************************************************
 *
 *  �֐���	db_fdown()
 *
 *  �@�\	�������ڂɂ��Ă̍~���]���֐�
 * 
 *  �֐��l
 *     		-1
 *	 	0
 *	 	1
 *
 ******************************************************************************/
static int
db_fdown( data1, data2, len ) 
	register float	*data1, *data2;
	int	len;
{
	if ( *data1 > *data2 ) 
		return( -1 );
	else if ( *data1 == *data2 )
		return( 0 );
	else 
		return( 1 );
}


/*****************************************************************************
 *
 *  �֐���	db_sup()
 *
 *  �@�\	�����񍀖ڂɂ��Ă̏����]���֐�
 * 
 *  �֐��l
 *     		������
 *		0
 *		������
 *
 ******************************************************************************/
static int
db_sup( data1, data2, len ) 
	register char	*data1, *data2;
	register int	len;
{
	return( strncmp( data1, data2, len ) );
}


/*****************************************************************************
 *
 *  �֐���	db_sdown()
 *
 *  �@�\	�����񍀖ڂɂ��Ă̕]���֐�
 * 
 *  �֐��l
 *     		������
 *		0
 *		������
 *
 ******************************************************************************/
static int
db_sdown( data1, data2, len ) 
	register char	*data1, *data2;
	register int	len;
{
	return( strncmp( data2, data1, len ) );
}

