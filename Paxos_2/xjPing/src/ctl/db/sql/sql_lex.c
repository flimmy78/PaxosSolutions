/*******************************************************************************
 * 
 *  sql_lex.c --- xjPing (On Memory DB)  Library programs
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

/*
 *  ����������͗p���������^�������֐��Q
 *
 */
#include <stdio.h>
#include <ctype.h>
#include "neo_system.h"
#include "neo_db.h"
#include "sql.h"


#define yylval  sqllval

/*===================================================================
*   character handle table
*==================================================================*/
static int  char_tab[256];      /* character handle table */

#define A_WHITE     0x00000001      /* space character  */
#define A_1CHAR     0x00000002      /* punctuate character  */
#define A_STR       0x00000004      /* start symbol character */
#define A_NUMB      0x00000008      /* 0-9 */
#define A_QUOTE     0x00000010      /* ' */

#define A_MORE      0x00000100      /* double punct :=,>= etc */

#define M_STR       0x00000200      /* symbol charcter  */
#define M_NUMB      0x00000400      /* 0-9_ */
#define M_HEX       0x00000800      /* hexa character */

#define is_white(c) ( char_tab[c] & A_WHITE )
#define what_is(c)  ( char_tab[c] & 0x00ff )
#define is_beginname(c) ( char_tab[c] & A_STR )
#define is_midname(c)   ( char_tab[c] & M_STR )

/*===================================================================
*   unget character buffer
*   =================================================
*   |   yysbuf[YYLMAX]              |
*   =================================================
*       ^ *yysptr ( next store pointer )
*
*   yytext return buffer
*   =================================================
*   |   text_buffer[YYTEXT_MAX]          |
*   =================================================
*       ^ *yytext   ^ inptr         ^ text_buffer_end
*       token start next store pointer
*   previous_yytext 0 yytext 0
*==================================================================*/
#define YYTEXT_MAX 512
#define YYLMAX  256

static  char    *text_buffer, *inptr, *text_buffer_end;
static  char    yysbuf[YYLMAX];
static  char    *yysptr = yysbuf;

static  int yytchar;            /* current character    */
static  int yyleng;             /* return buffer length */
static  int through_flag;           /* not convert Capital  */

static  char    *yytext;            /* return buffer pointer */
static  int save_yytext = 1;        /* save yytext flag */

/*===================================================================
*   token buffer
*   =================================================
*   |   token_stack[TOKEN_MAX]          |
*   =================================================
*    -> ^ token_idx     ^ token_max
*       next read   next store
*==================================================================*/

static  FILE    *yyin, *yyout;
/***
static  FILE    *yyin ={&stdin}, *yyout ={&stdout};
***/
static  const char    *strptr;

int (*cl_getc)();
static int	more_punct();
static int	read_token();
static int	more_str();
static int	cl_literal();
static void	error();


/*===================================================================
*   functions
*==================================================================*/
static  int (*input)();         /* getchar function */
static  void (*unput)();         /* un_getchar function */

static int
cl_getstr(in)
{
    register    int c;
    if((c = *strptr)) strptr++;
    return(c);
}

static int
input_nl()
/*===================================================================
*   �@�\
*       �ǂ݂��ǂ�����������΁A�����
*       �Ȃ���Γ��̓\�[�X����ꕶ���ǂ݂���
*       �Z�[�u�v��������΁i���������Q���������j
*       ���������Q�������ɃZ�[�u���A�m�t�k�k�ŕ���
*       �s�ԍ��i���������������������j���Ǘ�����
*   �֐��l
*       �ǂ݂��񂾕���
*       �O�@�F���@�d�n�e
*==================================================================*/
{
    register    int c;

    if( yysptr > yysbuf )
        c   = *--yysptr;
    else
        c = (*cl_getc)( yyin );

    if( save_yytext )
    {
        if( inptr < text_buffer_end )
        {
            *inptr++    = c;
            *inptr  = 0;
        }
        else if( inptr == text_buffer_end )
        {
            error("lex:text_buffer overflow\n");
        }
    }

    return( yytchar = (c & 0xff) );
}

static void
unput_nl(c)
register int    c;
/*==============================================================
*   �@�\
*       �����������ɂP�����߂�
*       ���������������P��������
*       �s�ԍ��i���������������������j���Ǘ�����
*   �֐��l
*       �Ȃ�
*==============================================================*/
{
    *yysptr++ = yytchar = c;

    if( save_yytext ) {
        if( inptr <= text_buffer )
            error("lex:text_buffer underflow\n");
        else
            *--inptr    = 0;
    }
}

static int sql_start_token;

static void
_sql_lex_start(str, bufp, size, start_token)
const char *str;
char *bufp;
int size;
int start_token;
{
    yyin = stdin;
    yyout = stdout;
    strptr = str;
    cl_getc = cl_getstr;
    save_yytext = 1;
    yysptr = yysbuf;
    text_buffer = bufp;
    text_buffer_end = bufp + size;
    inptr = text_buffer;
    sql_start_token = start_token;
}

void
sql_lex_start(str, bufp, size)
const char *str;
char *bufp;
int size;
{
    _sql_lex_start(str, bufp, size, START_program);
}

void
sql_lex_pstat_start(str, bufp, size)
const char *str;
char *bufp;
int size;
{
    _sql_lex_start(str, bufp, size, START_pstatement);
}

void
sql_lex_init()
/*===================================================================
*   �@�\
*       �k�d�w�̃L�����N�^�e�[�u��������������
*       �L�C���[�h�n�b�V���e�[�u��������
===================================================================*/
/*-------------------------------------------------------------------
*   function
*       initialize lex table
*   return value
*       NONE
*------------------------------------------------------------------*/
{
    register char   *p;
    static  char    *sabc   =  "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static  char    *s123   =  "0123456789";
    static  char    *shex   =  "XABCDEF";


    char_tab[' ']   |= A_WHITE;
    char_tab['\t']  |= A_WHITE;
    char_tab['\n']  |= A_WHITE;

    char_tab['\'']  |= A_QUOTE;

    p   = "=-()<>!|&~;,.*+/?";
    while( *p ) char_tab[(int)*p++]  |= A_1CHAR;

    p   = "<>=!|&";
    while( *p ) char_tab[(int)*p++]  |= A_MORE;

    p   = s123;
    while( *p )
        char_tab[(int)*p++]  |= A_NUMB | M_STR | M_NUMB | M_HEX;
    p   = sabc;
    while( *p ){
        char_tab[*p + 'a' - 'A']    |= A_STR | M_STR ;
        char_tab[(int)*p++]  |= A_STR | M_STR ;
    }
    p   = shex;
    while( *p ) {
        char_tab[*p + 'a' - 'A']    |= M_NUMB|M_HEX;
        char_tab[(int)*p++]  |= M_NUMB|M_HEX;
    }

    char_tab['_']   |= M_STR ;
	int	i;
	for( i = 128; i < 256; i++ ) {
		char_tab[i]	|= A_STR|M_STR;
	}

    input = input_nl;
    unput = unput_nl;
}

int
sqllex()
{
    register int    c;
    if (sql_start_token) {
        int t = sql_start_token;
        sql_start_token = 0;
        return t;
    }
    c = read_token();
    return(c);
}

static int
read_token()
/*==============================================================-
*   �@�\
*       �ꕶ���ǂݍ��݁A�����͂��s���A�g�[�N����������
*   �֐��l
*       �g�[�N���ԍ�
*       0 : EOF
*==============================================================*/
/*-------------------------------------------------------------------
*   function
*       read character & make up token
*   return value
*       token
*       0 : EOF
*------------------------------------------------------------------*/
{
    register int    c,c1,lval;

    yytext  = ++inptr;


    for( ; (lval = c = (*input)()); inptr = yytext )
    {
        if( char_tab[c] & A_MORE )
        /*==========================
        * �ŏ��̕������������ꕶ�����A�Q�C�P�U�i�����i�a�C�g�j
        *=========================*/
        {
            c1  = (*input)();

            switch( what_is( c ) )
            {
                case A_1CHAR    :
            /*==========================
            * �ŏ��̕������������ꕶ��
            *=========================*/
                    lval =  more_punct( c,c1 );
                    yylval.i = lval;
                    goto gotone;

                default :
                    (*unput)( c1 );
            }
        }
        switch( what_is( c ) )
        {
            case    A_WHITE :   lval = 0;   break;

            case    A_QUOTE :   lval = cl_literal();
                        goto gotone;

            case    A_1CHAR :
                        if( c == '.' ) {
                            c1 = (*input)();
                            if( what_is(c1)
                                == A_NUMB ) {
                                (*unput)(c1);
                                goto to_float;
                            }
                            (*unput)(c1);
                        }
                        yylval.i = c;
                        goto gotone;

            case    A_STR   :   lval = more_str( M_STR );
                        goto gotone;

        to_float:
            case    A_NUMB  :   (*unput)( c );
                        lval = more_str( M_NUMB );
                        goto gotone;

            default     :
                error("lex:Unknown character %c\n",c);
                lval = 0;
				return( 0 );	// EOF
        }

gotone:
        if( lval )  break;
    }
    /*==========================
    * lexval �ɒl���Z�b�g����
    *=========================*/

    yyleng  = inptr - yytext;

    return( lval );
}

static	int
more_punct( c , c2 )
register int    c;  /* first character */
register int    c2; /* second character */
/*===================================================================
*   �@�\
*       �������ꕶ���g�[�N��������
*       �R�����g��ǂ݂Ƃ΂�
*       ���������łȂ��Ƃ���Ԗڂ̕������Q��ǂݖ߂�
*   �֐��l
*       �g�[�N���ԍ�
===================================================================*/
/*-------------------------------------------------------------------
*   function
*       make up double punctuate token
*   return value
*       0 : COMMENT
*------------------------------------------------------------------*/
{
    register int    val;

    val = 0;
    switch(c2)
    {
    case    '=' :
        switch( c )
        {
        case    '=' : val = SEQ;    break;
        case    '>' : val = SGE;    break;
        case    '<' : val = SLE;    break;
        case    '!' : val = SNE;    break;
        default     : break;
        }
        break;
    case    '|' :   if(c == '|')    val = SOR;
                break;
    case    '&' :   if(c == '&')    val = SAND;
                break;
    case    '>' :   if(c == '<')    val = SNE;
                break;

    default     :
        switch( c )
        {
        case    '>' : val = SGT;    (*unput)(c2);   break;
        case    '<' : val = SLT;    (*unput)(c2);   break;
        }
    }

    if( !val ){
        (*unput)( c2 );
        val = c;
    }

    return( val );
}

static int
more_str( mask )
int mask;   /* B,H,M_NUMB,M_STR */
/*==============================================================-
*   �@�\
*       ���������̒l�ɂ��p�����ׂ��������ǂ݂���
*       �L�[���[�h�A�V���{���A���l������
*       ���l�萔�̂Ƃ��\�i�l���������������̂������ɃZ�b�g����
*       �����{�p���̓G���[�ɂ��A�p����ǂݎ̂Ă�
*       .�̂���ꍇ��float�Ƃ���B
*   �֐��l
*       �g�N���ԍ�
*==============================================================*/
/*-------------------------------------------------------------------
*   function
*       make up KEYWORD SYMBOL INT_CONST BIT_CONST
*       ignore  bad continuous
*   return value
*       token
*------------------------------------------------------------------*/
{
    register    int c;
    long long   n = 0;
    char        *p;
    double      f, fd;

DBG_ENT( M_SQL, "read_token" );

    for(n = 0; char_tab[ c = (*input)()] & mask ; )
    {
        if(mask == M_STR || c == '_')   continue;

        if( c == 'X' || c == 'x' ) {
            mask = M_HEX;
            continue;
        }
        if( mask == M_NUMB )
            n *= 10;
        else if( mask == M_HEX )
        {
            n <<= 4;
            if( char_tab[c] & A_STR )
            {
                if( c < 'a' )
                    n   += 10 + c - 'A';
                else
                    n   += 10 + c - 'a';
                continue;
            }
        }

        n   += c - '0';
    }
    if( mask == M_NUMB && c == '.' ) {  /* must be float */

        f = n;

        for(fd = 10.0; char_tab[ c = (*input)()] & mask ; fd *= 10.0){

            f += (c - '0')/fd;

        }

        (*unput)( c );
        yylval.f = f;
        return( SFLOAT );
    }
    (*unput)( c );

    /*========================================
    * �����{�p���̓G���[�ɂ��A�p����ǂݎ̂Ă�
    *=======================================*/
    if( char_tab[c] & M_STR )
    {
        p = ++inptr;
        for(; char_tab[ c = (*input)()] & M_STR ; );
        (*unput)( c );
        error("lex:bad constant %s\n",p);
    }

    /*========================================
    * �V���{�����A�L�C���[�h�����ׂ�
    *=======================================*/
    if( mask == M_NUMB || mask == M_HEX ){
        mask    = SNUM;
        yylval.l = n;
    }
    else if( mask == M_STR )    /* mask == M_STR */
    {
        mask = SNAME;
        if( !( mask = sqlkey( yytext ) ) ) {
            mask = SNAME;
        }
        yylval.s = yytext;
    }

    return( mask );
}

static int
cl_literal()
/*===================================================================
*   �@�\
*       �f�ł�����ꂽ���������؉��H���Ȃ��ŕ����萔�Ƃ���
*       But, '' should be '
*   �֐��l
*       �b�x�Q�r�s�q�Q�b�n�m�r�s
===================================================================*/
/*-------------------------------------------------------------------
*   function
*       make up LITERAL
*   return value
*       CY_STR_CONST
*------------------------------------------------------------------*/
{
    register    int c;


    through_flag    = 1;

    while( (c = (*input)()) )
    {
        if(c == '\'' )
        {
            if((c = (*input)()) != '\'' ){
                break;
            } else {
                --inptr;
                *inptr = 0;
            }
        }
    }

    through_flag    = 0;

    if( islower( c ) )
        c   = toupper( c );
    (*unput)(c);
    *(inptr -1) = 0;

    yylval.s = yytext+1;
    return(SSTRING);

}

void
sqlerror(s)
char    *s;
{
DBG_ENT( M_SQL, "yyerror" );
    DBG_A(("db_lex-1:yacc:[%s] yytext=[%s]\n",s,yytext));
}

static void
error(s,a,b,c,d)
char    *s;
{
DBG_ENT( M_SQL, "error" );
    DBG_A((s,a,b,c,d));
}

