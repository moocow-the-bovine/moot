/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/


/*--------------------------------------------------------------------------
 * File: mootHMMTrainer.cc
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
#include "mootToken.h"
#include "mootTokenIO.h"


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
  //-- prepare filename
  char *myfile = strdup(filename);

  //-- prepare lexer
  TokenReader treader;
  treader.select_stream(in,myfile);

  //-- prepare variables
  mootTokenLexer::TokenType typ;

  //-- do training
  train_init();
  while ((typ = treader.get_token()) != mootTokenLexer::TLEOF) {
    switch (typ) {

    case mootTokenLexer::TLTOKEN:
      train_token(treader.token());
      break;

    case mootTokenLexer::TLEOS:
      train_eos();
      train_bos();
      break;

    default:
      carp("%s: Error: unknown token '%s' in file '%s' at line %d, column %d\n",
	   "mootHMMTrainer::train_stream()",
	   filename,
	   treader.lexer.yytext,
	   treader.lexer.theLine,
	   treader.lexer.theColumn);
      break;
    }
  }

  free(myfile);
  treader.select_stream(stdin,"-");
  return true;
}


/*
bool mootHMMTrainer::train_from_stream(FILE *in, const char *filename)
{
  //-- prepare lexer
  mootTokenReader treader;
  treader.select_stream(in,filename);

  //-- prepare variables
  mootTokenLexer::TokenType typ;

  //-- do training
  train_init();
  while ((typ = lexer.yylex()) != mootTaggerLexer::DTEOF) {
    switch (typ) {
      
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
	   "mootHMMTrainer::train_from_stream()",
	   filename,
	   lexer.yytext,
	   lexer.theLine,
	   lexer.theColumn);
      break;
    }
  }
  return true;
}
*/

/*------------------------------------------------------------
 * Mid-level training methods : init
 */
void mootHMMTrainer::train_init(void)
{
  ngset.resize((size_t)kmax);
  ng.resize((size_t)kmax);
  train_bos();
  //-- count one EOS marker (TnT compatibility hack)
  if (want_ngrams) {
    ng.clear();
    ng.push_back(eos_tag);
    ngrams.add_count(ng,1);
  }
}

/*------------------------------------------------------------
 * Mid-level training methods : bos
 */
void mootHMMTrainer::train_bos(void)
{
  if (want_ngrams) {
    for (NgramSet::ngsType::iterator ngsi = ngset.ngs.begin();
	 ngsi != ngset.ngs.end();
	 ngsi++)
      {
	ngsi->clear();
	ngsi->insert(eos_tag);
      }
  }
}


/*------------------------------------------------------------
 * Mid-level training methods : token
 */

/*-- new (hack) */
void mootHMMTrainer::train_token(const mootToken &curtok)
{
  TagSet tagset;
  curtok.tokExport(NULL,&tagset);
  train_token(curtok.toktext, tagset);
}

/*-- DEPRECATED */
void mootHMMTrainer::train_token(const mootTokString &curtok, const TagSet &curtags)
{
  CountT count = ((CountT)curtags.size());
  TagSet::const_iterator cti;

  if (want_lexfreqs) {
    //-- count lexical frequencies
    for (cti = curtags.begin();
	 cti != curtags.end();
	 cti++) {
      lexfreqs.add_count(curtok, *cti, (count ? (1.0/count) : 0.0));
    }
  }

  if (want_ngrams) _train_token_ng(curtags);
}


/*------------------------------------------------------------
 * Mid-level training methods : token : ngrams
 */

/*-- deprecated, but the real thing */
void mootHMMTrainer::_train_token_ng(const TagSet &curtags)
{
  if (!want_ngrams) return;

  //-- count kmax-grams: add in next tagset
  ngset.step(curtags);

  //-- count all current ngrams
  NgramSet::ngIterator ngsi;
  size_t len;
  CountT ngcount;
  for (len = 1; len <= kmax; len++) {
    //-- get count
    ngcount = 0;
    for (ngsi = ngset.iter_begin(len);
	 ngset.iter_valid(ngsi);
	 ngset.iter_next(ngsi))
      {
	ngcount++;
      }
    ngcount = 1.0/ngcount; //-- normalize
    
    //-- count ngrams
    for (ngsi = ngset.iter_begin(len);
	 ngset.iter_valid(ngsi);
	 ngset.iter_next(ngsi))
      {
	ngset.iter2ngram(ngsi,ng);
	//-- ignore redundant n-grams for the boundary tag
	if (len > 1 && ((ng[0] == eos_tag && ng[1] == eos_tag)
			|| (ng.back() == eos_tag && ng[len-2] == eos_tag)))
	  continue;
	ngrams.add_count(ng,ngcount);
      }
  }

  //-- hack
  last_was_eos = false;
}

/*-- new (hack) */
void mootHMMTrainer::_train_token_ng(const mootToken &curtok)
{
  TagSet curtags;
  curtok.tokExport(NULL,&curtags);
  _train_token_ng(curtags);
}


/*------------------------------------------------------------
 * Mid-level training methods : eos
 */
void mootHMMTrainer::train_eos(void)
{
  if (want_ngrams && !last_was_eos) {
    TagSet eostags;
    eostags.insert(eos_tag);
    for (int i = 1; i < kmax; i++) {
      _train_token_ng(eostags);
    }
  }
  last_was_eos = true;
}



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
