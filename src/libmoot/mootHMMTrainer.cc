/*--------------------------------------------------------------------------
/*
   moot version 1.0.4 : moocow's part-of-speech tagger
   Copyright (C) 2002-2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */ * File: mootHMMTrainer.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + HMM-Trainer for KDWDS PoS-tagger: the guts
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <mootConfig.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#include "mootHMMTrainer.h"
#include "mootTaggerLexer.h"

moot_BEGIN_NAMESPACE

/*------------------------------------------------------------
 * Constructor / destructor
 */
// (none)

/*------------------------------------------------------------
 * Reset / Clear
 */
//void mootHMMTrainer::clear(void)

/*------------------------------------------------------------
 * Top-level Training
 */
bool mootHMMTrainer::train_from_file(const char *filename)
{
  FILE *file = fopen(filename,"r");
  if (!file) {
    carp("mootHMMTrainer::train_from_file(): open failed for file '%s': %s\n",
	 filename, strerror(errno));
    return false;
  }
  bool rc = train_from_stream(file, filename);
  fclose(file);
  return rc;
}

bool mootHMMTrainer::train_from_stream(FILE *in, const char *filename)
{
  //-- prepare lexer
  mootTaggerLexer lexer;
  lexer.step_streams(in,stderr);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  //-- prepare variables
  int tok;
  mootTokString curtok;
  TagSet         curtags;

  //-- do training
  train_init();
  while ((tok = lexer.yylex()) != mootTaggerLexer::DTEOF) {
    switch (tok) {
      
    case mootTaggerLexer::TOKEN:
      curtok = (const char *)lexer.yytext;
      break;
      
    case mootTaggerLexer::TAG:
      curtags.insert(mootTagString((const char *)lexer.yytext));
      break;

    case mootTaggerLexer::EOT:
      train_token(curtok,curtags);
      curtags.clear();
      break;

    case mootTaggerLexer::EOS:
      train_eos();
      train_bos();
      break;

    default:
      carp("%s: Error: unknown token '%s' in file '%s' at line %d, column %d\n",
	   "mootHMMTrainer::train_stream()",
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
//void mootHMMTrainer::train_init(void)

/*------------------------------------------------------------
 * Mid-level training methods : bos
 */
//void mootHMMTrainer::train_bos(void) {

/*------------------------------------------------------------
 * Mid-level training methods : token
 */
//void mootHMMTrainer::train_token(const mootTokString &curtok, const TagSet &curtags)

//void mootHMMTrainer::_train_token_ng(const TagSet &curtags)

/*------------------------------------------------------------
 * Mid-level training methods : eos
 */
//void mootHMMTrainer::train_eos(void)


/*------------------------------------------------------------
 * Warnings / Errors
 */
void mootHMMTrainer::carp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

moot_END_NAMESPACE
