/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdst_param_lexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + Lexer for TnT-style parameter files
 *   + process with Alain Coetmeur's 'flex++' to produce a C++ lexer
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdst_param_lexer

%header{

#include <string.h>
#include <string>
#include <FSMSymSpec.h>
#include "dwdst_param_parser.h"

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/*!
 * \class dwdst_param_lexer
 * \brief Flex++ lexer for dwdst-pargen (TnT-style) parameter files.  Supports comments introduced with '%%'.
 */
%}

%define LEX_PARAM \
  YY_dwdst_param_parser_STYPE *yylval, YY_dwdst_param_parser_LTYPE *yylloc


%define CLASS dwdst_param_lexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
  public: \
   /* -- positional parameters */\
    /** \brief current line*/\
    int theLine;\
    /** \brief current column*/\
    int theColumn;\
    /** \brief token-buffering */\
    /*FSMSymbolString tokbuf;*/\
    /** \brief whether to clear the token-buffer on 'tokbuf_append()' */\
    /*bool tokbuf_clear;*/\
  private: \
    /* private local data */ \
    bool use_string; \
    char *stringbuf; \
  public: \
    /** \brief virtual destructor to shut up gcc */\
    virtual ~dwdst_param_lexer(void) {};\
    /** \brief use stream input */\
    void select_streams(FILE *in=stdin, FILE *out=stdout); \
    /** \brief use string input */\
    void select_string(const char *in, FILE *out=stdout); \
    /** \brief for token-buffering: append yyleng characters of yytext to 'tokbuf' */\
    /*inline void tokbuf_append(char *text, int leng);*/

%define CONSTRUCTOR_INIT :\
  theLine(1), \
  theColumn(1), \
  use_string(false), \
  stringbuf(NULL)

%define INPUT_CODE \
  /* yy_input(char *buf, int &result, int max_size) */\
  if (use_string) {\
    size_t len = strlen(stringbuf) > (size_t)max_size ? max_size : strlen(stringbuf);\
    strncpy(buffer,stringbuf,len);\
    stringbuf += len;\
    return result = len;\
  }\
  /* black magic */\
  return result= fread(buffer, 1, max_size, YY_dwdst_param_lexer_IN);



/*----------------------------------------------------------------------
 * Definitions & other lexer properties
 *----------------------------------------------------------------------*/
newline    [\n\r]
nonnewline [^\n\r]
whitespace [ \t]
textchar   [^ \n\r\t]
textorsp   [^\n\r\t]

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%

^(" "+) {
  // -- ignore leading whitespace
  theColumn += yyleng;
}

"%%"({nonnewline}*) {
   // -- ignore comments
   theColumn += yyleng;
}

({whitespace}*)"\t"({whitespace}*) {
  // -- tab: return the current token-buffer
  theColumn += yyleng;
  return '\t';
}

{textchar}({textorsp}*{textchar})? {
  // -- any other text: append to the token-buffer
  theColumn += yyleng;
  yylval->symstr = new FSMSymbolString((const char *)yytext);
  return dwdst_param_parser::REGEX;
}

([\-\+]?)([0-9]*)(\.?)([0-9]+) {
  // -- count : return it
  theLine++; theColumn = 0;
  yylval->cost = atof((const char *)yytext);
  return dwdst_param_parser::COUNT;
}

{newline} {
  // -- newlines : ignore
  theLine++; theColumn = 0;
  return '\n';
}

<<EOF>> {
  return 0;
}

. {
  // -- huh? -- just ignore it!
  theColumn += yyleng;
  fprintf(stderr,"dwds_param_lexer warning: unknown character '%c': ignored.\n", yytext[YY_MORE_ADJ]);
}

%%

// -- co-exist with BumbleBee 'clex.h'
#ifdef REJECT
# undef REJECT
#endif



/*----------------------------------------------------------------------
 * Local Methods for dwdst_param_lexer
 *----------------------------------------------------------------------*/

/*
 * void dwdst_param_lexer::select_streams(FILE *in, FILE *out)
 */
void dwdst_param_lexer::select_streams(FILE *in, FILE *out) {
  yyin = in;
  yyout = out;
  use_string = false;

  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}

/*
 * void dwdst_param_lexer::select_string(const char *in, FILE *out=stdout)
 */
void dwdst_param_lexer::select_string(const char *in, FILE *out=stdout) {
  select_streams(stdin,out);  // flex __really__ wants a real input stream

  // -- string-buffer stuff
  use_string = true;
  stringbuf = (char *)in;
}


/*
 * tokbuf_append(text,leng)
 */
inline void dwdst_param_lexer::tokbuf_append(char *text, int leng) {
  if (tokbuf_clear) {
    tokbuf = (char *)text;
    tokbuf_clear = false;
  } else {
    tokbuf.append((char *)text,leng);
  }
}
