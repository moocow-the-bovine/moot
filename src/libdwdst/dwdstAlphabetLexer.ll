/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstAlphabetLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + Lexer for intermediate alphabet files as produced by 'kempe-alphagen.perl'
 *   + process with Alain Coetmeur's 'flex++' to produce a C++ lexer
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdstAlphabetLexer

%header{

#include <string.h>
#include <string>

#include <FSMSymSpec.h>
#include "dwdstAlphabetParser.h"

/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
 * \class dwdstAlphabetLexer
 * \brief Lexer for intermediate alphabet files as produced by 'kempe-alphagen.perl'.
 * Supports comments introduced with '#'.
 */
%}

%define LEX_PARAM \
  YY_dwdstAlphabetParser_STYPE *yylval, YY_dwdstAlphabetParser_LTYPE *yylloc


%define CLASS dwdstAlphabetLexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
  public: \
   /* -- positional parameters */\
    /** current line*/\
    int theLine;\
    /** current column*/\
    int theColumn;\
    /** tab-width for location-computation */\
    int tabWidth; \
    /** token-buffering */\
    FSMSymbolString tokbuf;\
    /** whether to clear the token-buffer on 'tokbuf_append()' */\
    bool tokbuf_clear;\
  private: \
    /* private local data */ \
    bool use_string; \
    char *stringbuf; \
  public: \
    /** virtual destructor to shut up gcc */\
    virtual ~dwdstAlphabetLexer(void) {};\
    /** use stream input */\
    void select_streams(FILE *in=stdin, FILE *out=stdout); \
    /** use string input */\
    void select_string(const char *in, FILE *out=stdout); \
    /** for token-buffering: append yyleng characters of yytext to 'tokbuf' */\
    inline void tokbuf_append(char *text, int leng);

%define CONSTRUCTOR_INIT :\
  theLine(1), \
  theColumn(1), \
  tabWidth(8), \
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
  return result= fread(buffer, 1, max_size, YY_dwdstAlphabetLexer_IN);



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

"#"({nonnewline}*) {
   // -- ignore comments
   theColumn += yyleng;
}

(" "+)/"\t" {
  // -- spaces preceeding tab: ignore
  theColumn += yyleng;
}

"\t"(" "*) {
  // -- tab: round to $tabWidth characters
  theColumn += tabWidth - (theColumn % tabWidth) + yyleng - 1;
  return '\t';
}

{textchar}({textorsp}*{textchar})? {
  // -- any other text: append to the token-buffer
  theColumn += yyleng;
  yylval->symstr = new FSMSymbolString((const char *)yytext);
  return dwdstAlphabetParser::SYMBOL;
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
  fprintf(stderr,
          "dwdstAlphabetLexer warning: unknown character '%c': ignored.\n",
          yytext[YY_MORE_ADJ]);
}

%%

// -- co-exist with BumbleBee 'clex.h'
#ifdef REJECT
# undef REJECT
#endif



/*----------------------------------------------------------------------
 * Local Methods for dwdstAlphabetLexer
 *----------------------------------------------------------------------*/

/*
 * void dwdstAlphabetLexer::select_streams(FILE *in, FILE *out)
 */
void dwdstAlphabetLexer::select_streams(FILE *in, FILE *out) {
  yyin = in;
  yyout = out;
  use_string = false;

  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}

/*
 * void dwdstAlphabetLexer::select_string(const char *in, FILE *out=stdout)
 */
void dwdstAlphabetLexer::select_string(const char *in, FILE *out=stdout) {
  select_streams(stdin,out);  // flex __really__ wants a real input stream

  // -- string-buffer stuff
  use_string = true;
  stringbuf = (char *)in;
}


/*
 * tokbuf_append(text,leng)
 */
inline void dwdstAlphabetLexer::tokbuf_append(char *text, int leng) {
  if (tokbuf_clear) {
    tokbuf = (char *)text;
    tokbuf_clear = false;
  } else {
    tokbuf.append((char *)text,leng);
  }
}
