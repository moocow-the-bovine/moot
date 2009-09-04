#ifndef FLEX_HEADER_mootLexfreqsLexer_h
#define FLEX_HEADER_mootLexfreqsLexer_h
#define YY_mootLexfreqsLexer_CHAR unsigned char
#line 1 "./flexskel.h"
/* A lexical scanner header generated by flex */
/* MODIFIED FOR C++ CLASS BY Alain Coetmeur: coetmeur(at)icdc.fr */
/* Note that (at) mean the 'at' symbol that I cannot write */
/* because it is expanded to the class name */
/* made at Informatique-CDC, Research&development department */
/* company from the Caisse Des Depots et Consignations */


/*********************************************/
/* SYSTEM dependent declaration, includes... */
/*********************************************/
/* cfront 1.2 defines "c_plusplus" instead of "__cplusplus" */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_PROTOS
#define YY_USE_PROTOS
#endif
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else	/* ! __cplusplus */
#ifdef __STDC__
#ifdef __GNUC__
#else
#endif	/* __GNUC__ */
#ifndef YY_USE_PROTOS
#define YY_USE_PROTOS
#endif
#endif	/* __STDC__ */
#endif	/* ! __cplusplus */
/*********************************************/
/* COMPILER DEPENDENT   MACROS               */
/*********************************************/
/* use prototypes in function declarations */
#ifndef YY_PROTO
#ifdef YY_USE_PROTOS
#define YY_PROTO(proto) proto
#else
#define YY_PROTO(proto) ()
#endif
#endif
#include <stdio.h>




/* % here is the declaration from section1 %header{  */ 
#line 33 "mootLexfreqsLexer.ll"

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
\class mootLexfreqsLexer
\brief \c flex++ lexer for (TnT-style) moot lexical-frequency parameter files
\details Supports comments introduced with '%%'.

\file mootLexfreqsLexer.h
\brief \c flex++ lexical-frequency parameter file lexer: autogenerated header

\file mootLexfreqsLexer.ll
\brief \c flex++ lexical-frequency parameter file lexer: sources
*/

#include <mootLexfreqsParser.h>
#include <mootGenericLexer.h>

using namespace moot;
#line 55 "mootLexfreqsLexer.ll"
#define YY_mootLexfreqsLexer_LEX_PARAM  \
  YY_mootLexfreqsParser_STYPE *yylval, YY_mootLexfreqsParser_LTYPE *yylloc
#line 58 "mootLexfreqsLexer.ll"
#define YY_mootLexfreqsLexer_CLASS  mootLexfreqsLexer
#line 60 "mootLexfreqsLexer.ll"
#define YY_mootLexfreqsLexer_INHERIT  \
  : public GenericLexer
#line 63 "mootLexfreqsLexer.ll"
#define YY_mootLexfreqsLexer_INPUT_CODE  \
  return moot::GenericLexer::yyinput(buffer,result,max_size);
#line 66 "mootLexfreqsLexer.ll"
#define YY_mootLexfreqsLexer_MEMBERS  \
  public: \
    /** virtual destructor to shut up gcc */\
    virtual ~mootLexfreqsLexer(void) {};\
  /* moot::GenericLexer helpers */ \
  virtual void  *mgl_yy_current_buffer_p(void) \
                 {return (&yy_current_buffer);};\
  virtual void  *mgl_yy_create_buffer(int size, FILE *unused=stdin) \
                 {return (void*)(yy_create_buffer(unused,size));};\
  virtual void   mgl_yy_init_buffer(void *buf, FILE *unused=stdin) \
                 {yy_init_buffer((YY_BUFFER_STATE)buf,unused);};\
  virtual void   mgl_yy_delete_buffer(void *buf) \
                 {yy_delete_buffer((YY_BUFFER_STATE)buf);};\
  virtual void   mgl_yy_switch_to_buffer(void *buf) \
                 {yy_switch_to_buffer((YY_BUFFER_STATE)buf);};\
  virtual void   mgl_begin(int stateno);
#line 84 "mootLexfreqsLexer.ll"
#define YY_mootLexfreqsLexer_CONSTRUCTOR_INIT  :\
  GenericLexer("mootLexfreqsLexer")
#line 100 "mootLexfreqsLexer.ll"
#line 52 "./flexskel.h"



#ifndef YY_mootLexfreqsLexer_TEXT
#define YY_mootLexfreqsLexer_TEXT yytext
#endif
#ifndef YY_mootLexfreqsLexer_LENG
#define YY_mootLexfreqsLexer_LENG yyleng
#endif
#ifndef YY_mootLexfreqsLexer_IN
#define YY_mootLexfreqsLexer_IN yyin
#endif
#ifndef YY_mootLexfreqsLexer_OUT
#define YY_mootLexfreqsLexer_OUT yyout
#endif
#ifndef YY_mootLexfreqsLexer_LEX_RETURN
#define YY_mootLexfreqsLexer_LEX_RETURN int
#else
#ifndef YY_mootLexfreqsLexer_LEX_DEFINED
#define YY_mootLexfreqsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootLexfreqsLexer_LEX
#define YY_mootLexfreqsLexer_LEX yylex
#else
#ifndef YY_mootLexfreqsLexer_LEX_DEFINED
#define YY_mootLexfreqsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootLexfreqsLexer_LEX_PARAM
#ifndef YY_USE_PROTOS
#define YY_mootLexfreqsLexer_LEX_PARAM 
#else
#define YY_mootLexfreqsLexer_LEX_PARAM void
#endif
#else
#ifndef YY_mootLexfreqsLexer_LEX_DEFINED
#define YY_mootLexfreqsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootLexfreqsLexer_LEX_PARAM_DEF
#define YY_mootLexfreqsLexer_LEX_PARAM_DEF
#else
#ifndef YY_mootLexfreqsLexer_LEX_DEFINED
#define YY_mootLexfreqsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootLexfreqsLexer_RESTART
#define YY_mootLexfreqsLexer_RESTART yyrestart
#endif
#ifndef YY_mootLexfreqsLexer_SWITCH_TO_BUFFER
#define YY_mootLexfreqsLexer_SWITCH_TO_BUFFER yy_switch_to_buffer
#endif
#ifndef YY_mootLexfreqsLexer_LOAD_BUFFER_STATE
#define YY_mootLexfreqsLexer_LOAD_BUFFER_STATE yy_load_buffer_state
#endif

#ifndef YY_mootLexfreqsLexer_CREATE_BUFFER
#define YY_mootLexfreqsLexer_CREATE_BUFFER yy_create_buffer
#ifndef YY_USE_CLASS
#ifndef yy_new_buffer
#define yy_new_buffer yy_create_buffer
#endif
#endif
#endif
#ifndef YY_mootLexfreqsLexer_DELETE_BUFFER
#define YY_mootLexfreqsLexer_DELETE_BUFFER yy_delete_buffer
#endif
#ifndef YY_mootLexfreqsLexer_INIT_BUFFER
#define YY_mootLexfreqsLexer_INIT_BUFFER yy_init_buffer
#endif
#ifdef YY_mootLexfreqsLexer_FLEX_DEBUG
#ifndef YY_mootLexfreqsLexer_DEBUG
#define YY_mootLexfreqsLexer_DEBUG 1
#endif
#else
#ifndef YY_mootLexfreqsLexer_DEBUG
#define YY_mootLexfreqsLexer_DEBUG 0
#endif
#endif

#if YY_mootLexfreqsLexer_DEBUG != 0
#ifndef YY_mootLexfreqsLexer_DEBUG_FLAG
#define YY_mootLexfreqsLexer_DEBUG_FLAG yy_flex_debug
#endif
#ifndef YY_mootLexfreqsLexer_DEBUG_INIT
#define YY_mootLexfreqsLexer_DEBUG_INIT 1
#endif
#endif




#ifndef YY_USE_CLASS
typedef struct yy_buffer_state *YY_BUFFER_STATE;

extern void YY_mootLexfreqsLexer_RESTART YY_PROTO(( FILE *input_file ));
extern void YY_mootLexfreqsLexer_SWITCH_TO_BUFFER YY_PROTO(( YY_BUFFER_STATE new_buffer ));
extern void YY_mootLexfreqsLexer_LOAD_BUFFER_STATE YY_PROTO(( void ));
extern YY_BUFFER_STATE YY_mootLexfreqsLexer_CREATE_BUFFER YY_PROTO(( FILE *file, int size ));
extern void YY_mootLexfreqsLexer_DELETE_BUFFER YY_PROTO(( YY_BUFFER_STATE b ));
extern void YY_mootLexfreqsLexer_INIT_BUFFER YY_PROTO(( YY_BUFFER_STATE b, FILE *file ));

#if YY_mootLexfreqsLexer_DEBUG != 0
extern int YY_mootLexfreqsLexer_DEBUG_FLAG ;
#endif
extern YY_mootLexfreqsLexer_CHAR  *YY_mootLexfreqsLexer_TEXT;
extern int YY_mootLexfreqsLexer_LENG;
extern FILE *YY_mootLexfreqsLexer_IN, *YY_mootLexfreqsLexer_OUT;
#ifdef YY_mootLexfreqsLexer_LEX_DEFINED
extern YY_mootLexfreqsLexer_LEX_RETURN YY_mootLexfreqsLexer_LEX ( YY_mootLexfreqsLexer_LEX_PARAM )
YY_mootLexfreqsLexer_LEX_PARAM_DEF
#else
#ifndef YY_DECL
extern YY_mootLexfreqsLexer_LEX_RETURN YY_mootLexfreqsLexer_LEX ( YY_mootLexfreqsLexer_LEX_PARAM )
YY_mootLexfreqsLexer_LEX_PARAM_DEF
#else
/* no declaration if oldstyle flex */
#endif
#endif
#else

#ifndef YY_mootLexfreqsLexer_CLASS
#define YY_mootLexfreqsLexer_CLASS mootLexfreqsLexer
#endif
#ifndef YY_mootLexfreqsLexer_ECHO
#define YY_mootLexfreqsLexer_ECHO yy_echo
#endif
#ifdef YY_mootLexfreqsLexer_ECHO_PURE
#define YY_mootLexfreqsLexer_ECHO_NOCODE
#endif
#ifndef YY_mootLexfreqsLexer_ECHO_CODE
#define YY_mootLexfreqsLexer_ECHO_CODE fwrite( (char *) YY_mootLexfreqsLexer_TEXT, YY_mootLexfreqsLexer_LENG, 1, YY_mootLexfreqsLexer_OUT );
#endif
#ifndef YY_mootLexfreqsLexer_INPUT
#define YY_mootLexfreqsLexer_INPUT yy_input
#endif
#ifdef YY_mootLexfreqsLexer_INPUT_PURE
#define YY_mootLexfreqsLexer_INPUT_NOCODE
#endif
#ifndef YY_mootLexfreqsLexer_INPUT_CODE
#define YY_mootLexfreqsLexer_INPUT_CODE return result= fread(  buffer, 1,max_size,YY_mootLexfreqsLexer_IN );
#endif
#ifdef YY_mootLexfreqsLexer_FATAL_ERROR_PURE
#define YY_mootLexfreqsLexer_FATAL_ERRO_NOCODE
#endif
#ifndef YY_mootLexfreqsLexer_FATAL_ERROR
#define YY_mootLexfreqsLexer_FATAL_ERROR yy_fatal_error
#endif
#ifndef YY_mootLexfreqsLexer_FATAL_ERROR_CODE
#define YY_mootLexfreqsLexer_FATAL_ERROR_CODE fputs( msg, stderr );putc( '\n', stderr );exit( 1 );
#endif
#ifndef YY_mootLexfreqsLexer_WRAP
#define YY_mootLexfreqsLexer_WRAP yy_wrap
#endif
#ifdef YY_mootLexfreqsLexer_WRAP_PURE
#define YY_mootLexfreqsLexer_WRAP_NOCODE
#endif
#ifndef YY_mootLexfreqsLexer_WRAP_CODE
#define YY_mootLexfreqsLexer_WRAP_CODE return 1;
#endif


#ifndef YY_mootLexfreqsLexer_INHERIT
#define YY_mootLexfreqsLexer_INHERIT
#endif
#ifndef YY_mootLexfreqsLexer_MEMBERS
#define YY_mootLexfreqsLexer_MEMBERS 
#endif
#ifndef YY_mootLexfreqsLexer_CONSTRUCTOR_PARAM
#define YY_mootLexfreqsLexer_CONSTRUCTOR_PARAM
#endif
#ifndef YY_mootLexfreqsLexer_CONSTRUCTOR_CODE
#define YY_mootLexfreqsLexer_CONSTRUCTOR_CODE
#endif
#ifndef YY_mootLexfreqsLexer_CONSTRUCTOR_INIT
#define YY_mootLexfreqsLexer_CONSTRUCTOR_INIT
#endif
typedef struct yy_buffer_state *YY_BUFFER_STATE;

class YY_mootLexfreqsLexer_CLASS YY_mootLexfreqsLexer_INHERIT
{
 private:/* data */
 YY_mootLexfreqsLexer_CHAR  *yy_c_buf_p;
 YY_mootLexfreqsLexer_CHAR  yy_hold_char;
 int yy_n_chars;
 int yy_init;
 int yy_start;
 int yy_did_buffer_switch_on_eof;
 private: /* functions */
 void yy_initialize();
 int input();
 int yyinput() {return input();};
 int yy_get_next_buffer();
 void yyunput( YY_mootLexfreqsLexer_CHAR  c, YY_mootLexfreqsLexer_CHAR  *buf_ptr );
 /* use long instead of yy_state_type because it is undef */
 long yy_get_previous_state_ ( void );
 long yy_try_NUL_trans_  ( long current_state_ );
 protected:/* non virtual */
 YY_BUFFER_STATE yy_current_buffer;
 void YY_mootLexfreqsLexer_RESTART ( FILE *input_file );
 void YY_mootLexfreqsLexer_SWITCH_TO_BUFFER( YY_BUFFER_STATE new_buffer );
 void YY_mootLexfreqsLexer_LOAD_BUFFER_STATE( void );
 YY_BUFFER_STATE YY_mootLexfreqsLexer_CREATE_BUFFER( FILE *file, int size );
 void YY_mootLexfreqsLexer_DELETE_BUFFER( YY_BUFFER_STATE b );
 void YY_mootLexfreqsLexer_INIT_BUFFER( YY_BUFFER_STATE b, FILE *file );
 protected: /* virtual */
 virtual void YY_mootLexfreqsLexer_ECHO()
#ifdef YY_mootLexfreqsLexer_ECHO_PURE
  =0
#endif
  ;
 virtual int  YY_mootLexfreqsLexer_INPUT(char  *buf,int &result,int max_size)
#ifdef YY_mootLexfreqsLexer_INPUT_PURE
  =0
#endif
  ;
 virtual void  YY_mootLexfreqsLexer_FATAL_ERROR(const char *msg)
#ifdef YY_mootLexfreqsLexer_FATAL_ERROR_PURE
  =0
#endif
  ;
 virtual int  YY_mootLexfreqsLexer_WRAP()
#ifdef YY_mootLexfreqsLexer_WRAP_PURE
  =0
#endif
  ;
 public:
 YY_mootLexfreqsLexer_CHAR  *YY_mootLexfreqsLexer_TEXT;
 int YY_mootLexfreqsLexer_LENG;
 FILE *YY_mootLexfreqsLexer_IN, *YY_mootLexfreqsLexer_OUT;
 YY_mootLexfreqsLexer_LEX_RETURN YY_mootLexfreqsLexer_LEX ( YY_mootLexfreqsLexer_LEX_PARAM);
 YY_mootLexfreqsLexer_CLASS(YY_mootLexfreqsLexer_CONSTRUCTOR_PARAM) ;
#if YY_mootLexfreqsLexer_DEBUG != 0
 int YY_mootLexfreqsLexer_DEBUG_FLAG;
#endif
 public: /* added members */
 YY_mootLexfreqsLexer_MEMBERS 
};
#endif



/* declaration of externs for public use of yylex scanner */

/* % here is the declaration from section2 %header{ */ 
#line 151 "mootLexfreqsLexer.ll"
#endif
#line 302 "./flexskel.h"

/* end of generated header */