/* -*- Mode: C++ -*-*/
/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2014 by Bryan Jurish <moocow@cpan.org>

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


#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <moot.h>

#if 0
#include <mootHMM.h>
#include <mootLexfreqs.h>
#include <mootClassfreqs.h>
#include <mootNgrams.h>
#include <mootCIO.h>
#include <mootUtils.h>
#include <mootModelSpec.h>
#endif

using namespace std;
using namespace moot;
using namespace mootio;

namespace moot {
  // -- global classes/structs
  extern const char *PROGNAME;
  extern int	     vlevel;

  template <typename ArgsT, class HMMT=mootHMM>
  class HmmSpec {
  public:
    typedef ArgsT Args;

  public:
    Args args;
    HMMT* hmmp;

  public:
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
    HmmSpec()
    { memset(this,0,sizeof(HmmSpec)); };

    /** set compile-time options (clobber); uses globals: PROGNAME */
    void set_hmm_compile_options();

    /** set runtime options; uses globals: PROGNAME */
    void set_hmm_runtime_options(bool given_only=false);

    /** load hmm (global); calls set_hmm_compile_options(), set_hmm_runtime_options() */
    bool load_hmm(bool try_bin=true);

    /** utility for compile-only option checking; returns \a opt_given */
    bool check_compile_option(const char *opt_name, bool opt_given);
  };

  //-- header stuff, top-level (globals: PROGNAME)
  void put_hmm_header(TokenWriter *writer, mootHMM    &hmm);
  void put_hmm_header(TokenWriter *writer, mootDynHMM &dyn);

  //-- header stuff, low-level (globals: PROGNAME)
  void put_hmm_header_begin(TokenWriter *writer);
  void put_hmm_header_config(TokenWriter *writer, mootHMM    &hmm);
  void put_hmm_header_config(TokenWriter *writer, mootDynHMM &dyn);
  void put_hmm_header_end(TokenWriter *writer);


  //############################################################################
  //############################################################################

  //--------------------------------------------------------------------------
  template <typename SpecT, class HMMT>
  void HmmSpec<SpecT,HMMT>::set_hmm_compile_options()
  {
    //-- hmm: compile-time options
    hmmp->hash_ngrams = hash_ngrams_arg();
    hmmp->unknown_lex_threshhold = unknown_lex_threshhold_arg();
    hmmp->unknown_class_threshhold = unknown_class_threshhold_arg();
#ifdef MOOT_ENABLE_SUFFIX_TRIE
    hmmp->suftrie.maxlen() = trie_depth_arg();
    hmmp->suftrie.maxcount = trie_threshhold_arg();
    hmmp->suftrie.theta = trie_theta_arg();
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
      hmmp->nglambda1 = nlambdas_[0];
      hmmp->nglambda2 = nlambdas_[1];
#ifdef MOOT_USE_TRIGRAMS
      hmmp->nglambda3 = nlambdas_[2];
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
      hmmp->wlambda0 = wlambdas_[0];
      hmmp->wlambda1 = wlambdas_[1];
      if (hmmp->wlambda0 + hmmp->wlambda1 != 1)
	moot_msg(vlevel,vlWarnings,"%s: Warning: Lexical smoothing constants do not sum to one: %s\n", PROGNAME, wlambdas_arg());
    }
  
    // -- parse and assign lexical smoothing constants (clambdas)
    if (clambdas_given()) {
      double clambdas_[2] = {1,0};
      if (!moot_parse_doubles(clambdas_arg(), clambdas_, 2))
	moot_croak("%s: could not parse lexical-class smoothing constants '%s'\n", PROGNAME, clambdas_arg());
      hmmp->clambda0 = clambdas_[0];
      hmmp->clambda1 = clambdas_[1];
      if (hmmp->clambda0 + hmmp->clambda1 != 1)
	moot_msg(vlevel,vlWarnings,"%s: Warning: Lexical-class smoothing constants do not sum to one: %s\n", PROGNAME, clambdas_arg());
    }
  };

  //--------------------------------------------------------------------------
  template <typename SpecT, class HMMT>
  void HmmSpec<SpecT,HMMT>::set_hmm_runtime_options(bool given_only)
  {
    //-- runtime options
    if (!given_only) {
      //-- clobber / initialize
      hmmp->unknown_token_name(unknown_token_arg());
      hmmp->unknown_tag_name(unknown_tag_arg());
      hmmp->relax = relax_arg();
      hmmp->use_lex_classes = use_classes_arg();
      hmmp->use_flavors = use_flavors_arg();
      hmmp->beamwd = beam_width_arg();
      hmmp->save_ambiguities = save_ambiguities_arg();
      hmmp->save_mark_unknown = save_mark_unknown_arg();
      hmmp->ndots = ndots_arg();
      hmmp->verbose = vlevel;
    } else {
      //-- explicit override only
      if (unknown_token_given()) hmmp->unknown_token_name(unknown_token_arg());
      if (unknown_tag_given()) hmmp->unknown_tag_name(unknown_tag_arg());
      if (relax_given()) hmmp->relax = relax_arg();
      if (use_classes_given()) hmmp->use_lex_classes = use_classes_arg();
      if (use_flavors_given()) hmmp->use_flavors = use_flavors_arg();
      if (beam_width_given()) hmmp->beamwd = log(beam_width_arg());
      if (save_ambiguities_given()) hmmp->save_ambiguities = save_ambiguities_arg();
      if (save_mark_unknown_given()) hmmp->save_mark_unknown = save_mark_unknown_arg();
      if (ndots_given()) hmmp->ndots = ndots_arg();
      hmmp->verbose = vlevel;
    }
  };

  //--------------------------------------------------------------------------
  template <typename SpecT, class HMMT>
  bool HmmSpec<SpecT,HMMT>::check_compile_option(const char *opt_name, bool opt_given)
  {
    if (opt_given)
      moot_msg(vlevel,vlWarnings, "%s: Warning: ignoring model compilation option `--%s' for binary model `%s'\n", PROGNAME, opt_name, model_arg());
    return opt_given;
  };

  //--------------------------------------------------------------------------
  template <typename SpecT, class HMMT>
  bool HmmSpec<SpecT,HMMT>::load_hmm(bool try_bin)
  {
    //-- check whether compile-time-only options were specified for binary model
    set_hmm_runtime_options();
    if ( try_bin && moot_file_exists(model_arg()) ) {
      check_compile_option("hash-ngrams", hash_ngrams_given());
      check_compile_option("unknown-threshhold",unknown_lex_threshhold_given());
      check_compile_option("class-threshhold",unknown_class_threshhold_given());
#ifdef MOOT_ENABLE_SUFFIX_TRIE
      check_compile_option("trie-depth",trie_depth_given());
      check_compile_option("trie-threshhold",trie_threshhold_given());
      check_compile_option("trie-theta",trie_theta_given());
#endif
      check_compile_option("nlambdas",nlambdas_given());
      check_compile_option("wlambdas",wlambdas_given());
      check_compile_option("clambdas",clambdas_given());
    } else {
      set_hmm_compile_options();
    }

    //-- actually load model
    if (!hmmp->load_model(model_arg(),
			  eos_tag_arg(),
			  PROGNAME,
			  (!nlambdas_given()),
			  (!wlambdas_given()),
			  (!clambdas_given())))
      {
	moot_croak("%s: could not load model `%s'\n", PROGNAME, model_arg());
      }

    //-- re-set runtime options (override)
    set_hmm_runtime_options(true);
    return true;
  };

}

#endif // MOOT_COMPUTILS_H
