/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: postag_lexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + lexer for KDWDS tagger PoS-tag lists
 *   + input format:
 *     - PoS tags are space- and/or newline-separated
 *     - comment-lines introduced with '#'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name postag_lexer

%define CLASS postag_lexer
%define MEMBERS \
  public: \
    /* local methods */ \
    void step_streams(FILE *in, FILE *out);

%header{
typedef enum {
    PTEOF,
    PTUNKNOWN,
    POSTAG
  } postag_token_type;
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

({space}|{newline})+           { /* ignore and newlines */ }
^{space}*\#([^\r\n]*){newline} { /* ignore comments */ }

{wchar}+                       { return POSTAG; }
<<EOF>>                        { return PTEOF; }
.			       { return PTUNKNOWN; }

%%


/*----------------------------------------------------------------------
 * postag_lexer::step_streams(FILE *in, FILE *out)
 *   + hack for non-global yywrap()
 *----------------------------------------------------------------------*/
void postag_lexer::step_streams(FILE *in, FILE *out)
{
  yyin = in;
  yyout = out;
  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}
