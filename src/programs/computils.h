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
  extern mootHMM     hmm;
  extern int	     vlevel;

  template <typename ArgsT>
  class HmmSpec {
  public:
    typedef ArgsT Args;

  public:
    Args args;

    int hash_ngrams_arg()   { return args.hash_ngrams_arg; };
    bool hash_ngrams_given() { return args.hash_ngrams_given; };

    int relax_arg()   { return args.relax_arg; };
    bool relax_given() { return args.relax_given; };

    int use_classes_arg()   { return args.use_classes_arg; };
    bool use_classes_given() { return args.use_classes_given; };

    int use_flavors_arg()   { return args.use_flavors_arg; };
    bool use_flavors_given() { return args.use_flavors_given; };

    const char * unknown_token_arg() { return args.unknown_token_arg; };
    bool unknown_token_given() { return args.unknown_token_given; };

    const char * unknown_tag_arg() { return args.unknown_tag_arg; };
    bool unknown_tag_given() { return args.unknown_tag_given; };

    const char * eos_tag_arg() { return args.eos_tag_arg; };
    bool eos_tag_given() { return args.eos_tag_given; };

    ProbT unknown_lex_threshhold_arg() { return args.unknown_threshhold_arg; };
    bool unknown_lex_threshhold_given() { return args.unknown_threshhold_given; };

    ProbT unknown_class_threshhold_arg() { return args.class_threshhold_arg; };
    bool unknown_class_threshhold_given() { return args.class_threshhold_given; };

    ProbT beam_width_arg() { return args.beam_width_arg; };
    bool beam_width_given() { return args.beam_width_given; };

    int trie_depth_arg() { return args.trie_depth_arg; };
    bool trie_depth_given() { return args.trie_depth_given; };

    int trie_threshhold_arg() { return args.trie_threshhold_arg; };
    bool trie_threshhold_given() { return args.trie_threshhold_given; };

    ProbT trie_theta_arg() { return args.trie_theta_arg; };
    bool trie_theta_given() { return args.trie_theta_given; };

    const char * nlambdas_arg() { return args.nlambdas_arg; };
    bool nlambdas_given() { return args.nlambdas_given; };

    const char * wlambdas_arg() { return args.wlambdas_arg; };
    bool wlambdas_given() { return args.wlambdas_given; };

    const char * clambdas_arg() { return args.clambdas_arg; };
    bool clambdas_given() { return args.clambdas_given; };

    int save_ambiguities_arg() { return args.save_ambiguities_flag; };
    bool save_ambiguities_given() { return args.save_ambiguities_given; };

    int save_mark_unknown_arg() { return args.mark_unknown_flag; };
    bool save_mark_unknown_given() { return args.mark_unknown_given; };

    size_t ndots_arg() { return args.dots_arg; };
    bool ndots_given() { return args.dots_given; };

    const char * model_arg() { return args.model_arg; };
    bool model_given() { return args.model_given; };

  public:
    HmmSpec(void)
    { memset(this,0,sizeof(HmmSpec)); };

    void set_hmm_compile_options(); //-- globals: hmm, PROGNAME
    void set_hmm_runtime_options(); //-- globals: hmm, PROGNAME
    bool load_hmm(bool try_bin=true); //-- globals: hmm, PROGNAME
  };

  //############################################################################
  //############################################################################

  //--------------------------------------------------------------------------
  template <typename SpecT>
  void HmmSpec<SpecT>::set_hmm_compile_options()
  {
    //-- hmm: compile-time options
    hmm.hash_ngrams = hash_ngrams_arg();
    hmm.unknown_lex_threshhold = unknown_lex_threshhold_arg();
    hmm.unknown_class_threshhold = unknown_class_threshhold_arg();
#ifdef MOOT_ENABLE_SUFFIX_TRIE
    hmm.suftrie.maxlen() = trie_depth_arg();
    hmm.suftrie.maxcount = trie_threshhold_arg();
    hmm.suftrie.theta = trie_theta_arg();
#else
    if (trie_depth_given() || trie_threshhold_given() || trie_theta_given()) {
      moot_msg(vlevel,vlWarning, "%s: Warning: suffix trie support disabled: ignoring trie-related option(s)\n", PROGNAME);
    }
#endif

    // -- parse and assign n-gram smoothing constants (nlambdas)
    if (nlambdas_given()) {
      double nlambdas_[3] = {0,1,0};
      if (!moot_parse_doubles(nlambdas_arg(), nlambdas_, 3))
	moot_croak("%s: could not parse n-gram smoothing constants '%s'\n", PROGNAME, nlambdas_arg());
      hmm.nglambda1 = nlambdas_[0];
      hmm.nglambda2 = nlambdas_[1];
#ifdef MOOT_USE_TRIGRAMS
      hmm.nglambda3 = nlambdas_[2];
#else
      if (nlambdas_[2] != 0)
	moot_msg(vlevel,vlWarnings,"%s: Warning: use of trigrams disabled.\n", PROGNAME);
#endif
      if (nlambdas_[0]+nlambdas_[1]+nlambdas_[2] != 1)
	moot_msg(vlevel,vlWarnings,"%s: Warning: n-gram smoothing constants do not sum to one: %s\n", PROGNAME, nlambdas_arg());
    }

    // -- parse and assign lexical smoothing constants (wlambdas)
    if (wlambdas_given()) {
      double wlambdas_[2] = {1,0};
      if (!moot_parse_doubles(wlambdas_arg(), wlambdas_, 2))
	moot_croak("%s: could not parse lexical smoothing constants '%s'\n", PROGNAME, wlambdas_arg());
      hmm.wlambda0 = wlambdas_[0];
      hmm.wlambda1 = wlambdas_[1];
      if (hmm.wlambda0 + hmm.wlambda1 != 1)
	moot_msg(vlevel,vlWarnings,"%s: Warning: Lexical smoothing constants do not sum to one: %s\n", PROGNAME, wlambdas_arg());
    }
  
    // -- parse and assign lexical smoothing constants (clambdas)
    if (clambdas_given()) {
      double clambdas_[2] = {1,0};
      if (!moot_parse_doubles(clambdas_arg(), clambdas_, 2))
	moot_croak("%s: could not parse lexical-class smoothing constants '%s'\n", PROGNAME, clambdas_arg());
      hmm.clambda0 = clambdas_[0];
      hmm.clambda1 = clambdas_[1];
      if (hmm.clambda0 + hmm.clambda1 != 1)
	moot_msg(vlevel,vlWarnings,"%s: Warning: Lexical-class smoothing constants do not sum to one: %s\n", PROGNAME, clambdas_arg());
    }
  };

  //--------------------------------------------------------------------------
  template <typename SpecT>
  void HmmSpec<SpecT>::set_hmm_runtime_options()
  {
    //-- runtime options
    hmm.unknown_token_name(unknown_token_arg());
    hmm.unknown_tag_name(unknown_tag_arg());
    hmm.relax = relax_arg();
    hmm.use_lex_classes = use_classes_arg();
    hmm.use_flavors = use_flavors_arg();
    hmm.beamwd = beam_width_arg();
    hmm.save_ambiguities = save_ambiguities_arg();
    hmm.save_mark_unknown = save_mark_unknown_arg();
    hmm.ndots = ndots_arg();
    hmm.verbose = vlevel;
  };

  //--------------------------------------------------------------------------
  template <typename SpecT>
  bool HmmSpec<SpecT>::load_hmm(bool try_bin)
  {
    set_hmm_runtime_options();
    set_hmm_compile_options();

    //-- actually load model
    if (!hmm.load_model(model_arg(),
			eos_tag_arg(),
			PROGNAME,
			(!nlambdas_given()),
			(!wlambdas_given()),
			(!clambdas_given())))
      {
	moot_croak("%s: could not load model `%s'\n", PROGNAME, model_arg());
      }

    //-- check whether compile-time-only options were specified for binary model
    if (try_bin && moot_file_exists(model_arg())
	&& (hash_ngrams_given()
	    || unknown_lex_threshhold_given()
	    || unknown_class_threshhold_given()
#ifdef MOOT_ENABLE_SUFFIX_TRIE
	    || trie_depth_given()
	    || trie_threshhold_given()
	    || trie_theta_given()
#endif
	    || nlambdas_given()
	    || wlambdas_given()
	    || clambdas_given()))
      {
	moot_msg(vlevel,vlWarnings, "%s: Warning: model compilation option(s) are meaningless for binary model '%s'\n", PROGNAME, model_arg());
      }

    //-- re-set runtime options
    set_hmm_runtime_options();    
    return true;
  };

  

}

#endif // MOOT_COMPUTILS_H
