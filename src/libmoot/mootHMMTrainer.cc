/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
*/


/*--------------------------------------------------------------------------
 * File: mootHMMTrainer.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + HMM-Trainer for moot PoS-tagger: the guts
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

//#define mootHMMTrainerDEBUG 1
#undef mootHMMTrainerDEBUG

#ifdef mootHMMTrainerDEBUG
# define DEBUG(code) code
#else
# define DEBUG(code) 
#endif

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
bool mootHMMTrainer::train_from_file(const string &filename)
{
  FILE *file = fopen(filename.c_str(),"r");
  if (!file) {
    carp("mootHMMTrainer::train_from_file(): open failed for file '%s': %s\n",
	 filename.c_str(), strerror(errno));
    return false;
  }
  bool rc = train_from_stream(file, filename);
  fclose(file);
  return rc;
}

bool mootHMMTrainer::train_from_reader(TokenReader *reader)
{
  mootTokenType typ;
  mootToken *tokptr;

  //-- do training
  train_init();
  while (reader && (typ = reader->get_token()) != TokTypeEOF) {
    DEBUG( carp("DEBUG: Got token type %d\n", typ) );
    switch (typ) {

    case TokTypeComment:
      break;

    case TokTypeVanilla:
      tokptr = reader->token();
      if (tokptr) train_token(*tokptr);
      break;

    case TokTypeEOS:
      train_eos();
      train_bos();
      break;

    default:
      reader->carp("%s: Error: unknown token '%s'",
		   "during mootHMMTrainer::train_stream()",
		   (reader->token()
		    ? reader->token()->text().c_str()
		    : "(null)"));
		   
      break;
    }
  }
  return true;
}


/*------------------------------------------------------------
 * Mid-level training methods : init
 */
void mootHMMTrainer::train_init(void)
{
  ng.resize(3);
  train_bos();
  //-- count one EOS marker (TnT compatibility hack)
  if (want_ngrams) ngrams.add_count(eos_tag,1.0);
}

/*------------------------------------------------------------
 * Mid-level training methods : bos
 */
void mootHMMTrainer::train_bos(void)
{
  DEBUG( carp("\nDEBUG: train_bos() : called\n") );
  if (want_ngrams) {
    DEBUG( carp("DEBUG: train_bos() : pre: ng=%s\n", ng.as_string().c_str()) );
    ng.clear();
    ng.push_back(eos_tag);
    DEBUG( carp("DEBUG: train_bos() : post: ng=%s\n", ng.as_string().c_str()) );
  }
  DEBUG( carp("DEBUG: train_bos() : completed.\n") );
}


/*------------------------------------------------------------
 * Mid-level training methods : token
 */

/*-- new (hack) */
void mootHMMTrainer::train_token(const mootToken &curtok)
{
  if (curtok.toktype() != TokTypeVanilla) return; //-- ignore comments, etc.

  if (curtok.besttag().empty()) {
    carp("mootHMMTrainer::train_token(): no best tag for token `%s'",
	 curtok.text().c_str());
  }

  //-- count lexical frequencies
  if (want_lexfreqs) {
    DEBUG( carp("DEBUG: train_token(`%s') : training lexfreqs\n", curtok.text().c_str()) );
    lexfreqs.add_count(curtok.text(), curtok.besttag(), 1.0);
  }

  //-- count class frequencies
  if (want_classfreqs) {
    mootTagSet lclass;
    curtok.tokExport(NULL,&lclass,false);
    lcfreqs.add_count(lclass, curtok.besttag(), 1.0);
  }

  //-- count n-gram frequencies
  if (want_ngrams) {
    DEBUG( carp("DEBUG: train_token(`%s') : training ngrams\n", curtok.text().c_str()) );

    ng.push(curtok.besttag());
    if (ng.size()>=3) ngrams.add_counts(ng, 1.0); //.. add counts if we can

    DEBUG( carp("DEBUG: train_token(`%s') : ngram=%s\n", \
	       curtok.text().c_str(), ng.as_string().c_str()) );

    //-- hack
    last_was_eos = false;
  }
}


/*------------------------------------------------------------
 * Mid-level training methods : eos
 */
void mootHMMTrainer::train_eos(void)
{
  DEBUG( carp("DEBUG: train_eos() : called\n") );
  //-- on entry, we have <t1,t2,t3> or <__$,t1>
  //   and have trained for it and all proper prefixes
  if (want_ngrams && !last_was_eos) {
    //-- train for <t2,t3,__$>
    ng.push(eos_tag);
    ngrams.add_counts(ng, 1.0);
    DEBUG( carp("DEBUG: train_eos() : trained ng=%s\n", ng.as_string().c_str()) );

    //-- train for <t3,__$>
    ng.push(eos_tag);
    ng.pop_back();
    ngrams.add_counts(ng, 1.0);
    DEBUG( carp("DEBUG: train_eos() : trained ng=%s\n", ng.as_string().c_str()) );

    //-- train (again) for <__$>
    ngrams.add_count(eos_tag, 1.0);
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

#undef DEBUG //-- this name is WAY too common to let it float out of this file...
