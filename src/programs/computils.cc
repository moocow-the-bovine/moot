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
 * File: computils.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot HMM PoS tagger/disambiguator : compilation utilities
 *--------------------------------------------------------------------------*/

#include "computils.h"

namespace moot {
  const char *PROGNAME = "moot::computils";
  int     vlevel;
  mootHMM hmm;
  HmmSpec hs;


//--------------------------------------------------------------------------
bool load_hmm(const char *modelname, bool try_bin)
{
  //-- load hmm: constants
  hmm.hash_ngrams = hs.hash_ngrams;
  hmm.relax = hs.relax;
  hmm.use_lex_classes = hs.use_classes;
  hmm.use_flavors = hs.use_flavors;
  hmm.unknown_token_name(hs.unknown_token_name);
  hmm.unknown_tag_name(hs.unknown_tag_name);
  hmm.unknown_lex_threshhold = hs.unknown_lex_threshhold;
  hmm.unknown_class_threshhold = hs.unknown_class_threshhold;
#ifdef MOOT_ENABLE_SUFFIX_TRIE
  hmm.suftrie.maxlen() = hs.trie_depth;
  hmm.suftrie.maxcount = hs.trie_threshhold;
  hmm.suftrie.theta = hs.trie_theta;
#else
  if (args.trie_depth_given || args.trie_threshhold_given || args.trie_theta_given) {
    moot_msg(vlevel,vlWarning, "%s: Warning: suffix trie support disabled: ignoring trie-related option(s)\n", PROGNAME);
  }
#endif

  //-- runtime options
  hmm.beamwd = hs.beam_width;
  hmm.save_ambiguities = hs.save_ambiguities;
  hmm.save_mark_unknown = hs.save_mark_unknown;
  hmm.ndots = hs.ndots;
  hmm.verbose = vlevel;

  // -- parse and assign n-gram smoothing constants (nlambdas)
  if (hs.nlambdas) {
    double nlambdas[3] = {0,1,0};
    if (!moot_parse_doubles(hs.nlambdas, nlambdas, 3))
      moot_croak("%s: could not parse N-Gram smoothing constants '%s'\n", PROGNAME, hs.nlambdas);
    hmm.nglambda1 = nlambdas[0];
    hmm.nglambda2 = nlambdas[1];
#ifdef MOOT_USE_TRIGRAMS
    hmm.nglambda3 = nlambdas[2];
#else
    if (nlambdas[2] != 0)
      moot_msg(vlevel,vlWarnings,"%s: Warning: use of trigrams disabled.\n", PROGNAME);
#endif
    if (nlambdas[0]+nlambdas[1]+nlambdas[2] != 1)
      moot_msg(vlevel,vlWarnings,"%s: Warning: N-gram smoothing constants do not sum to one: %s\n", PROGNAME, hs.nlambdas);
  }

  // -- parse and assign lexical smoothing constants (wlambdas)
  if (hs.wlambdas) {
    double wlambdas[2] = {1,0};
    if (!moot_parse_doubles(hs.wlambdas, wlambdas, 2))
      moot_croak("%s: could not parse lexical smoothing constants '%s'\n", PROGNAME, hs.wlambdas);
    hmm.wlambda0 = wlambdas[0];
    hmm.wlambda1 = wlambdas[1];
    if (hmm.wlambda0 + hmm.wlambda1 != 1)
      moot_msg(vlevel,vlWarnings,"%s: Warning: Lexical smoothing constants do not sum to one: %s\n", PROGNAME, hs.wlambdas);
  }
  
  // -- parse and assign lexical smoothing constants (clambdas)
  if (hs.clambdas) {
    double clambdas[2] = {1,0};
    if (!moot_parse_doubles(hs.clambdas, clambdas, 2))
      moot_croak("%s: could not parse lexical-class smoothing constants '%s'\n", PROGNAME, hs.clambdas);
    hmm.clambda0 = clambdas[0];
    hmm.clambda1 = clambdas[1];
    if (hmm.clambda0 + hmm.clambda1 != 1)
      moot_msg(vlevel,vlWarnings,"%s: Warning: Lexical-class smoothing constants do not sum to one: %s\n", PROGNAME, hs.clambdas);
  }

  //-- load model
  if (!hmm.load_model(modelname,
		      hs.eos_tag,
		      PROGNAME,
		      (hs.nlambdas==NULL),
		      (hs.wlambdas==NULL),
		      (hs.clambdas==NULL)))
    {
      moot_croak("%s: could not load model `%s'\n", PROGNAME, modelname);
    }

  return true;
};

};
