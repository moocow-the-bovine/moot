/*--------------------------------------------------------------------------
 * File: dwdstHMMTrainer.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + HMM-Trainer for KDWDS PoS-tagger: the guts
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#include "dwdstHMMTrainer.h"
#include "dwdstTaggerLexer.h"

DWDST_BEGIN_NAMESPACE

/*------------------------------------------------------------
 * Constructor / destructor
 */
// (none)

/*------------------------------------------------------------
 * Reset / Clear
 */
//void dwdstHMMTrainer::clear(void)

/*------------------------------------------------------------
 * Top-level Training
 */
bool dwdstHMMTrainer::train_from_file(const char *filename)
{
  FILE *file = fopen(filename,"r");
  if (!file) {
    carp("dwdstHMMTrainer::train_from_file(): open failed for file '%s': %s\n",
	 filename, strerror(errno));
    return false;
  }
  bool rc = train_from_stream(file, filename);
  fclose(file);
  return rc;
}

bool dwdstHMMTrainer::train_from_stream(FILE *in, const char *filename)
{
  //-- prepare lexer
  dwdstTaggerLexer lexer;
  lexer.step_streams(in,stderr);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  //-- prepare variables
  int tok;
  dwdstTokString curtok;
  TagSet         curtags;

  //-- do training
  train_init();
  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
    switch (tok) {
      
    case dwdstTaggerLexer::TOKEN:
      curtok = (const char *)lexer.yytext;
      break;
      
    case dwdstTaggerLexer::TAG:
      curtags.insert(dwdstTagString((const char *)lexer.yytext));
      break;

    case dwdstTaggerLexer::EOT:
      train_token(curtok,curtags);
      curtags.clear();
      break;

    case dwdstTaggerLexer::EOS:
      train_eos();
      train_bos();
      break;

    default:
      carp("%s: Error: unknown token '%s' in file '%s' at line %d, column %d\n",
	   "dwdstHMMTrainer::train_stream()",
	   filename,
	   lexer.yytext,
	   lexer.theLine,
	   lexer.theColumn);
      break;
    }
  }
  return true;
}

/*------------------------------------------------------------
 * Mid-level training methods : init
 */
//void dwdstHMMTrainer::train_init(void)

/*------------------------------------------------------------
 * Mid-level training methods : bos
 */
//void dwdstHMMTrainer::train_bos(void) {

/*------------------------------------------------------------
 * Mid-level training methods : token
 */
//void dwdstHMMTrainer::train_token(const dwdstTokString &curtok, const TagSet &curtags)

//void dwdstHMMTrainer::_train_token_ng(const TagSet &curtags)

/*------------------------------------------------------------
 * Mid-level training methods : eos
 */
//void dwdstHMMTrainer::train_eos(void)


/*------------------------------------------------------------
 * Warnings / Errors
 */
void dwdstHMMTrainer::carp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

DWDST_END_NAMESPACE
