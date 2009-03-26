#ifndef FLEX_HEADER_mootTokenLexer_h
#define FLEX_HEADER_mootTokenLexer_h
#define YY_mootTokenLexer_CHAR unsigned char
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
#line 47 "mootTokenLexer.ll"
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
 * \class mootTokenLexer
 * \brief Flex++ lexer for moot PoS tagger native text input.
 *
 * Assumes pre-tokenized input:
 * one token per line,  blank lines = EOS, raw text only (no markup!).
 * Format: 1 tok/line, comments introduced with '%%'.  Supports
 * multiple tags/tok.
 *
 * token-line format (TAB-separated):
 *
 * TOKEN_TEXT  ANALYSIS_1 ... ANALYSIS_N
 *
 * Analysis format(s):
 *
 * ...(COST?)... "[" TAG ["]"|" "] ...(COST?)...
 *
 * COST format(s):
 *
 * "<" COST_FLOAT ">"
 *
 */

#include <mootToken.h>
#include <mootGenericLexer.h>

using namespace moot;
#line 81 "mootTokenLexer.ll"
#define YY_mootTokenLexer_CLASS  mootTokenLexer
#line 83 "mootTokenLexer.ll"
#define YY_mootTokenLexer_INHERIT  \
  : public moot::GenericLexer
#line 86 "mootTokenLexer.ll"
#define YY_mootTokenLexer_INPUT_CODE  \
  return moot::GenericLexer::yyinput(buffer,result,max_size);
#line 89 "mootTokenLexer.ll"
#define YY_mootTokenLexer_MEMBERS  \
  public: \
  /* -- public typedefs */\
  typedef moot::mootTokenType TokenType; \
  /* extra token types */ \
  static const int LexTypeText = moot::NTokTypes+1;    /* literal token text */ \
  static const int LexTypeTag = moot::NTokTypes+2;     /* analysis tag */ \
  static const int LexTypeDetails = moot::NTokTypes+3; /* analysis details */ \
  static const int LexTypeEOA = moot::NTokTypes+4;     /* end-of-analysis (separator) */ \
  static const int LexTypeEOT = moot::NTokTypes+5;     /* end-of-token */ \
  static const int LexTypeIgnore = moot::NTokTypes+6;  /* ignored data (unused) */ \
  \
  public: \
   /** last token type */ \
   int lasttyp; \
   \
   /* -- pre-allocated construction buffers */ \
   /* current token (default) */ \
   moot::mootToken mtoken_default; \
   /* current token (real) */ \
   moot::mootToken *mtoken; \
   \
   /** current analysis (real) */ \
   moot::mootToken::Analysis *manalysis;\
   \
   /** whether to ignore comments (default=false) */ \
   bool ignore_comments; \
   /** whether first analysis parsed should be considered 'best' (default=true) */ \
   bool first_analysis_is_best; \
   /** whether we're parsing a 'best' analysis */\
   bool current_analysis_is_best; \
   /** whether to (otherwise) ignore first analysis (default=false) */ \
   bool ignore_first_analysis; \
   /** whether to ignore current analysis */\
   bool ignore_current_analysis; \
   \
  public: \
    /* -- local methods */ \
    /** virtual destructor to shut up gcc */\
    virtual ~mootTokenLexer(void) {};\
    /** reset to initial state */ \
    virtual void reset(void); \
    /** actions to perform on end-of-analysis */ \
    inline void on_EOA(void) \
    { \
      /*-- EOA: add & clear current analysis, if any */ \
      /*-- add & clear current analysis, if any */ \
      if (lasttyp != LexTypeEOA) { \
        /*-- set default tag */\
        if (manalysis->tag.empty()) { \
          manalysis->tag.swap(manalysis->details); \
        }  \
        /* set best tag if applicable */\
        if (current_analysis_is_best) { \
          mtoken->besttag(manalysis->tag); \
          current_analysis_is_best = false; \
        } \
        if (ignore_current_analysis) { \
          ignore_current_analysis=false; \
          mtoken->tok_analyses.pop_back(); \
        } \
      } \
    }; \
  /*-- moot::GenericLexer helpers */ \
  virtual void  *mgl_yy_current_buffer_p(void) \
                 {return reinterpret_cast<void*>(&yy_current_buffer);}; \
  virtual void  *mgl_yy_create_buffer(int size, FILE *unused=stdin) \
                 {return reinterpret_cast<void*>(yy_create_buffer(unused,size));};\
  virtual void   mgl_yy_init_buffer(void *buf, FILE *unused=stdin) \
                 {yy_init_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf),unused);};\
  virtual void   mgl_yy_delete_buffer(void *buf) \
                 {yy_delete_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf));};\
  virtual void   mgl_yy_switch_to_buffer(void *buf) \
                 {yy_switch_to_buffer(reinterpret_cast<YY_BUFFER_STATE>(buf));};\
  virtual void   mgl_begin(int stateno);
#line 166 "mootTokenLexer.ll"
#define YY_mootTokenLexer_CONSTRUCTOR_INIT  :\
  GenericLexer("mootTokenLexer"), \
  yyin(NULL), \
  lasttyp(moot::TokTypeEOS), \
  manalysis(NULL), \
  ignore_comments(false), \
  first_analysis_is_best(true), \
  current_analysis_is_best(false), \
  ignore_first_analysis(false), \
  ignore_current_analysis(false)
#line 177 "mootTokenLexer.ll"
#define YY_mootTokenLexer_CONSTRUCTOR_CODE  \
  mtoken = &mtoken_default;
#line 208 "mootTokenLexer.ll"
#line 52 "./flexskel.h"



#ifndef YY_mootTokenLexer_TEXT
#define YY_mootTokenLexer_TEXT yytext
#endif
#ifndef YY_mootTokenLexer_LENG
#define YY_mootTokenLexer_LENG yyleng
#endif
#ifndef YY_mootTokenLexer_IN
#define YY_mootTokenLexer_IN yyin
#endif
#ifndef YY_mootTokenLexer_OUT
#define YY_mootTokenLexer_OUT yyout
#endif
#ifndef YY_mootTokenLexer_LEX_RETURN
#define YY_mootTokenLexer_LEX_RETURN int
#else
#ifndef YY_mootTokenLexer_LEX_DEFINED
#define YY_mootTokenLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootTokenLexer_LEX
#define YY_mootTokenLexer_LEX yylex
#else
#ifndef YY_mootTokenLexer_LEX_DEFINED
#define YY_mootTokenLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootTokenLexer_LEX_PARAM
#ifndef YY_USE_PROTOS
#define YY_mootTokenLexer_LEX_PARAM 
#else
#define YY_mootTokenLexer_LEX_PARAM void
#endif
#else
#ifndef YY_mootTokenLexer_LEX_DEFINED
#define YY_mootTokenLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootTokenLexer_LEX_PARAM_DEF
#define YY_mootTokenLexer_LEX_PARAM_DEF
#else
#ifndef YY_mootTokenLexer_LEX_DEFINED
#define YY_mootTokenLexer_LEX_DEFINED 
#endif
#endif

#ifndef YY_mootTokenLexer_RESTART
#define YY_mootTokenLexer_RESTART yyrestart
#endif
#ifndef YY_mootTokenLexer_SWITCH_TO_BUFFER
#define YY_mootTokenLexer_SWITCH_TO_BUFFER yy_switch_to_buffer
#endif
#ifndef YY_mootTokenLexer_LOAD_BUFFER_STATE
#define YY_mootTokenLexer_LOAD_BUFFER_STATE yy_load_buffer_state
#endif

#ifndef YY_mootTokenLexer_CREATE_BUFFER
#define YY_mootTokenLexer_CREATE_BUFFER yy_create_buffer
#ifndef YY_USE_CLASS
#ifndef yy_new_buffer
#define yy_new_buffer yy_create_buffer
#endif
#endif
#endif
#ifndef YY_mootTokenLexer_DELETE_BUFFER
#define YY_mootTokenLexer_DELETE_BUFFER yy_delete_buffer
#endif
#ifndef YY_mootTokenLexer_INIT_BUFFER
#define YY_mootTokenLexer_INIT_BUFFER yy_init_buffer
#endif
#ifdef YY_mootTokenLexer_FLEX_DEBUG
#ifndef YY_mootTokenLexer_DEBUG
#define YY_mootTokenLexer_DEBUG 1
#endif
#else
#ifndef YY_mootTokenLexer_DEBUG
#define YY_mootTokenLexer_DEBUG 0
#endif
#endif

#if YY_mootTokenLexer_DEBUG != 0
#ifndef YY_mootTokenLexer_DEBUG_FLAG
#define YY_mootTokenLexer_DEBUG_FLAG yy_flex_debug
#endif
#ifndef YY_mootTokenLexer_DEBUG_INIT
#define YY_mootTokenLexer_DEBUG_INIT 1
#endif
#endif




#ifndef YY_USE_CLASS
typedef struct yy_buffer_state *YY_BUFFER_STATE;

extern void YY_mootTokenLexer_RESTART YY_PROTO(( FILE *input_file ));
extern void YY_mootTokenLexer_SWITCH_TO_BUFFER YY_PROTO(( YY_BUFFER_STATE new_buffer ));
extern void YY_mootTokenLexer_LOAD_BUFFER_STATE YY_PROTO(( void ));
extern YY_BUFFER_STATE YY_mootTokenLexer_CREATE_BUFFER YY_PROTO(( FILE *file, int size ));
extern void YY_mootTokenLexer_DELETE_BUFFER YY_PROTO(( YY_BUFFER_STATE b ));
extern void YY_mootTokenLexer_INIT_BUFFER YY_PROTO(( YY_BUFFER_STATE b, FILE *file ));

#if YY_mootTokenLexer_DEBUG != 0
extern int YY_mootTokenLexer_DEBUG_FLAG ;
#endif
extern YY_mootTokenLexer_CHAR  *YY_mootTokenLexer_TEXT;
extern int YY_mootTokenLexer_LENG;
extern FILE *YY_mootTokenLexer_IN, *YY_mootTokenLexer_OUT;
#ifdef YY_mootTokenLexer_LEX_DEFINED
extern YY_mootTokenLexer_LEX_RETURN YY_mootTokenLexer_LEX ( YY_mootTokenLexer_LEX_PARAM )
YY_mootTokenLexer_LEX_PARAM_DEF
#else
#ifndef YY_DECL
extern YY_mootTokenLexer_LEX_RETURN YY_mootTokenLexer_LEX ( YY_mootTokenLexer_LEX_PARAM )
YY_mootTokenLexer_LEX_PARAM_DEF
#else
/* no declaration if oldstyle flex */
#endif
#endif
#else

#ifndef YY_mootTokenLexer_CLASS
#define YY_mootTokenLexer_CLASS mootTokenLexer
#endif
#ifndef YY_mootTokenLexer_ECHO
#define YY_mootTokenLexer_ECHO yy_echo
#endif
#ifdef YY_mootTokenLexer_ECHO_PURE
#define YY_mootTokenLexer_ECHO_NOCODE
#endif
#ifndef YY_mootTokenLexer_ECHO_CODE
#define YY_mootTokenLexer_ECHO_CODE fwrite( (char *) YY_mootTokenLexer_TEXT, YY_mootTokenLexer_LENG, 1, YY_mootTokenLexer_OUT );
#endif
#ifndef YY_mootTokenLexer_INPUT
#define YY_mootTokenLexer_INPUT yy_input
#endif
#ifdef YY_mootTokenLexer_INPUT_PURE
#define YY_mootTokenLexer_INPUT_NOCODE
#endif
#ifndef YY_mootTokenLexer_INPUT_CODE
#define YY_mootTokenLexer_INPUT_CODE return result= fread(  buffer, 1,max_size,YY_mootTokenLexer_IN );
#endif
#ifdef YY_mootTokenLexer_FATAL_ERROR_PURE
#define YY_mootTokenLexer_FATAL_ERRO_NOCODE
#endif
#ifndef YY_mootTokenLexer_FATAL_ERROR
#define YY_mootTokenLexer_FATAL_ERROR yy_fatal_error
#endif
#ifndef YY_mootTokenLexer_FATAL_ERROR_CODE
#define YY_mootTokenLexer_FATAL_ERROR_CODE fputs( msg, stderr );putc( '\n', stderr );exit( 1 );
#endif
#ifndef YY_mootTokenLexer_WRAP
#define YY_mootTokenLexer_WRAP yy_wrap
#endif
#ifdef YY_mootTokenLexer_WRAP_PURE
#define YY_mootTokenLexer_WRAP_NOCODE
#endif
#ifndef YY_mootTokenLexer_WRAP_CODE
#define YY_mootTokenLexer_WRAP_CODE return 1;
#endif


#ifndef YY_mootTokenLexer_INHERIT
#define YY_mootTokenLexer_INHERIT
#endif
#ifndef YY_mootTokenLexer_MEMBERS
#define YY_mootTokenLexer_MEMBERS 
#endif
#ifndef YY_mootTokenLexer_CONSTRUCTOR_PARAM
#define YY_mootTokenLexer_CONSTRUCTOR_PARAM
#endif
#ifndef YY_mootTokenLexer_CONSTRUCTOR_CODE
#define YY_mootTokenLexer_CONSTRUCTOR_CODE
#endif
#ifndef YY_mootTokenLexer_CONSTRUCTOR_INIT
#define YY_mootTokenLexer_CONSTRUCTOR_INIT
#endif
typedef struct yy_buffer_state *YY_BUFFER_STATE;

class YY_mootTokenLexer_CLASS YY_mootTokenLexer_INHERIT
{
 private:/* data */
 YY_mootTokenLexer_CHAR  *yy_c_buf_p;
 YY_mootTokenLexer_CHAR  yy_hold_char;
 int yy_n_chars;
 int yy_init;
 int yy_start;
 int yy_did_buffer_switch_on_eof;
 private: /* functions */
 void yy_initialize();
 int input();
 int yyinput() {return input();};
 int yy_get_next_buffer();
 void yyunput( YY_mootTokenLexer_CHAR  c, YY_mootTokenLexer_CHAR  *buf_ptr );
 /* use long instead of yy_state_type because it is undef */
 long yy_get_previous_state_ ( void );
 long yy_try_NUL_trans_  ( long current_state_ );
 protected:/* non virtual */
 YY_BUFFER_STATE yy_current_buffer;
 void YY_mootTokenLexer_RESTART ( FILE *input_file );
 void YY_mootTokenLexer_SWITCH_TO_BUFFER( YY_BUFFER_STATE new_buffer );
 void YY_mootTokenLexer_LOAD_BUFFER_STATE( void );
 YY_BUFFER_STATE YY_mootTokenLexer_CREATE_BUFFER( FILE *file, int size );
 void YY_mootTokenLexer_DELETE_BUFFER( YY_BUFFER_STATE b );
 void YY_mootTokenLexer_INIT_BUFFER( YY_BUFFER_STATE b, FILE *file );
 protected: /* virtual */
 virtual void YY_mootTokenLexer_ECHO()
#ifdef YY_mootTokenLexer_ECHO_PURE
  =0
#endif
  ;
 virtual int  YY_mootTokenLexer_INPUT(char  *buf,int &result,int max_size)
#ifdef YY_mootTokenLexer_INPUT_PURE
  =0
#endif
  ;
 virtual void  YY_mootTokenLexer_FATAL_ERROR(const char *msg)
#ifdef YY_mootTokenLexer_FATAL_ERROR_PURE
  =0
#endif
  ;
 virtual int  YY_mootTokenLexer_WRAP()
#ifdef YY_mootTokenLexer_WRAP_PURE
  =0
#endif
  ;
 public:
 YY_mootTokenLexer_CHAR  *YY_mootTokenLexer_TEXT;
 int YY_mootTokenLexer_LENG;
 FILE *YY_mootTokenLexer_IN, *YY_mootTokenLexer_OUT;
 YY_mootTokenLexer_LEX_RETURN YY_mootTokenLexer_LEX ( YY_mootTokenLexer_LEX_PARAM);
 YY_mootTokenLexer_CLASS(YY_mootTokenLexer_CONSTRUCTOR_PARAM) ;
#if YY_mootTokenLexer_DEBUG != 0
 int YY_mootTokenLexer_DEBUG_FLAG;
#endif
 public: /* added members */
 YY_mootTokenLexer_MEMBERS 
};
#endif



/* declaration of externs for public use of yylex scanner */

/* % here is the declaration from section2 %header{ */ 
#line 427 "mootTokenLexer.ll"
#endif
#line 302 "./flexskel.h"

/* end of generated header */
