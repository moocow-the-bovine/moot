/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstTaggerLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + lexer for KDWDS tagger
 *   + assumes pre-tokenized input
 *     - space-separated tokens
 *     - one sentence per line
 *     - raw text (no markup!)
 *   + process with Coetmeur's flex++ to produce 'dwdstTaggerLexer.cc'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdstTaggerLexer

%define CLASS dwdstTaggerLexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
    /** \brief typedef for token-types */\
    typedef enum {\
      DTEOF,\
      UNKNOWN,\
      EOS,\
      TOKEN\
    } TokenType;\
  public: \
    /* -- local methods */ \
    /** \brief hack for non-global yywrap() */\
    void step_streams(FILE *in, FILE *out);

%header{
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/*!
 * \class dwdstTaggerLexer
 * \brief
 * Flex++ lexer for KDWDS tagger.  Assumes pre-tokenized input:
 * space-separated tokens, one sentence per line, raw text (no markup!).
 */

%}


/*----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/
space      [ \t]
newline    [\n\r]
wchar      [^ \t\n\r]

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%

{newline}+                     { return EOS; }
{space}+                       { /* ignore spaces */ }

^{space}*\#([^\r\n]*){newline} { /* ignore comments */ }

{wchar}+                       { return TOKEN; }
.                              { return UNKNOWN; }

<<EOF>>                        { return DTEOF; }

%%


/*----------------------------------------------------------------------
 * dwdstTaggerLexer::step_streams(FILE *in, FILE *out)
 *   + hack for non-global yywrap()
 *----------------------------------------------------------------------*/
void dwdstTaggerLexer::step_streams(FILE *in, FILE *out)
{
  yyin = in;
  yyout = out;
  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}
