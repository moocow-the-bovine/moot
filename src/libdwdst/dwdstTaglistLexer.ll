/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstTaglistLexer.ll
 * Author: Bryan Jurish
 * Description:
 *   + lexer for KDWDS tagger PoS-tag lists
 *   + input format:
 *     - PoS tags are space- and/or newline-separated
 *     - comment-lines introduced with '#'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdstTaglistLexer

%define CLASS dwdstTaglistLexer
%define MEMBERS \
  public: \
    /* -- public typedefs */\
    /** \brief typedef for token-types */\
    typedef enum { \
     PTEOF, \
     PTUNKNOWN, \
     POSTAG \
    } TokenType; \
  \
  public: \
    /* -- local methods */ \
    /** \brief hack for non-global yywrap() */\
    void step_streams(FILE *in, FILE *out);

%header{
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/*!
 * \class dwdstTaglistLexer
 * \brief
 * Flex++ lexer for KDWDS tagger PoS-tag lists.
 * Input format: PoS tags are space- and/or newline-separated.
 * Comment-lines introduced with '#'.
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

({space}|{newline})+           { /* ignore and newlines */ }
^{space}*\#([^\r\n]*){newline} { /* ignore comments */ }

{wchar}+                       { return POSTAG; }
<<EOF>>                        { return PTEOF; }
.			       { return PTUNKNOWN; }

%%


/*----------------------------------------------------------------------
 * dwdstTaglistLexer::step_streams(FILE *in, FILE *out)
 *   + hack for non-global yywrap()
 *----------------------------------------------------------------------*/
void dwdstTaglistLexer::step_streams(FILE *in, FILE *out)
{
  yyin = in;
  yyout = out;
  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}
