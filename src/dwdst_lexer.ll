/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdst_lexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + lexer for KDWDS tagger
 *   + assumes pre-tokenized input
 *     - space-separated tokens
 *     - one sentence per line
 *     - raw text (no markup!)
 *   + process with Coetmeur's flex++ to produce 'dwdst_lexer.cc'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdst_lexer

%define CLASS dwdst_lexer
%define MEMBERS \
  public: \
    /* local methods */ \
    void step_streams(FILE *in, FILE *out);

%header{
typedef enum {
    DTEOF,
    UNKNOWN,
    EOS,
    TOKEN
  } dwdst_token_type;
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
 * dwdst_lexer::step_streams(FILE *in, FILE *out)
 *   + hack for non-global yywrap()
 *----------------------------------------------------------------------*/
void dwdst_lexer::step_streams(FILE *in, FILE *out)
{
  yyin = in;
  yyout = out;
  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}
