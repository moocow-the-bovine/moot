/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstParamLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + Lexer for TnT-style parameter files
 *   + process with Alain Coetmeur's 'flex++' to produce a C++ lexer
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdstParamLexer

%header{

#include <string.h>
#include <string>

#include <FSMSymSpec.h>
#include "dwdstParamParser.h"

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/*!
 * \class dwdstParamLexer
 * \brief Flex++ lexer for dwdst-pargen (TnT-style) parameter files. 
 * Supports comments introduced with '%%'.
 */
%}

%define LEX_PARAM \
  YY_dwdstParamParser_STYPE *yylval, YY_dwdstParamParser_LTYPE *yylloc


%define CLASS dwdstParamLexer
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
    FSMSymbolString tokbuf;\
    /** \brief whether to clear the token-buffer on 'tokbuf_append()' */\
    bool tokbuf_clear;\
  private: \
    /* private local data */ \
    bool use_string; \
    char *stringbuf; \
  public: \
    /** \brief virtual destructor to shut up gcc */\
    virtual ~dwdstParamLexer(void) {};\
    /** \brief use stream input */\
    void select_streams(FILE *in=stdin, FILE *out=stdout); \
    /** \brief use string input */\
    void select_string(const char *in, FILE *out=stdout); \
    /** \brief for token-buffering: append yyleng characters of yytext to 'tokbuf' */\
    inline void tokbuf_append(char *text, int leng);

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
  return result= fread(buffer, 1, max_size, YY_dwdstParamLexer_IN);



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
  return dwdstParamParser::REGEX;
}

([\-\+]?)([0-9]*)(\.?)([0-9]+) {
  // -- count : return it
  theLine++; theColumn = 0;
  yylval->count = atof((const char *)yytext);
  return dwdstParamParser::COUNT;
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
  fprintf(stderr,"dwdstParamLexer warning: unknown character '%c': ignored.\n", yytext[YY_MORE_ADJ]);
}

%%

// -- co-exist with BumbleBee 'clex.h'
#ifdef REJECT
# undef REJECT
#endif



/*----------------------------------------------------------------------
 * Local Methods for dwdstParamLexer
 *----------------------------------------------------------------------*/

/*
 * void dwdstParamLexer::select_streams(FILE *in, FILE *out)
 */
void dwdstParamLexer::select_streams(FILE *in, FILE *out) {
  yyin = in;
  yyout = out;
  use_string = false;

  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}

/*
 * void dwdstParamLexer::select_string(const char *in, FILE *out=stdout)
 */
void dwdstParamLexer::select_string(const char *in, FILE *out=stdout) {
  select_streams(stdin,out);  // flex __really__ wants a real input stream

  // -- string-buffer stuff
  use_string = true;
  stringbuf = (char *)in;
}


/*
 * tokbuf_append(text,leng)
 */
inline void dwdstParamLexer::tokbuf_append(char *text, int leng) {
  if (tokbuf_clear) {
    tokbuf = (char *)text;
    tokbuf_clear = false;
  } else {
    tokbuf.append((char *)text,leng);
  }
}
