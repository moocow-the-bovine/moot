#define YY_CHAR unsigned char
#line 1 "/usr/local/share/flex++bison++/flexskel.cc"
/* A lexical scanner generated by flex */
/* scanner skeleton version:
 * $Header$
 */
/* MODIFIED FOR C++ CLASS BY Alain Coetmeur: coetmeur(at)icdc.fr */
/* Note that (at) mean the 'at' symbol that I cannot write */
/* because it is expanded to the class name */
/* made at Informatique-CDC, Research&development department */
/* company from the Caisse Des Depots et Consignations */
/* institutional financial group  */

/* theses symbols are added before this file */
/* #define YY_CHAR 'unsigned char' if 8bit or 'char' if 7bit */
/* #define FLEX_DEBUG if debug mode */
#define FLEX_SCANNER
/* cfront 1.2 defines "c_plusplus" instead of "__cplusplus" */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
/* Old MSC, before c7 */
#ifdef MSDOS
#ifndef _MSDOS
#define _MSDOS
#endif
#endif
/* turboc */
#ifdef __MSDOS__
#ifndef _MSDOS
#define _MSDOS
#endif
#endif

#ifdef __cplusplus
#include <stdlib.h>
#define YY_USE_CONST
#define YY_USE_PROTOS
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#ifndef _MSDOS
#include <osfcn.h>
#endif
#else   /* ! __cplusplus */
#ifdef __STDC__
#ifdef __GNUC__
#include <stddef.h>
void *malloc( size_t );
void free( void* );
int read();
#else
#include <stdlib.h>
#endif  /* __GNUC__ */
#define YY_USE_PROTOS
#define YY_USE_CONST
#endif  /* __STDC__ */
#endif  /* ! __cplusplus */
#ifdef __TURBOC__
#define YY_USE_CONST
#endif
#include <stdio.h>


/*********************************************/
/* COMPILER DEPENDENT   MACROS               */
/*********************************************/
/* use prototypes in function declarations */
/* the "const" storage-class-modifier is valid */
#ifndef YY_USE_CONST
#define const
#endif
/* use prototypes in function declarations */
#ifndef YY_PROTO
#ifdef YY_USE_PROTOS
#define YY_PROTO(proto) proto
#else
#define YY_PROTO(proto) ()
#endif
#endif


/*********************/
/* parameters        */

/* amount of stuff to slurp up with each read */
#ifndef YY_READ_BUF_SIZE
#define YY_READ_BUF_SIZE 8192
#endif
/* size of default input buffer */
#ifndef YY_BUF_SIZE
#define YY_BUF_SIZE (YY_READ_BUF_SIZE * 2) 
#endif

/***********************************/
/* to be redefined for application */

/* returned upon end-of-file */
#define YY_END_TOK 0
/* no semi-colon after return; correct usage is to write "yyterminate();" -
 * we don't want an extra ';' after the "return" because that will cause
 * some compilers to complain about unreachable statements.
 */
#define yyterminate() return ( YY_NULL )

/* code executed at the end of each rule */
#define YY_BREAK break;

/* #define YY_USER_ACTION */
/* #define YY_USER_INIT */


#ifndef YY_USE_CLASS
/* copy whatever the last rule matched to the standard output */
/* cast to (char *) is because for 8-bit chars, yy___text is (unsigned char *) */
/* this used to be an fputs(), but since the string might contain NUL's,
 * we now use fwrite()
 */
#define ECHO (void) fwrite( (char *) yy___text, yy___leng, 1, yy___out )

/* gets input and stuffs it into "buf".  number of characters read, or YY_NULL,
 * is returned in "result".
 */
#ifdef _MSDOS
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread(buf,1,max_size,yy___in)) < 0 ) \
	    YY_FATAL_ERROR( "fread() in flex scanner failed" );
#else
#define YY_INPUT(buf,result,max_size) \
	if ( (result = read( fileno(yy___in), (char *) buf, max_size )) < 0 ) \
	    YY_FATAL_ERROR( "read() in flex scanner failed" );

#endif
/* report a fatal error */

/* The funky do-while is used to turn this macro definition into
 * a single C statement (which needs a semi-colon terminator).
 * This avoids problems with code like:
 *
 *      if ( something_happens )
 *              YY_FATAL_ERROR( "oops, the something happened" );
 *      else
 *              everything_okay();
 *
 * Prior to using the do-while the compiler would get upset at the
 * "else" because it interpreted the "if" statement as being all
 * done when it reached the ';' after the YY_FATAL_ERROR() call.
 */

#define YY_FATAL_ERROR(msg) \
	do \
		{ \
		(void) fputs( msg, stderr ); \
		(void) putc( '\n', stderr ); \
		exit( 1 ); \
		} \
	while ( 0 )

/* default yywrap function - always treat EOF as an EOF */
#define yywrap() 1


/* default declaration of generated scanner - a define so the user can
 * easily add parameters
 */
#define YY_DECL int yylex YY_PROTO(( void )) 
#else 
/* c++ */
#define ECHO yy___echo()
#define YY_INPUT(buf,result,max_size) \
	if ( yy___input((char *)buf, result,max_size) < 0 ) \
	    YY_FATAL_ERROR( "YY_INPUT() in flex scanner failed" );

#define YY_FATAL_ERROR(msg) yy___fatal_error(msg)
#define yywrap() yy___wrap()

#endif
/***********************************/
/* not to be changed */
#define YY_NULL 0
#define YY_END_OF_BUFFER_CHAR 0
/* special action meaning "start processing a new file" */
#define YY_NEW_FILE yy___newfile 
/* enter a start condition.  This macro really ought to take a parameter,
 * but we do it the disgusting crufty way forced on us by the ()-less
 * definition of BEGIN
 */
#define BEGIN yy_start = 1 + 2 *

/* action number for EOF rule of a given start state */
#define YY_STATE_EOF(state) (YY_END_OF_BUFFER + state + 1)



/* % section 1 definitions go here */ 
#line 1 "mootPPLexer.ll"
#define INITIAL 0
/*-*- Mode: Flex++ -*-*/
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/
/*----------------------------------------------------------------------
 * Name: mootPPLexer.ll
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + preprocessor for the moot tagger
 *   + process with Coetmeur's flex++ to produce 'moot_lexer.cc'
 *----------------------------------------------------------------------*/
/* --- Lexer name --- */
#define YY_mootPPLexer_FLEX_SCANNER
/*----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
#include "mootPPLexer.h"
#line 125 "mootPPLexer.ll"
#line 196 "/usr/local/share/flex++bison++/flexskel.cc"

#define yy___text YY_mootPPLexer_TEXT
#define yy___leng YY_mootPPLexer_LENG
#define yy___in YY_mootPPLexer_IN
#define yy___out YY_mootPPLexer_OUT
#define yy___newfile \
	do \
		{ \
		YY_mootPPLexer_INIT_BUFFER( yy_current_buffer, yy___in ); \
		YY_mootPPLexer_LOAD_BUFFER_STATE(); \
		} \
	while ( 0 )
#if YY_mootPPLexer_DEBUG != 0
#define yy___flex_debug YY_mootPPLexer_DEBUG_FLAG
#endif


#ifdef YY_USE_CLASS

#define yy___echo YY_mootPPLexer_ECHO
#define yy___input YY_mootPPLexer_INPUT
#define yy___fatal_error YY_mootPPLexer_FATAL_ERROR
#define yy___wrap YY_mootPPLexer_WRAP

#endif

/* done after the current pattern has been matched and before the
 * corresponding action - sets up yy___text
 */
#define YY_DO_BEFORE_ACTION \
	yy___text = yy_bp; \
/* % code to fiddle yy___text and yy___leng for yymore() goes here */ \
	yy___leng = yy_cp - yy_bp; \
	yy_hold_char = *yy_cp; \
	*yy_cp = '\0'; \
	yy_c_buf_p = yy_cp;

#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2

/* return all but the first 'n' matched characters back to the input stream */
#define yyless(n) \
	do \
		{ \
		/* undo effects of setting up yy___text */ \
		*yy_cp = yy_hold_char; \
		yy_c_buf_p = yy_cp = yy_bp + n; \
		YY_DO_BEFORE_ACTION; /* set up yy___text again */ \
		} \
	while ( 0 )

#define unput(c) yyunput( c, yy___text )



struct yy_buffer_state
    {
    FILE *yy_input_file;

    YY_mootPPLexer_CHAR *yy_ch_buf;               /* input buffer */
    YY_mootPPLexer_CHAR *yy_buf_pos;      /* current position in input buffer */

    /* size of input buffer in bytes, not including room for EOB characters */
    int yy_buf_size;    

    /* number of characters read into yy_ch_buf, not including EOB characters */
    int yy_n_chars;

    int yy_eof_status;          /* whether we've seen an EOF on this buffer */
#define EOF_NOT_SEEN 0
    /* "pending" happens when the EOF has been seen but there's still
     * some text process
     */
#define EOF_PENDING 1
#define EOF_DONE 2
    };

/* we provide macros for accessing buffer states in case in the
 * future we want to put the buffer states in a more general
 * "scanner state"
 */

#ifndef YY_USE_CLASS

#if YY_mootPPLexer_DEBUG != 0
int YY_mootPPLexer_DEBUG_FLAG=YY_mootPPLexer_DEBUG_INIT;
#endif
#define YY_CURRENT_BUFFER yy_current_buffer
static YY_BUFFER_STATE yy_current_buffer;
/* yy_hold_char holds the character lost when yy___text is formed */
static YY_mootPPLexer_CHAR yy_hold_char;

static int yy_n_chars;          /* number of characters read into yy_ch_buf */

/* GLOBAL */
YY_mootPPLexer_CHAR *yy___text;
int yy___leng;

FILE *yy___in = (FILE *) 0, *yy___out = (FILE *) 0;

#ifdef __cplusplus
static int yyinput YY_PROTO(( void ));
#else
static int input YY_PROTO(( void ));
#endif
/* these variables are all declared out here so that section 3 code can
 * manipulate them
 */
/* points to current character in buffer */
static YY_mootPPLexer_CHAR *yy_c_buf_p = (YY_mootPPLexer_CHAR *) 0;
static int yy_init = 1;         /* whether we need to initialize */
static int yy_start = 0;        /* start state number */

/* flag which is used to allow yywrap()'s to do buffer switches
 * instead of setting up a fresh yy___in.  A bit of a hack ...
 */
static int yy_did_buffer_switch_on_eof;

static int yy_get_next_buffer YY_PROTO(( void ));
static void yyunput YY_PROTO(( YY_mootPPLexer_CHAR c, YY_mootPPLexer_CHAR *buf_ptr ));

#else
/* c++ */
#ifndef YY_mootPPLexer_ECHO_NOCODE
void YY_mootPPLexer_CLASS::yy___echo()
{YY_mootPPLexer_ECHO_CODE
}
#endif
#ifndef YY_mootPPLexer_INPUT_NOCODE
int  YY_mootPPLexer_CLASS::yy___input(char * buffer,int &result,int max_size)
{YY_mootPPLexer_INPUT_CODE
}
#endif
#ifndef YY_mootPPLexer_FATAL_ERROR_NOCODE
void YY_mootPPLexer_CLASS::yy___fatal_error(char *msg)
{YY_mootPPLexer_FATAL_ERROR_CODE
}
#endif
#ifndef YY_mootPPLexer_WRAP_NOCODE
int  YY_mootPPLexer_CLASS::yy___wrap()
{YY_mootPPLexer_WRAP_CODE
}
#endif
void YY_mootPPLexer_CLASS::yy_initialize()
{
 yy___in=0;yy___out=0;yy_init = 1;
 yy_start=0;
 yy___text=0;yy___leng=0;
 yy_current_buffer=0;
 yy_did_buffer_switch_on_eof=0;
 yy_c_buf_p=0;yy_hold_char=0;yy_n_chars=0;
#if YY_mootPPLexer_DEBUG != 0
 YY_mootPPLexer_DEBUG_FLAG=YY_mootPPLexer_DEBUG_INIT;
#endif
}

YY_mootPPLexer_CLASS::YY_mootPPLexer_CLASS(YY_mootPPLexer_CONSTRUCTOR_PARAM) YY_mootPPLexer_CONSTRUCTOR_INIT
{yy_initialize();
 YY_mootPPLexer_CONSTRUCTOR_CODE;
}

#endif


#ifndef YY_USER_ACTION
#define YY_USER_ACTION
#endif

#ifndef YY_USER_INIT
#define YY_USER_INIT
#endif

/* % data tables for the DFA go here */ 
#define YY_END_OF_BUFFER 22
typedef int yy_state_type;
static const short int yy_accept[67] =
    {   0,
        0,    0,   22,   19,   17,   18,   18,   16,   16,   16,
       16,   16,   16,    9,   16,   13,   17,   14,   15,   16,
       16,    0,    0,    0,    0,   16,    9,   10,    0,   16,
       16,    0,    0,    0,   13,    0,    0,    0,    0,    0,
        0,    0,    0,    2,    0,    0,   12,    4,    0,    8,
        7,    0,    0,    3,    0,    0,    0,    0,    0,    0,
        1,   11,    5,    6,    1,    0
    } ;

static const YY_CHAR yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    1,    4,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    2,    5,    6,    7,    6,    6,    8,    6,    6,
        6,    6,    9,   10,   11,   12,   13,   14,   14,   14,
       14,   14,   14,   14,   14,   14,   14,    6,   15,   16,
        6,   17,    5,    6,   18,   18,   18,   18,   18,   18,
       18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
       18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
        6,    6,    6,    6,   19,    6,   20,   18,   18,   21,

       22,   18,   23,   24,   18,   18,   18,   25,   18,   18,
       26,   18,   27,   18,   28,   29,   30,   18,   18,   18,
       18,   18,    6,    6,    6,    6,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    2,
        6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
        6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
        6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
        6,   18,   18,   18,   18,   18,   18,   18,   18,   18,

       18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
       18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
       18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
       18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
       18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
       18,   18,   18,   18,   18
    } ;

static const YY_CHAR yy_meta[31] =
    {   0,
        1,    2,    2,    2,    2,    2,    2,    2,    2,    2,
        3,    4,    2,    5,    2,    2,    4,    6,    4,    6,
        6,    6,    6,    6,    6,    6,    6,    6,    6,    6
    } ;

static const short int yy_base[73] =
    {   0,
        0,    0,  187,  197,  169,  197,  197,   29,   32,   30,
       28,  154,   42,   29,   53,  156,  164,  163,  197,    0,
      150,  143,  132,  130,  128,  142,   38,  138,  136,   79,
       84,  114,  123,    0,  136,   71,   75,   85,   84,   72,
      134,  143,  152,  197,    0,   69,   84,  197,   70,  197,
      197,   64,   74,  197,   21,   73,    0,   74,   73,   70,
      197,    0,  197,  197,  197,  197,  181,  183,   61,  187,
      191,   48
    } ;

static const short int yy_def[73] =
    {   0,
       66,    1,   66,   66,   66,   66,   66,   67,   67,    9,
        9,    9,    9,   66,   66,   68,   66,   66,   66,    9,
        9,   66,   66,   66,   66,    9,   66,   66,   66,   15,
        9,   66,   32,   69,   68,   66,   66,   66,   66,   66,
       70,   70,   32,   66,   32,   32,   69,   66,   66,   66,
       66,   66,   71,   66,   42,   32,   72,   66,   66,   42,
       66,   72,   66,   66,   66,    0,   66,   66,   66,   66,
       66,   66
    } ;

static const short int yy_nxt[228] =
    {   0,
        4,    5,    6,    7,    8,    9,    9,   10,   11,   12,
       11,   13,    9,   14,    9,   15,    9,   16,    9,   16,
       16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
       18,   19,   19,   66,   66,   66,   21,   26,   29,   26,
       29,   27,   27,   18,   19,   19,   41,   29,   60,   29,
       22,   27,   23,   62,   24,   28,   25,   30,   20,   20,
       20,   20,   20,   20,   20,   31,   47,   20,   20,   20,
       32,   20,   32,   32,   33,   32,   32,   32,   32,   32,
       32,   32,   32,   20,   36,   48,   65,   64,   63,   61,
       54,   20,   59,   58,   57,   41,   56,   52,   51,   50,

       32,   41,   49,   41,   41,   42,   41,   41,   41,   41,
       41,   41,   41,   41,   43,   43,   43,   43,   43,   43,
       43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
       44,   45,   43,   45,   45,   45,   45,   45,   45,   45,
       45,   45,   45,   45,   53,   53,   34,   53,   46,   28,
       54,   28,   53,   53,   53,   28,   53,   40,   39,   54,
       38,   53,   37,   36,   18,   17,   34,   28,   55,   43,
       17,   43,   43,   43,   43,   43,   43,   43,   43,   43,
       43,   43,   20,   20,   20,   35,   66,   66,   35,   41,
       41,   41,   41,   53,   53,   53,    3,   66,   66,   66,

       66,   66,   66,   66,   66,   66,   66,   66,   66,   66,
       66,   66,   66,   66,   66,   66,   66,   66,   66,   66,
       66,   66,   66,   66,   66,   66,   66
    } ;

static const short int yy_chk[228] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        8,    8,    8,    9,    9,    9,   10,   11,   14,   11,
       14,   11,   14,   13,   13,   13,   55,   27,   55,   27,
       10,   27,   10,   72,   10,   13,   10,   15,   15,   15,
       15,   15,   15,   15,   15,   15,   69,   15,   15,   15,
       15,   15,   15,   15,   15,   15,   15,   15,   15,   15,
       15,   15,   15,   30,   36,   36,   60,   59,   58,   56,
       53,   30,   52,   49,   47,   60,   46,   40,   39,   38,

       30,   31,   37,   31,   31,   31,   31,   31,   31,   31,
       31,   31,   31,   31,   32,   32,   32,   32,   32,   32,
       32,   32,   32,   32,   32,   32,   32,   32,   32,   32,
       32,   32,   32,   32,   32,   32,   32,   32,   32,   32,
       32,   32,   32,   32,   41,   41,   35,   41,   33,   29,
       41,   28,   41,   42,   42,   26,   42,   25,   24,   42,
       23,   42,   22,   21,   18,   17,   16,   12,   42,   43,
        5,   43,   43,   43,   43,   43,   43,   43,   43,   43,
       43,   43,   67,   67,   67,   68,    3,    0,   68,   70,
       70,   70,   70,   71,   71,   71,   66,   66,   66,   66,

       66,   66,   66,   66,   66,   66,   66,   66,   66,   66,
       66,   66,   66,   66,   66,   66,   66,   66,   66,   66,
       66,   66,   66,   66,   66,   66,   66
    } ;

static yy_state_type yy_last_accepting_state;
static YY_CHAR *yy_last_accepting_cpos;

#if YY_mootPPLexer_DEBUG != 0
static const short int yy_rule_linenum[21] =
    {   0,
      134,  138,  142,  147,  158,  159,  160,  161,  163,  164,
      166,  167,  168,  170,  171,  172,  174,  175,  176,  176
    } ;

#endif
/* the intent behind this definition is that it'll catch
 * any uses of REJECT which flex missed
 */
#define REJECT reject_used_but_not_detected
#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0
#line 369 "/usr/local/share/flex++bison++/flexskel.cc"
#ifndef YY_USE_CLASS
static yy_state_type yy_get_previous_state YY_PROTO(( void ));
static yy_state_type yy_try_NUL_trans YY_PROTO(( yy_state_type current_state ));
#else
#define yy_get_previous_state() ((yy_state_type)(yy_get_previous_state_()))
#define yy_try_NUL_trans(c) ((yy_state_type)(yy_try_NUL_trans_(c)))
#endif

#ifndef YY_USE_CLASS
#ifdef YY_mootPPLexer_LEX_DEFINED
YY_mootPPLexer_LEX_RETURN YY_mootPPLexer_LEX ( YY_mootPPLexer_LEX_PARAM )
YY_mootPPLexer_LEX_PARAM_DEF
#else
YY_DECL
#endif
#else
YY_mootPPLexer_LEX_RETURN YY_mootPPLexer_CLASS::YY_mootPPLexer_LEX ( YY_mootPPLexer_LEX_PARAM)

#endif
    {
    register yy_state_type yy_current_state;
    register YY_mootPPLexer_CHAR *yy_cp, *yy_bp;
    register int yy_act;

/* % user's declarations go here */ 


/* % end of prolog */ 
#line 394 "/usr/local/share/flex++bison++/flexskel.cc"

    if ( yy_init )
	{
	 
	 {
	 YY_USER_INIT;
	 }
	if ( ! yy_start )
	    yy_start = 1;       /* first start state */

	if ( ! yy___in )
	    yy___in = stdin;

	if ( ! yy___out )
	    yy___out = stdout;

	if ( yy_current_buffer )
	    YY_mootPPLexer_INIT_BUFFER( yy_current_buffer, yy___in );
	else
	    yy_current_buffer = YY_mootPPLexer_CREATE_BUFFER( yy___in, YY_BUF_SIZE );

	YY_mootPPLexer_LOAD_BUFFER_STATE();
	yy_init=0;
	}

    while ( 1 )         /* loops until end-of-file is reached */
	{
/* % yymore()-related code goes here */ 
#line 422 "/usr/local/share/flex++bison++/flexskel.cc"
	yy_cp = yy_c_buf_p;

	/* support of yy___text */
	*yy_cp = yy_hold_char;

	/* yy_bp points to the position in yy_ch_buf of the start of the
	 * current run.
	 */
	yy_bp = yy_cp;

/* % code to set up and find next match goes here */ 
	yy_current_state = yy_start;
yy_match:
	do
	    {
	    register YY_CHAR yy_c = yy_ec[*yy_cp];
	    if ( yy_accept[yy_current_state] )
		{
		yy_last_accepting_state = yy_current_state;
		yy_last_accepting_cpos = yy_cp;
		}
	    while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
		{
		yy_current_state = yy_def[yy_current_state];
		if ( yy_current_state >= 67 )
		    yy_c = yy_meta[yy_c];
		}
	    yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
	    ++yy_cp;
	    }
	while ( yy_current_state != 66 );
	yy_cp = yy_last_accepting_cpos;
	yy_current_state = yy_last_accepting_state;
#line 433 "/usr/local/share/flex++bison++/flexskel.cc"

yy_find_action:
/* % code to find the action number goes here */ 
	yy_act = yy_accept[yy_current_state];
#line 436 "/usr/local/share/flex++bison++/flexskel.cc"

	YY_DO_BEFORE_ACTION;
	YY_USER_ACTION;

do_action:      /* this label is used only to access EOF actions */
#if YY_mootPPLexer_DEBUG != 0
	if ( yy___flex_debug )
		{
		if ( yy_act == 0 )
			fprintf( stderr, "--scanner backtracking\n" );
		else if ( yy_act < YY_END_OF_BUFFER -1 )
			fprintf( stderr, 
			 "--accepting rule at line %d (\"%s\")\n",
			 yy_rule_linenum[yy_act], yy___text );
		else if ( yy_act == YY_END_OF_BUFFER -1 )
			fprintf( stderr, 
			 "--accepting default rule (\"%s\")\n",
			 yy___text );
		else if ( yy_act == YY_END_OF_BUFFER )
			fprintf( stderr, "--(end of buffer or a NUL)\n" );
		else
			fprintf( stderr, "--EOF\n" );
		}
#endif
	switch ( yy_act )
	    {
/* % actions go here */ 
	    case 0: /* must backtrack */
	    /* undo the effects of YY_DO_BEFORE_ACTION */
	    *yy_cp = yy_hold_char;
	    yy_cp = yy_last_accepting_cpos;
	    yy_current_state = yy_last_accepting_state;
	    goto yy_find_action;


/*----------------------------------------------------------------------
 * XML markup removal 
 */
//int xml_c_tmp;

case 1:
#line 134 "mootPPLexer.ll"
{
  theColumn += yyleng;
  return EOS;
}
	YY_BREAK
case 2:
#line 138 "mootPPLexer.ll"
{
  theColumn += yyleng;
  return XML_START_TAG;
}
	YY_BREAK
case 3:
#line 142 "mootPPLexer.ll"
{
  theColumn += yyleng;
  return XML_END_TAG;
}
	YY_BREAK
case 4:
#line 147 "mootPPLexer.ll"
{
  theColumn += yyleng;
  //-- character entity: translate (weird!)
  /*
  theColumn += yyleng;
  sscanf(yytext+2, "%d", &xml_c_tmp);
  yytext[0] = (unsigned char)xml_c_tmp;
  yytext[1] = '\0';
  */
  return XML_ENTITY;
}
	YY_BREAK
case 5:
#line 158 "mootPPLexer.ll"
{ theColumn += yyleng; return XML_ENTITY; }
	YY_BREAK
case 6:
#line 159 "mootPPLexer.ll"
{ theColumn += yyleng; return XML_ENTITY; }
	YY_BREAK
case 7:
#line 160 "mootPPLexer.ll"
{ theColumn += yyleng; return XML_ENTITY; }
	YY_BREAK
case 8:
#line 161 "mootPPLexer.ll"
{ theColumn += yyleng; return XML_ENTITY; }
	YY_BREAK
case 9:
#line 163 "mootPPLexer.ll"
{ theColumn += yyleng; return INTEGER; }
	YY_BREAK
case 10:
#line 164 "mootPPLexer.ll"
{ theColumn += yyleng; return FLOAT; }
	YY_BREAK
case 11:
#line 166 "mootPPLexer.ll"
{ theColumn += yyleng; return WORD; }
	YY_BREAK
case 12:
#line 167 "mootPPLexer.ll"
{ theColumn += yyleng; return WORD; }
	YY_BREAK
case 13:
#line 168 "mootPPLexer.ll"
{ theColumn += yyleng; return WORD; }
	YY_BREAK
case 14:
*yy_cp = yy_hold_char; /* undo effects of setting up yytext */
yy_c_buf_p = yy_cp = yy_bp + 1;
YY_DO_BEFORE_ACTION; /* set up yytext again */
#line 170 "mootPPLexer.ll"
{ theColumn++; return EOS; }
	YY_BREAK
case 15:
*yy_cp = yy_hold_char; /* undo effects of setting up yytext */
yy_c_buf_p = yy_cp = yy_bp + 1;
YY_DO_BEFORE_ACTION; /* set up yytext again */
#line 171 "mootPPLexer.ll"
{ theColumn++; return EOS; }
	YY_BREAK
case 16:
#line 172 "mootPPLexer.ll"
{ theColumn+=yyleng; return PUNCT; }
	YY_BREAK
case 17:
#line 174 "mootPPLexer.ll"
{ theColumn+=yyleng; /* do nothing */ }
	YY_BREAK
case 18:
#line 175 "mootPPLexer.ll"
{ theLine++; theColumn=0; }
	YY_BREAK
case 19:
#line 176 "mootPPLexer.ll"
{ theColumn++; return UNKNOWN; }
	YY_BREAK
case YY_STATE_EOF(INITIAL):
#line 178 "mootPPLexer.ll"
{ return PPEOF; }
	YY_BREAK
case 21:
#line 180 "mootPPLexer.ll"
ECHO;
	YY_BREAK
#line 463 "/usr/local/share/flex++bison++/flexskel.cc"

	    case YY_END_OF_BUFFER:
		{
		/* amount of text matched not including the EOB char */
		int yy_amount_of_matched_text = yy_cp - yy___text - 1;

		/* undo the effects of YY_DO_BEFORE_ACTION */
		*yy_cp = yy_hold_char;

		/* note that here we test for yy_c_buf_p "<=" to the position
		 * of the first EOB in the buffer, since yy_c_buf_p will
		 * already have been incremented past the NUL character
		 * (since all states make transitions on EOB to the end-
		 * of-buffer state).  Contrast this with the test in yyinput().
		 */
		if ( yy_c_buf_p <= &yy_current_buffer->yy_ch_buf[yy_n_chars] )
		    /* this was really a NUL */
		    {
		    yy_state_type yy_next_state;

		    yy_c_buf_p = yy___text + yy_amount_of_matched_text;

		    yy_current_state = yy_get_previous_state();

		    /* okay, we're now positioned to make the
		     * NUL transition.  We couldn't have
		     * yy_get_previous_state() go ahead and do it
		     * for us because it doesn't know how to deal
		     * with the possibility of jamming (and we
		     * don't want to build jamming into it because
		     * then it will run more slowly)
		     */

		    yy_next_state = yy_try_NUL_trans( yy_current_state );

		    yy_bp = yy___text + YY_MORE_ADJ;

		    if ( yy_next_state )
			{
			/* consume the NUL */
			yy_cp = ++yy_c_buf_p;
			yy_current_state = yy_next_state;
			goto yy_match;
			}

		    else
			{
/* % code to do backtracking for compressed tables and set up yy_cp goes here */ 
			    yy_cp = yy_last_accepting_cpos;
			    yy_current_state = yy_last_accepting_state;
#line 511 "/usr/local/share/flex++bison++/flexskel.cc"
			goto yy_find_action;
			}
		    }

		else switch ( yy_get_next_buffer() )
		    {
		    case EOB_ACT_END_OF_FILE:
			{
			yy_did_buffer_switch_on_eof = 0;

			if ( yywrap() )
			    {
			    /* note: because we've taken care in
			     * yy_get_next_buffer() to have set up yy___text,
			     * we can now set up yy_c_buf_p so that if some
			     * total hoser (like flex itself) wants
			     * to call the scanner after we return the
			     * YY_NULL, it'll still work - another YY_NULL
			     * will get returned.
			     */
			    yy_c_buf_p = yy___text + YY_MORE_ADJ;

			    yy_act = YY_STATE_EOF((yy_start - 1) / 2);
			    goto do_action;
			    }

			else
			    {
			    if ( ! yy_did_buffer_switch_on_eof )
				YY_NEW_FILE;
			    }
			}
			break;

		    case EOB_ACT_CONTINUE_SCAN:
			yy_c_buf_p = yy___text + yy_amount_of_matched_text;

			yy_current_state = yy_get_previous_state();

			yy_cp = yy_c_buf_p;
			yy_bp = yy___text + YY_MORE_ADJ;
			goto yy_match;

		    case EOB_ACT_LAST_MATCH:
			yy_c_buf_p =
			    &yy_current_buffer->yy_ch_buf[yy_n_chars];

			yy_current_state = yy_get_previous_state();

			yy_cp = yy_c_buf_p;
			yy_bp = yy___text + YY_MORE_ADJ;
			goto yy_find_action;
		    }
		break;
		}

	    default:
#if YY_mootPPLexer_DEBUG != 0
		fprintf(stderr, "action # %d\n", yy_act );
#endif
		YY_FATAL_ERROR(
			"fatal flex scanner internal error--no action found" );
	    }
	}
	yyterminate();/* avoid the no return value error message on MS-C7/dos */
    }


/* yy_get_next_buffer - try to read in a new buffer
 *
 * synopsis
 *     int yy_get_next_buffer();
 *     
 * returns a code representing an action
 *     EOB_ACT_LAST_MATCH - 
 *     EOB_ACT_CONTINUE_SCAN - continue scanning from current position
 *     EOB_ACT_END_OF_FILE - end of file
 */
#ifndef YY_USE_CLASS
static int yy_get_next_buffer()
#else
int YY_mootPPLexer_CLASS::yy_get_next_buffer()
#endif
    {
    register YY_mootPPLexer_CHAR *dest = yy_current_buffer->yy_ch_buf;
    register YY_mootPPLexer_CHAR *source = yy___text - 1; /* copy prev. char, too */
    register int number_to_move, i;
    int ret_val;

    if ( yy_c_buf_p > &yy_current_buffer->yy_ch_buf[yy_n_chars + 1] )
	YY_FATAL_ERROR(
		"fatal flex scanner internal error--end of buffer missed" );

    /* try to read more data */

    /* first move last chars to start of buffer */
    number_to_move = yy_c_buf_p - yy___text;

    for ( i = 0; i < number_to_move; ++i )
	*(dest++) = *(source++);

    if ( yy_current_buffer->yy_eof_status != EOF_NOT_SEEN )
	/* don't do the read, it's not guaranteed to return an EOF,
	 * just force an EOF
	 */
	yy_n_chars = 0;

    else
	{
	int num_to_read = yy_current_buffer->yy_buf_size - number_to_move - 1;

	if ( num_to_read > YY_READ_BUF_SIZE )
	    num_to_read = YY_READ_BUF_SIZE;

	else if ( num_to_read <= 0 )
	    YY_FATAL_ERROR( "fatal error - scanner input buffer overflow" );

	/* read in more data */
	YY_INPUT( (&yy_current_buffer->yy_ch_buf[number_to_move]),
		  yy_n_chars, num_to_read );
	}

    if ( yy_n_chars == 0 )
	{
	if ( number_to_move - YY_MORE_ADJ == 1 )
	    {
	    ret_val = EOB_ACT_END_OF_FILE;
	    yy_current_buffer->yy_eof_status = EOF_DONE;
	    }

	else
	    {
	    ret_val = EOB_ACT_LAST_MATCH;
	    yy_current_buffer->yy_eof_status = EOF_PENDING;
	    }
	}

    else
	ret_val = EOB_ACT_CONTINUE_SCAN;

    yy_n_chars += number_to_move;
    yy_current_buffer->yy_ch_buf[yy_n_chars] = YY_END_OF_BUFFER_CHAR;
    yy_current_buffer->yy_ch_buf[yy_n_chars + 1] = YY_END_OF_BUFFER_CHAR;

    /* yy___text begins at the second character in yy_ch_buf; the first
     * character is the one which preceded it before reading in the latest
     * buffer; it needs to be kept around in case it's a newline, so
     * yy_get_previous_state() will have with '^' rules active
     */

    yy___text = &yy_current_buffer->yy_ch_buf[1];

    return ( ret_val );
    }


/* yy_get_previous_state - get the state just before the EOB char was reached
 *
 * synopsis
 *     yy_state_type yy_get_previous_state();
 */

#ifndef YY_USE_CLASS
static yy_state_type yy_get_previous_state()
#else
long YY_mootPPLexer_CLASS::yy_get_previous_state_()
#endif
    {
    register yy_state_type yy_current_state;
    register YY_mootPPLexer_CHAR *yy_cp;

/* % code to get the start state into yy_current_state goes here */ 
    yy_current_state = yy_start;
#line 683 "/usr/local/share/flex++bison++/flexskel.cc"

    for ( yy_cp = yy___text + YY_MORE_ADJ; yy_cp < yy_c_buf_p; ++yy_cp )
	{
/* % code to find the next state goes here */ 
	register YY_CHAR yy_c = (*yy_cp ? yy_ec[*yy_cp] : 1);
	if ( yy_accept[yy_current_state] )
	    {
	    yy_last_accepting_state = yy_current_state;
	    yy_last_accepting_cpos = yy_cp;
	    }
	while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
	    {
	    yy_current_state = yy_def[yy_current_state];
	    if ( yy_current_state >= 67 )
		yy_c = yy_meta[yy_c];
	    }
	yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
#line 687 "/usr/local/share/flex++bison++/flexskel.cc"
	}

#ifndef YY_USE_CLASS
    return ( yy_current_state );
#else
    return (long)( yy_current_state );
#endif
    }


/* yy_try_NUL_trans - try to make a transition on the NUL character
 *
 * synopsis
 *     next_state = yy_try_NUL_trans( current_state );
 */

#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
static yy_state_type yy_try_NUL_trans( register yy_state_type yy_current_state )
#else
static yy_state_type yy_try_NUL_trans( yy_current_state )
register yy_state_type yy_current_state;
#endif
#else
long YY_mootPPLexer_CLASS::yy_try_NUL_trans_(long yy_current_state_)
#endif

    {
#ifndef YY_USE_CLASS
#else
    yy_state_type yy_current_state=(yy_state_type)yy_current_state_;
#endif
    register int yy_is_jam;
/* % code to find the next state, and perhaps do backtracking, goes here */ 
    register YY_CHAR *yy_cp = yy_c_buf_p;

    register YY_CHAR yy_c = 1;
    if ( yy_accept[yy_current_state] )
	{
	yy_last_accepting_state = yy_current_state;
	yy_last_accepting_cpos = yy_cp;
	}
    while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
	{
	yy_current_state = yy_def[yy_current_state];
	if ( yy_current_state >= 67 )
	    yy_c = yy_meta[yy_c];
	}
    yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
    yy_is_jam = (yy_current_state == 66);
#line 721 "/usr/local/share/flex++bison++/flexskel.cc"

#ifndef YY_USE_CLASS
    return ( yy_is_jam ? 0 : yy_current_state );
#else
    return (long)( yy_is_jam ? 0 : yy_current_state );
#endif
    }

#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
static void yyunput( YY_mootPPLexer_CHAR c, register YY_mootPPLexer_CHAR *yy_bp )
#else
static void yyunput( c, yy_bp )
YY_mootPPLexer_CHAR c;
register YY_mootPPLexer_CHAR *yy_bp;
#endif
#else
void YY_mootPPLexer_CLASS::yyunput( YY_mootPPLexer_CHAR c, YY_mootPPLexer_CHAR *yy_bp )
#endif

    {
    register YY_mootPPLexer_CHAR *yy_cp = yy_c_buf_p;

    /* undo effects of setting up yy___text */
    *yy_cp = yy_hold_char;

    if ( yy_cp < yy_current_buffer->yy_ch_buf + 2 )
	{ /* need to shift things up to make room */
	register int number_to_move = yy_n_chars + 2; /* +2 for EOB chars */
	register YY_mootPPLexer_CHAR *dest =
	    &yy_current_buffer->yy_ch_buf[yy_current_buffer->yy_buf_size + 2];
	register YY_mootPPLexer_CHAR *source =
	    &yy_current_buffer->yy_ch_buf[number_to_move];

	while ( source > yy_current_buffer->yy_ch_buf )
	    *--dest = *--source;

	yy_cp += dest - source;
	yy_bp += dest - source;
	yy_n_chars = yy_current_buffer->yy_buf_size;

	if ( yy_cp < yy_current_buffer->yy_ch_buf + 2 )
	    YY_FATAL_ERROR( "flex scanner push-back overflow" );
	}

    if ( yy_cp > yy_bp && yy_cp[-1] == '\n' )
	yy_cp[-2] = '\n';

    *--yy_cp = c;

    /* note: the formal parameter *must* be called "yy_bp" for this
     *       macro to now work correctly
     */
    YY_DO_BEFORE_ACTION; /* set up yy___text again */
    }

#ifndef YY_USE_CLASS
#ifdef __cplusplus
static int yyinput()
#else
static int input()
#endif
#else
int YY_mootPPLexer_CLASS::input()
#endif
    {
    int c;
    YY_mootPPLexer_CHAR *yy_cp = yy_c_buf_p;

    *yy_cp = yy_hold_char;

    if ( *yy_c_buf_p == YY_END_OF_BUFFER_CHAR )
	{
	/* yy_c_buf_p now points to the character we want to return.
	 * If this occurs *before* the EOB characters, then it's a
	 * valid NUL; if not, then we've hit the end of the buffer.
	 */
	if ( yy_c_buf_p < &yy_current_buffer->yy_ch_buf[yy_n_chars] )
	    /* this was really a NUL */
	    *yy_c_buf_p = '\0';

	else
	    { /* need more input */
	    yy___text = yy_c_buf_p;
	    ++yy_c_buf_p;

	    switch ( yy_get_next_buffer() )
		{
		case EOB_ACT_END_OF_FILE:
		    {
		    if ( yywrap() )
			{
			yy_c_buf_p = yy___text + YY_MORE_ADJ;
			return ( EOF );
			}

		    YY_NEW_FILE;
#ifndef YY_USE_CLASS
#ifdef __cplusplus
		    return ( yyinput() );
#else
		    return ( input() );
#endif
#else
		    return ( input() );
#endif
		    }
		    break;

		case EOB_ACT_CONTINUE_SCAN:
		    yy_c_buf_p = yy___text + YY_MORE_ADJ;
		    break;

		case EOB_ACT_LAST_MATCH:
#ifndef YY_USE_CLASS
#ifdef __cplusplus
		    YY_FATAL_ERROR( "unexpected last match in yyinput()" );
#else
		    YY_FATAL_ERROR( "unexpected last match in input()" );
#endif
#else
		    YY_FATAL_ERROR( "unexpected last match in YY_mootPPLexer_CLASS::input()" );
#endif
		}
	    }
	}

    c = *yy_c_buf_p;
    yy_hold_char = *++yy_c_buf_p;

    return ( c );
    }


#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
void YY_mootPPLexer_RESTART( FILE *input_file )
#else
void YY_mootPPLexer_RESTART( input_file )
FILE *input_file;
#endif
#else
void YY_mootPPLexer_CLASS::YY_mootPPLexer_RESTART ( FILE *input_file )
#endif

    {
    YY_mootPPLexer_INIT_BUFFER( yy_current_buffer, input_file );
    YY_mootPPLexer_LOAD_BUFFER_STATE();
    }


#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
void YY_mootPPLexer_SWITCH_TO_BUFFER( YY_BUFFER_STATE new_buffer )
#else
void YY_mootPPLexer_SWITCH_TO_BUFFER( new_buffer )
YY_BUFFER_STATE new_buffer;
#endif
#else
void YY_mootPPLexer_CLASS::YY_mootPPLexer_SWITCH_TO_BUFFER( YY_BUFFER_STATE new_buffer )
#endif

    {
    if ( yy_current_buffer == new_buffer )
	return;

    if ( yy_current_buffer )
	{
	/* flush out information for old buffer */
	*yy_c_buf_p = yy_hold_char;
	yy_current_buffer->yy_buf_pos = yy_c_buf_p;
	yy_current_buffer->yy_n_chars = yy_n_chars;
	}

    yy_current_buffer = new_buffer;
    YY_mootPPLexer_LOAD_BUFFER_STATE();

    /* we don't actually know whether we did this switch during
     * EOF (yywrap()) processing, but the only time this flag
     * is looked at is after yywrap() is called, so it's safe
     * to go ahead and always set it.
     */
    yy_did_buffer_switch_on_eof = 1;
    }


#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
void YY_mootPPLexer_LOAD_BUFFER_STATE( void )
#else
void YY_mootPPLexer_LOAD_BUFFER_STATE()
#endif
#else
void YY_mootPPLexer_CLASS::YY_mootPPLexer_LOAD_BUFFER_STATE(  )
#endif

    {
    yy_n_chars = yy_current_buffer->yy_n_chars;
    yy___text = yy_c_buf_p = yy_current_buffer->yy_buf_pos;
    yy___in = yy_current_buffer->yy_input_file;
    yy_hold_char = *yy_c_buf_p;
    }


#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
YY_BUFFER_STATE YY_mootPPLexer_CREATE_BUFFER( FILE *file, int size )
#else
YY_BUFFER_STATE YY_mootPPLexer_CREATE_BUFFER( file, size )
FILE *file;
int size;
#endif
#else
YY_BUFFER_STATE YY_mootPPLexer_CLASS::YY_mootPPLexer_CREATE_BUFFER( FILE *file, int size )
#endif

    {
    YY_BUFFER_STATE b;

    b = (YY_BUFFER_STATE) malloc( sizeof( struct yy_buffer_state ) );

    if ( ! b )
	YY_FATAL_ERROR( "out of dynamic memory in YY_mootPPLexer_CREATE_BUFFER()" );

    b->yy_buf_size = size;

    /* yy_ch_buf has to be 2 characters longer than the size given because
     * we need to put in 2 end-of-buffer characters.
     */
    b->yy_ch_buf = (YY_mootPPLexer_CHAR *) malloc( (unsigned) (b->yy_buf_size + 2) );

    if ( ! b->yy_ch_buf )
	YY_FATAL_ERROR( "out of dynamic memory in YY_mootPPLexer_CREATE_BUFFER()" );

    YY_mootPPLexer_INIT_BUFFER( b, file );

    return ( b );
    }


#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
void YY_mootPPLexer_DELETE_BUFFER( YY_BUFFER_STATE b )
#else
void YY_mootPPLexer_DELETE_BUFFER( b )
YY_BUFFER_STATE b;
#endif
#else
void YY_mootPPLexer_CLASS::YY_mootPPLexer_DELETE_BUFFER( YY_BUFFER_STATE b )
#endif

    {
    if ( b == yy_current_buffer )
	yy_current_buffer = (YY_BUFFER_STATE) 0;

    free( (char *) b->yy_ch_buf );
    free( (char *) b );
    }


#ifndef YY_USE_CLASS
#ifdef YY_USE_PROTOS
void YY_mootPPLexer_INIT_BUFFER( YY_BUFFER_STATE b, FILE *file )
#else
void YY_mootPPLexer_INIT_BUFFER( b, file )
YY_BUFFER_STATE b;
FILE *file;
#endif
#else
void YY_mootPPLexer_CLASS::YY_mootPPLexer_INIT_BUFFER( YY_BUFFER_STATE b, FILE *file)
#endif

    {
    b->yy_input_file = file;

    /* we put in the '\n' and start reading from [1] so that an
     * initial match-at-newline will be true.
     */

    b->yy_ch_buf[0] = '\n';
    b->yy_n_chars = 1;

    /* we always need two end-of-buffer characters.  The first causes
     * a transition to the end-of-buffer state.  The second causes
     * a jam in that state.
     */
    b->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;
    b->yy_ch_buf[2] = YY_END_OF_BUFFER_CHAR;

    b->yy_buf_pos = &b->yy_ch_buf[1];

    b->yy_eof_status = EOF_NOT_SEEN;
    }
#line 180 "mootPPLexer.ll"

/*----------------------------------------------------------------------
 * moot::GenericLexer requirements
 */
void mootPPLexer::mgl_begin(int stateno) { BEGIN(stateno); }
