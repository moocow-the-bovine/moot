#ifndef FLEX_HEADER_mootNgramsLexer_h
#define FLEX_HEADER_mootNgramsLexer_h
#define YY_mootNgramsLexer_CHAR unsigned char
#line 1 "/usr/local/share/flex++bison++/flexskel.h"
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
#line 33 "mootNgramsLexer.ll"

#include "mootTypes.h"
#include "mootNgramsParser.h"

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
 * \class mootNgramsLexer
 * \brief Flex++ lexer for (TnT-style) moot n-gram parameter files. 
 *
 * Supports comments introduced with '%%'.
 */
#line 48 "mootNgramsLexer.ll"
#define YY_mootNgramsLexer_LEX_PARAM  \
  YY_mootNgramsParser_STYPE *yylval, YY_mootNgramsParser_LTYPE *yylloc
#line 52 "mootNgramsLexer.ll"
#define YY_mootNgramsLexer_CLASS  mootNgramsLexer
#line 53 "mootNgramsLexer.ll"
#define YY_mootNgramsLexer_MEMBERS  \
  public: \
    /* -- public typedefs */\
  public: \
   /* -- positional parameters */\
    /** current line*/\
    int theLine;\
    /** current column*/\
    int theColumn;\
    /** token-buffering */\
    moot::mootTagString tokbuf;\
    /** whether to clear the token-buffer on 'tokbuf_append()' */\
    bool tokbuf_clear;\
  private: \
    /* private local data */ \
    bool use_string; \
    char *stringbuf; \
  public: \
    /** virtual destructor to shut up gcc */\
    virtual ~mootNgramsLexer(void) {}; \
    /** use stream input */\
    void select_streams(FILE *in=stdin, FILE *out=stdout); \
    /** use string input */\
    void select_string(const char *in, FILE *out=stdout); \
    /** for token-buffering: append yyleng characters of yytext to 'tokbuf' */\
    inline void tokbuf_append(char *text, int leng);
#line 80 "mootNgramsLexer.ll"
#define YY_mootNgramsLexer_CONSTRUCTOR_INIT  :\
  theLine(1), \
  theColumn(0), \
  use_string(false), \
  stringbuf(NULL)
#line 86 "mootNgramsLexer.ll"
#define YY_mootNgramsLexer_INPUT_CODE  \
  /* yy_input(char *buf, int &result, int max_size) */\
  if (use_string) {\
    size_t len = strlen(stringbuf) > (size_t)max_size ? max_size : strlen(stringbuf);\
    strncpy(buffer,stringbuf,len);\
    stringbuf += len;\
    return result = len;\
  }\
  /* black magic */\
  return result= fread(buffer, 1, max_size, YY_mootNgramsLexer_IN);
#line 111 "mootNgramsLexer.ll"
#line 52 "/usr/local/share/flex++bison++/flexskel.h"



#ifndef YY_mootNgramsLexer_TEXT
#define YY_mootNgramsLexer_TEXT yytext
#endif
#ifndef YY_mootNgramsLexer_LENG
#define YY_mootNgramsLexer_LENG yyleng
#endif
#ifndef YY_mootNgramsLexer_IN
#define YY_mootNgramsLexer_IN yyin
#endif
#ifndef YY_mootNgramsLexer_OUT
#define YY_mootNgramsLexer_OUT yyout
#endif
#ifndef YY_mootNgramsLexer_LEX_RETURN
#define YY_mootNgramsLexer_LEX_RETURN int
#else
#ifndef YY_mootNgramsLexer_LEX_DEFINED
#define YY_mootNgramsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootNgramsLexer_LEX
#define YY_mootNgramsLexer_LEX yylex
#else
#ifndef YY_mootNgramsLexer_LEX_DEFINED
#define YY_mootNgramsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootNgramsLexer_LEX_PARAM
#ifndef YY_USE_PROTOS
#define YY_mootNgramsLexer_LEX_PARAM 
#else
#define YY_mootNgramsLexer_LEX_PARAM void
#endif
#else
#ifndef YY_mootNgramsLexer_LEX_DEFINED
#define YY_mootNgramsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootNgramsLexer_LEX_PARAM_DEF
#define YY_mootNgramsLexer_LEX_PARAM_DEF
#else
#ifndef YY_mootNgramsLexer_LEX_DEFINED
#define YY_mootNgramsLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootNgramsLexer_RESTART
#define YY_mootNgramsLexer_RESTART yyrestart
#endif
#ifndef YY_mootNgramsLexer_SWITCH_TO_BUFFER
#define YY_mootNgramsLexer_SWITCH_TO_BUFFER yy_switch_to_buffer
#endif
#ifndef YY_mootNgramsLexer_LOAD_BUFFER_STATE
#define YY_mootNgramsLexer_LOAD_BUFFER_STATE yy_load_buffer_state
#endif

#ifndef YY_mootNgramsLexer_CREATE_BUFFER
#define YY_mootNgramsLexer_CREATE_BUFFER yy_create_buffer
#ifndef YY_USE_CLASS
#ifndef yy_new_buffer
#define yy_new_buffer yy_create_buffer
#endif
#endif
#endif
#ifndef YY_mootNgramsLexer_DELETE_BUFFER
#define YY_mootNgramsLexer_DELETE_BUFFER yy_delete_buffer
#endif
#ifndef YY_mootNgramsLexer_INIT_BUFFER
#define YY_mootNgramsLexer_INIT_BUFFER yy_init_buffer
#endif
#ifdef YY_mootNgramsLexer_FLEX_DEBUG
#ifndef YY_mootNgramsLexer_DEBUG
#define YY_mootNgramsLexer_DEBUG 1
#endif
#else
#ifndef YY_mootNgramsLexer_DEBUG
#define YY_mootNgramsLexer_DEBUG 0
#endif
#endif

#if YY_mootNgramsLexer_DEBUG != 0
#ifndef YY_mootNgramsLexer_DEBUG_FLAG
#define YY_mootNgramsLexer_DEBUG_FLAG yy_flex_debug
#endif
#ifndef YY_mootNgramsLexer_DEBUG_INIT
#define YY_mootNgramsLexer_DEBUG_INIT 1
#endif
#endif




#ifndef YY_USE_CLASS
typedef struct yy_buffer_state *YY_BUFFER_STATE;

extern void YY_mootNgramsLexer_RESTART YY_PROTO(( FILE *input_file ));
extern void YY_mootNgramsLexer_SWITCH_TO_BUFFER YY_PROTO(( YY_BUFFER_STATE new_buffer ));
extern void YY_mootNgramsLexer_LOAD_BUFFER_STATE YY_PROTO(( void ));
extern YY_BUFFER_STATE YY_mootNgramsLexer_CREATE_BUFFER YY_PROTO(( FILE *file, int size ));
extern void YY_mootNgramsLexer_DELETE_BUFFER YY_PROTO(( YY_BUFFER_STATE b ));
extern void YY_mootNgramsLexer_INIT_BUFFER YY_PROTO(( YY_BUFFER_STATE b, FILE *file ));

#if YY_mootNgramsLexer_DEBUG != 0
extern int YY_mootNgramsLexer_DEBUG_FLAG ;
#endif
extern YY_mootNgramsLexer_CHAR  *YY_mootNgramsLexer_TEXT;
extern int YY_mootNgramsLexer_LENG;
extern FILE *YY_mootNgramsLexer_IN, *YY_mootNgramsLexer_OUT;
#ifdef YY_mootNgramsLexer_LEX_DEFINED
extern YY_mootNgramsLexer_LEX_RETURN YY_mootNgramsLexer_LEX ( YY_mootNgramsLexer_LEX_PARAM )
YY_mootNgramsLexer_LEX_PARAM_DEF
#else
#ifndef YY_DECL
extern YY_mootNgramsLexer_LEX_RETURN YY_mootNgramsLexer_LEX ( YY_mootNgramsLexer_LEX_PARAM )
YY_mootNgramsLexer_LEX_PARAM_DEF
#else
/* no declaration if oldstyle flex */
#endif
#endif
#else

#ifndef YY_mootNgramsLexer_CLASS
#define YY_mootNgramsLexer_CLASS mootNgramsLexer
#endif
#ifndef YY_mootNgramsLexer_ECHO
#define YY_mootNgramsLexer_ECHO yy_echo
#endif
#ifdef YY_mootNgramsLexer_ECHO_PURE
#define YY_mootNgramsLexer_ECHO_NOCODE
#endif
#ifndef YY_mootNgramsLexer_ECHO_CODE
#define YY_mootNgramsLexer_ECHO_CODE fwrite( (char *) YY_mootNgramsLexer_TEXT, YY_mootNgramsLexer_LENG, 1, YY_mootNgramsLexer_OUT );
#endif
#ifndef YY_mootNgramsLexer_INPUT
#define YY_mootNgramsLexer_INPUT yy_input
#endif
#ifdef YY_mootNgramsLexer_INPUT_PURE
#define YY_mootNgramsLexer_INPUT_NOCODE
#endif
#ifndef YY_mootNgramsLexer_INPUT_CODE
#define YY_mootNgramsLexer_INPUT_CODE return result= fread(  buffer, 1,max_size,YY_mootNgramsLexer_IN );
#endif
#ifdef YY_mootNgramsLexer_FATAL_ERROR_PURE
#define YY_mootNgramsLexer_FATAL_ERRO_NOCODE
#endif
#ifndef YY_mootNgramsLexer_FATAL_ERROR
#define YY_mootNgramsLexer_FATAL_ERROR yy_fatal_error
#endif
#ifndef YY_mootNgramsLexer_FATAL_ERROR_CODE
#define YY_mootNgramsLexer_FATAL_ERROR_CODE fputs( msg, stderr );putc( '\n', stderr );exit( 1 );
#endif
#ifndef YY_mootNgramsLexer_WRAP
#define YY_mootNgramsLexer_WRAP yy_wrap
#endif
#ifdef YY_mootNgramsLexer_WRAP_PURE
#define YY_mootNgramsLexer_WRAP_NOCODE
#endif
#ifndef YY_mootNgramsLexer_WRAP_CODE
#define YY_mootNgramsLexer_WRAP_CODE return 1;
#endif


#ifndef YY_mootNgramsLexer_INHERIT
#define YY_mootNgramsLexer_INHERIT
#endif
#ifndef YY_mootNgramsLexer_MEMBERS
#define YY_mootNgramsLexer_MEMBERS 
#endif
#ifndef YY_mootNgramsLexer_CONSTRUCTOR_PARAM
#define YY_mootNgramsLexer_CONSTRUCTOR_PARAM
#endif
#ifndef YY_mootNgramsLexer_CONSTRUCTOR_CODE
#define YY_mootNgramsLexer_CONSTRUCTOR_CODE
#endif
#ifndef YY_mootNgramsLexer_CONSTRUCTOR_INIT
#define YY_mootNgramsLexer_CONSTRUCTOR_INIT
#endif
typedef struct yy_buffer_state *YY_BUFFER_STATE;

class YY_mootNgramsLexer_CLASS YY_mootNgramsLexer_INHERIT
{
 private:/* data */
 YY_mootNgramsLexer_CHAR  *yy_c_buf_p;
 YY_mootNgramsLexer_CHAR  yy_hold_char;
 int yy_n_chars;
 int yy_init;
 int yy_start;
 int yy_did_buffer_switch_on_eof;
 private: /* functions */
 void yy_initialize();
 int input();
 int yyinput() {return input();};
 int yy_get_next_buffer();
 void yyunput( YY_mootNgramsLexer_CHAR  c, YY_mootNgramsLexer_CHAR  *buf_ptr );
 /* use long instead of yy_state_type because it is undef */
 long yy_get_previous_state_ ( void );
 long yy_try_NUL_trans_  ( long current_state_ );
 protected:/* non virtual */
 YY_BUFFER_STATE yy_current_buffer;
 void YY_mootNgramsLexer_RESTART ( FILE *input_file );
 void YY_mootNgramsLexer_SWITCH_TO_BUFFER( YY_BUFFER_STATE new_buffer );
 void YY_mootNgramsLexer_LOAD_BUFFER_STATE( void );
 YY_BUFFER_STATE YY_mootNgramsLexer_CREATE_BUFFER( FILE *file, int size );
 void YY_mootNgramsLexer_DELETE_BUFFER( YY_BUFFER_STATE b );
 void YY_mootNgramsLexer_INIT_BUFFER( YY_BUFFER_STATE b, FILE *file );
 protected: /* virtual */
 virtual void YY_mootNgramsLexer_ECHO()
#ifdef YY_mootNgramsLexer_ECHO_PURE
  =0
#endif
  ;
 virtual int  YY_mootNgramsLexer_INPUT(char  *buf,int &result,int max_size)
#ifdef YY_mootNgramsLexer_INPUT_PURE
  =0
#endif
  ;
 virtual void  YY_mootNgramsLexer_FATAL_ERROR(char *msg)
#ifdef YY_mootNgramsLexer_FATAL_ERROR_PURE
  =0
#endif
  ;
 virtual int  YY_mootNgramsLexer_WRAP()
#ifdef YY_mootNgramsLexer_WRAP_PURE
  =0
#endif
  ;
 public:
 YY_mootNgramsLexer_CHAR  *YY_mootNgramsLexer_TEXT;
 int YY_mootNgramsLexer_LENG;
 FILE *YY_mootNgramsLexer_IN, *YY_mootNgramsLexer_OUT;
 YY_mootNgramsLexer_LEX_RETURN YY_mootNgramsLexer_LEX ( YY_mootNgramsLexer_LEX_PARAM);
 YY_mootNgramsLexer_CLASS(YY_mootNgramsLexer_CONSTRUCTOR_PARAM) ;
#if YY_mootNgramsLexer_DEBUG != 0
 int YY_mootNgramsLexer_DEBUG_FLAG;
#endif
 public: /* added members */
 YY_mootNgramsLexer_MEMBERS 
};
#endif



/* declaration of externs for public use of yylex scanner */

/* % here is the declaration from section2 %header{ */ 
#line 161 "mootNgramsLexer.ll"
#endif
#line 302 "/usr/local/share/flex++bison++/flexskel.h"

/* end of generated header */
