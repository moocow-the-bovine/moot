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
   /* -- local data */ \
   /** \brief current line*/\
   int theLine;\
   /** \brief current column*/\
   int theColumn;\
  public: \
    /* -- local methods */ \
    /** \brief hack for non-global yywrap() */\
    void step_streams(FILE *in, FILE *out);

%define CONSTRUCTOR_INIT :\
  theLine(1), \
  theColumn(1)

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
newline    (\r?\n)
wchar      [^ \t\n\r]

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%

{newline}{newline}+            { theLine += yyleng; theColumn = 0; return EOS; }
{space}+                       { theColumn += yyleng; /* ignore spaces */ }

^{space}*\#([^\r\n]*){newline} { theLine++; theColumn = 0; /* ignore comments */ }

{wchar}+                       { theColumn += yyleng; return TOKEN; }
.                              { theColumn += yyleng; return UNKNOWN; }

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
