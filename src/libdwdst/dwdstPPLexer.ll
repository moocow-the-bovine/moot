/*-*- Mode: Flex++ -*-*/
/*----------------------------------------------------------------------
 * Name: dwdstPPLexer.ll
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + preprocessor for the KDWDS tagger
 *   + based on 'Bbaw.l' by Thomas Hanneforth
 *   + process with Coetmeur's flex++ to produce 'dwdst_lexer.cc'
 *----------------------------------------------------------------------*/

/* --- Lexer name --- */
%name dwdstPPLexer

%header{
/*============================================================================
 * Doxygen docs
 *============================================================================*/
/*!
 * \class dwdstPPLexer
 * \brief Flex++ lexer for 'dwdspp' raw-text preprocessor:
 * Does rudimentary end-of-sentence and abbreviation recognition,
 * and filters out XML markup.
 */

%}

%define CLASS dwdstPPLexer
%define MEMBERS \
  public: \
  /* -- public typedefs */\
  /** typedef for token-types */\
  typedef enum { \
    PPEOF, \
    UNKNOWN, \
    EOS, \
    START_XML_TAG, \
    END_XML_TAG, \
    WORD, \
    ABBREV, \
    HYPHWORD, \
    INTEGER, \
    FLOAT, \
    PUNCT, \
    DATE \
  } TokenType; \
  \
  public: \
    /* -- public local data */ \
    /** enable verbose reporting (track ntokens)?  */\
    bool verbose; \
    /** number of tokens processed (for verbose mode) */\
    unsigned int ntokens; \
    \
    /* -- local methods */ \
    /** preprocess a C-stream */\
    bool tokenize_stream(FILE *in=stdin, FILE *out=stdout); \
    /** hack for non-global yywrap() */\
    void step_streams(FILE *in, FILE *out);

%define CONSTRUCTOR_CODE \
  ntokens = 0;


/*----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/
digit			[0-9]
uppercase_letter	[A-ZÄÖÜ]
lowercase_letter        [a-zäöüß]
dash			"-"

letter			[A-Za-z]
german_word		([A-Za-z]|"ä"|"ö"|"ü"|"Ä"|"Ö"|"Ü"|"ß"|"é"|"è"|"á"|"à")+
january			(([Jj][Aa][Nn]\.)|([Jj][Aa][Nn][Uu][Aa][Rr]))
february		(([Ff][Ee][Bb]\.)|([Ff][Ee][Bb][Rr][Uu][Aa][Rr]))
march			([Mm]("Ä"|"ä")[Rr][Zz])
april			(([Aa][Pp][Rr]\.)|([Aa][Pp][Rr][Ii][Ll]))
may			([Mm][Aa][Ii])
june			(([Jj][Uu][Nn]\.)|([Jj][Uu][Nn][Ii]))
july			(([Jj][Uu][Ll]\.)|([Jj][Uu][Ll][Ii]))
august			(([Aa][Uu][Gg]\.)|([Aa][Uu][Gg][Uu][Ss][Tt]))
september		(([Ss][Ee][Pp]\.)|([Ss][Ee][Pp][Tt][Ee][Mm][Bb][Ee][Rr]))
october			(([Oo][Kk][Tt]\.)|([Oo][Kk][Tt][Oo][Bb][Ee][Rr]))
november		(([Nn][Oo][Vv]\.)|([Nn][Oo][Vv][Ee][Mm][Bb][Ee][Rr]))
december		(([Dd][Ee][Zz]\.)|([Dd][Ee][Zz][Ee][Mm][Bb][Ee][Rr]))


month_name		({january}|{february}|{march}|{april}|{may}|{june}|{july}|{august}|{september}|{october}|{november}|{december})
day			((0?[1-9]\.)|([12][0-9]\.)|(3[01]\.))
month			(((0?[1-9])|10|11|12|1)\.)
year			([0-9]|([1-9][0-9][0-9]?[0-9]?))

/*----------------------------------------------------------------------
 * Rules
 *----------------------------------------------------------------------*/
%%

\<(\/?)(s|p|sp|lg)\>					{ return EOS; }
\<([\?\!]?){letter}+[^\>]*\>		                { return START_XML_TAG; }
\<\/{letter}+([0-9\.\_\-]*)\>				{ return END_XML_TAG; }

&#[0-9]+;						{ /* ignore XML char-entities */ }
"&dash;"						{ /* ditto */ }

({day})([ \t])*({month_name}|{month})([ \t])*({year})	{ return DATE;  }


[\+\-]?([0-9]+)						{ return INTEGER; }
[\+\-]?[0-9]*[\.,]([0-9]+)				{ return FLOAT; }

{german_word}-\n{german_word}				{ return HYPHWORD; }
{german_word}-{german_word}-{german_word}		{ return WORD; }
{german_word}-{german_word}				{ return WORD; }
{german_word}						{ return WORD; }


"bzw."							{ return ABBREV; }
"usw."							{ return ABBREV; }
"usf."							{ return ABBREV; }
"etc."		                                        { return ABBREV; }
"o.ä."							{ return ABBREV; }
"u.a."							{ return ABBREV; }
"Abb."							{ return ABBREV; }
"Art."							{ return ABBREV; }
"Anm."							{ return ABBREV; }
[Dd]".h."			  			{ return ABBREV; }
"Chr."							{ return ABBREV; }
"Dr."							{ return ABBREV; }
"Dr.h.c."						{ return ABBREV; }
"Dr.-Ing."						{ return ABBREV; }
"Dr.-Ing.E."					        { return ABBREV; }
"Dr.jur."						{ return ABBREV; }
"Dr.med.Dr."						{ return ABBREV; }
"Dr.phil."						{ return ABBREV; }
"geb."							{ return ABBREV; }
"gez."							{ return ABBREV; }
"Kl."							{ return ABBREV; }
"Kto.-Nr."		  				{ return ABBREV; }
"Inc."							{ return ABBREV; }
"lt."							{ return ABBREV; }
"Ltd."							{ return ABBREV; }
"M.d.B."						{ return ABBREV; }
"Mr."							{ return ABBREV; }
"Mrs."							{ return ABBREV; }
"Nachf."						{ return ABBREV; }
"Mill."							{ return ABBREV; }
"Nr."							{ return ABBREV; }
"pp."							{ return ABBREV; }
"Pf."							{ return ABBREV; }
"o.k."							{ return ABBREV; }
"s.u."							{ return ABBREV; }
"u."							{ return ABBREV; }
"o."							{ return ABBREV; }
"z.B."							{ return ABBREV; }
"z.T."							{ return ABBREV; }
"vgl."							{ return ABBREV; }

"."							{ return EOS; }

";"							{ return PUNCT; }
"?"							{ return PUNCT; }
"!"							{ return PUNCT; }
":"							{ return PUNCT; }
","							{ return PUNCT; }

[ \t]							{ /* do nothing */ }
[\n\r]							{  }
.							{ return UNKNOWN; }

<<EOF>>                                                 { return PPEOF; }

%%

/*----------------------------------------------------------------------
 */
void dwdstPPLexer::step_streams(FILE *in, FILE *out)
{
  yyin = in;
  yyout = out;
  // -- black magic from flex(1) manpage
  if (yy_current_buffer != NULL) { yy_delete_buffer(yy_current_buffer); }
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  BEGIN(INITIAL);
}

/*----------------------------------------------------------------------
 */
bool dwdstPPLexer::tokenize_stream(FILE *in=stdin, FILE *out=stdout)
{
  int tok;
  step_streams(in,out);

  while ((tok = yylex()) != PPEOF) {
    if (verbose) ntokens++;
    switch (tok) {
      case EOS:
          if (yytext[0] != '<') {
            yy_echo();
	    fputc(' ',out);
          }
          fputc('\n', out);

      case  START_XML_TAG:
      case END_XML_TAG:
	  /* ignore XML tags */
	  break;

      default:
	  /* write it as its own token */
          yy_echo();
          fputc(' ', out);
      }
  }
  fputc('\n', out);
  yyterminate();
  return true;
}
