/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstTaggerLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + lexer for KDWDS tagger
 *   + assumes pre-tokenized input
 *     - one token per line
 *     - n>=0 (possible) tags per token
 *     - blank lines mark end-of-sentence
 *     - supports line-comments introduced by '%%'
 *     - raw text (no markup!)
 *   + process with Coetmeur's flex++ to produce 'dwdstTaggerLexer.cc'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdstTaggerLexer

%define CLASS dwdstTaggerLexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
    /** typedef for token-types */\
    typedef enum {\
      DTEOF,   /**< end-of-file */\
      UNKNOWN, /**< unknown token */\
      EOS,     /**< end-of-sentence */\
      TOKEN,   /**< single token */\
      TAG,     /**< single tag */\
      EOT,     /**< end-of-token */\
      TAB,     /**< tab: internal use only */\
      SPACE    /**< space: internal use only */\
    } TokenType;\
    \
  public: \
   /* -- local data */ \
   /** current line*/\
   int theLine;\
   \
   /** current column*/\
   int theColumn;\
   \
   /** Last token-type returned */\
   TokenType lasttok; \
  public: \
    /* -- local methods */ \
    /** hack for non-global yywrap() */\
    void step_streams(FILE *in, FILE *out);

%define CONSTRUCTOR_INIT :\
  theLine(1), \
  theColumn(1),\
  lasttok(DTEOF)

%header{
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/**
 * \class dwdstTaggerLexer
 * \brief Flex++ lexer for KDWDS tagger.
 *
 * Assumes pre-tokenized input:
 * one token per line,  blank lines = EOS, raw text only (no markup!).
 * Format: 1 tok/line, comments introduced with '%%'.  Supports
 * multiple tags/tok.
 */

%}


/*----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/
space      [ ]
tab        [\t]
newline    [\n\r]
wchar      [^ \t\n\r]

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%

^([ \t]*){newline} {
  theLine++; theColumn=0;
  if (lasttok != EOS) {
    lasttok=EOS;
    return EOS;
  }
}

{newline} {
  theLine += yyleng;
  theColumn = 0;
  lasttok=EOT;
  return EOT;
}

^([ \t]*)"%%"([^\r\n]*){newline} {
  /* mostly ignore comments */
  theLine++;
  theColumn = 0;
  lasttok=SPACE;
}

{tab}+ {
  /* mostly ignore tabs */
  theColumn += 8*yyleng;
  lasttok=TAB;
}

^[^\t\r\n]+ {
  theColumn += yyleng;
  lasttok=TOKEN;
  return TOKEN;
}

[^\t\r\n]+ {
  theColumn += yyleng;
  lasttok=TAG;
  return TAG;
}

{space}+ {
  /* mostly ignore spaces */
  theColumn += yyleng;
  lasttok=SPACE;
}
                                 
. {
  theColumn += yyleng;
  lasttok=UNKNOWN;
  return UNKNOWN;
}

<<EOF>> {
  switch (lasttok) {
   case EOT:
     lasttok = EOS;
     break;
   case EOS:
   case DTEOF:
     lasttok = DTEOF;
     break;
   default:
     lasttok = EOT;
  }
  return lasttok;
}

%%


/*----------------------------------------------------------------------
 * dwdstTaggerLexer::step_streams(FILE *in, FILE *out)
 *   + hack for non-global yywrap()
 *----------------------------------------------------------------------*/
void dwdstTaggerLexer::step_streams(FILE *in, FILE *out)
{
  yyin = in;
  yyout = out;
  lasttok = DTEOF;
  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}
