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
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/*!
 * \class dwdst_param_lexer
 * \brief Flex++ lexer for TnT parameter files.  Supports comments introduced with '#'.
 */

/*#define DWDST_PARLEX_BUFSIZE 1024*/

%}

%define CLASS dwdst_param_lexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
    /** \brief Return type for dwdst_param_lexer::yylex() */\
    typedef enum { \
      PF_EOF, \
      PF_REGEX, \
      PF_COUNT, \
      PF_TAB, \
      PF_NEWLINE, \
      PF_UNKNOWN \
    } TokenType; \
  public: \
   /* -- positional parameters */\
    /** \brief current line*/\
    int theLine;\
    /** \brief current column*/\
    int theColumn;\
  private: \
    /* private local data */ \
    bool use_string; \
    char *stringbuf; \
    /* argh: regex-buffereing */ \
    /*char rebuf[DWDST_PARLEX_BUFSIZE];*/ \
    /*int  rebufleng;*/ \
  public: \
    /** \brief use stream input */\
    void select_streams(FILE *in=stdin, FILE *out=stdout); \
    /** \brief use string input */\
    void select_string(const char *in, FILE *out=stdout);

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
   // -- comments : throw away
   theColumn += yyleng;
}

({whitespace}*)"\t"({whitespace}*) {
  // -- tab: return the current text-segment
  theColumn += yyleng;
  return PF_TAB;
}

{textchar}({textorsp}*{textchar})? {
  // -- append all other text to the current text-segment
  theColumn += yyleng;
  return PF_REGEX;
}

([\-\+]?)([0-9]*)(\.?)([0-9]+)([ \t]*)("%%".*)?/{newline} {
  // -- counts : return it
  theLine++; theColumn = 0;
  return PF_COUNT;
}

{newline} {
  // -- newlines : ignore (and chuck any current text!)
  theLine++; theColumn = 0;
  //return PF_NEWLINE;
}

<<EOF>> {
  // -- end-of-file
  if (*yytext) return PF_REGEX;
  return PF_EOF;
}

. {
  // -- huh? -- just ignore it!
  theColumn += yyleng;
  fprintf(stderr,"dwds_param_lexer warning: unknown character '%c': ignored.\n", yytext[YY_MORE_ADJ]);
}

%%


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
