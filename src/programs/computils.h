/* -*- Mode: C++ -*-*/
/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2012 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: computils.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot HMM PoS tagger/disambiguator : compilation utilities
 *--------------------------------------------------------------------------*/
#ifndef MOOT_COMPUTILS_H
#define MOOT_COMPUTILS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <mootHMM.h>
#include <mootLexfreqs.h>
#include <mootClassfreqs.h>
#include <mootNgrams.h>
#include <mootCIO.h>
#include <mootUtils.h>
#include <mootModelSpec.h>

using namespace std;
using namespace moot;
using namespace mootio;

namespace moot {
  // -- global classes/structs
  extern const char *PROGNAME;
  extern int         vlevel;
  extern mootHMM     hmm;

  class HmmSpec {
  public:
    int hash_ngrams;
    int relax;
    int use_classes;
    int use_flavors;
    const char *unknown_token_name;
    const char *unknown_tag_name;
    const char *eos_tag;
    ProbT unknown_lex_threshhold;
    ProbT unknown_class_threshhold;
    ProbT beam_width;
    int trie_depth;
    int trie_threshhold;
    ProbT trie_theta;
    int trie_args_given;
    const char *nlambdas;
    const char *wlambdas;
    const char *clambdas;
    //
    //-- runtime options
    int save_ambiguities;
    int save_mark_unknown;
    size_t ndots;
  public:
    HmmSpec(void)
    { memset(this,0,sizeof(HmmSpec)); };
  };
  extern HmmSpec hs;

  bool load_hmm(const char *modelname, bool try_bin=true); //-- uses hs, hmm, vlevel, PROGNAME
}

#endif // MOOT_COMPUTILS_H
