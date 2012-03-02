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
 * File: mootcompile_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + HMM PoS tagger/disambiguator for DWDS project : model compiler
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <string>

#include <mootHMM.h>
#include <mootLexfreqs.h>
#include <mootClassfreqs.h>
#include <mootNgrams.h>
#include <mootCIO.h>
#include <mootUtils.h>
#include <mootModelSpec.h>
#include "mootcompile_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "mootcompile";
int vlevel;

// options & file-churning
gengetopt_args_info  args;

// -- files
mofstream out;

// -- global classes/structs
mootModelSpec  ms;
mootHMM        hmm;
mootNgrams     ngrams;
mootLexfreqs   lexfreqs(32768);
mootClassfreqs classfreqs(512);
mootTaster     taster;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  cmdline_parser_envdefaults(&args);

  //-- locale
  moot_setlocale();

  //-- verbose
  vlevel = args.verbose_arg;

  //-- sanity check
  if (args.inputs_num <= 0)
    moot_croak("%s: cowardly refusing to compile empty model!\n", PROGNAME);

  //-- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

  //-- output file
  if (!out.open(args.output_arg,"wb")) {
    moot_croak("%s: open failed for output-file '%s': %s\n", PROGNAME, out.name.c_str(), strerror(errno));
  }
  out.close(); //-- close again: HMM will write it itself

  // -- assign "unknown" ids & other flags
  hmm.hash_ngrams = args.hash_ngrams_arg;
  hmm.relax = args.relax_arg;
  hmm.use_lex_classes = args.use_classes_arg;
  hmm.use_flavors = args.use_flavors_arg;
  hmm.unknown_token_name(args.unknown_token_arg);
  hmm.unknown_tag_name(args.unknown_tag_arg);
  hmm.unknown_lex_threshhold = args.unknown_threshhold_arg;
  hmm.unknown_class_threshhold = args.class_threshhold_arg;
#ifdef MOOT_ENABLE_SUFFIX_TRIE
  hmm.suftrie.maxlen() = args.trie_depth_arg;
  hmm.suftrie.maxcount = args.trie_threshhold_arg;
  hmm.suftrie.theta = args.trie_theta_arg;
#endif
}

/*--------------------------------------------------------------------------
 * Model I/O
 *--------------------------------------------------------------------------*/
bool load_model(const char *modelname)
{
  //-- parse model spec
  if (!ms.parse(modelname, false))
    moot_croak("%s: could not parse text model specification '%s'\n", PROGNAME, modelname);

  // -- load model: lexical frequencies
  if (!ms.lexfile.empty() && moot_file_exists(ms.lexfile.c_str())) {
    moot_msg(vlevel, vlProgress, "%s: loading lexical frequency file '%s'...", PROGNAME, ms.lexfile.c_str());
    if (!lexfreqs.load(ms.lexfile.c_str()))
      moot_croak("\n%s: load FAILED for lexical frequency file '%s'\n", PROGNAME, ms.lexfile.c_str());
    moot_msg(vlevel, vlProgress," loaded.\n");
  }

  // -- load model: n-gram frequencies
  if (!ms.ngfile.empty() && moot_file_exists(ms.ngfile.c_str())) {
    moot_msg(vlevel, vlProgress, "%s: loading n-gram frequency file '%s'...", PROGNAME, ms.ngfile.c_str());
    if (!ngrams.load(ms.ngfile.c_str()))
      moot_croak("\n%s: load FAILED for n-gram frequency file '%s'\n", PROGNAME, ms.ngfile.c_str());
    moot_msg(vlevel,vlProgress," loaded.\n");
  }

  // -- load model: class frequencies
  if (!ms.lcfile.empty() && moot_file_exists(ms.lcfile.c_str())) {
    moot_msg(vlevel, vlProgress, "%s: loading class frequency file '%s'...", PROGNAME, ms.lcfile.c_str());
    if (!classfreqs.load(ms.lcfile.c_str()))
      moot_croak("\n%s: load FAILED for class frequency file '%s'\n",PROGNAME, ms.lcfile.c_str());
    moot_msg(vlevel,vlProgress," loaded.\n");
  }
  else {
    //-- no class freqs
    hmm.use_lex_classes = false;
    if (!ms.lcfile.empty())
      moot_msg(vlevel, vlWarnings, "%s: Warning: no class frequency file '%s' -- skipping.\n", PROGNAME, ms.lcfile.c_str());
  }

  // -- load model: flavors
  if (hmm.use_flavors && !ms.flafile.empty() && moot_file_exists(ms.flafile.c_str())) {
    //-- user-defined flavor file
    moot_msg(vlevel, vlProgress, "%s: loading flavor definition file '%s'...", PROGNAME, ms.flafile.c_str());
    taster.clear();
    if (!taster.load(ms.flafile))
      moot_croak("\n%s: load FAILED for flavor definition file '%s'\n", PROGNAME, ms.flafile.c_str());
    moot_msg(vlevel,vlProgress," loaded.\n");
  }
  else if (hmm.use_flavors) {
    //-- built-in flavors
    moot_msg(vlevel, vlProgress, "%s: using built-in flavor definitions\n", PROGNAME);
    //taster.set_default_rules(); //-- default
  }
  else {
    //-- flavors disabled
    moot_msg(vlevel, vlProgress, "%s: disabling token flavors\n", PROGNAME);
    taster.clear();
  }

  return true;
}


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
 try {
  GetMyOptions(argc,argv);

  //-- the guts : load input model(s)
  for (unsigned i=0; i < args.inputs_num; i++) {
    if (!load_model(args.inputs[i]))
      moot_croak("%s: load FAILED for model `%s' -- aborting!\n", PROGNAME, args.inputs[i]);
  }

  //-- compile HMM: check lex classes
  if (classfreqs.lctable.size() <= 1 && hmm.use_lex_classes) {
    hmm.use_lex_classes = false;
    moot_msg(vlevel, vlWarnings, "%s: Warning: no class frequencies available: model won't use lexical classes!\n", PROGNAME);
  }

  moot_msg(vlevel, vlProgress, "%s: compiling HMM...", PROGNAME);

  //-- compile hmm: ensure taster-flavors are computed for lexfreqs
  lexfreqs.unknown_threshhold = hmm.unknown_lex_threshhold;
  lexfreqs.compute_specials((hmm.use_flavors ? &taster : NULL), true);

  //-- compile guts
  if (!hmm.compile(lexfreqs, ngrams, classfreqs, args.eos_tag_arg, taster))
    moot_croak("\n%s: HMM compilation FAILED\n", PROGNAME);

  moot_msg(vlevel, vlProgress, " compiled.\n");

  // -- parse n-gram smoothing constants (nlambdas)
  if (args.nlambdas_arg) {
    double nlambdas[3] = {0,1,0};
    if (!moot_parse_doubles(args.nlambdas_arg, nlambdas, 3))
      moot_croak("%s: could not parse N-Gram smoothing constants '%s'\n", PROGNAME, args.nlambdas_arg);

    hmm.nglambda1 = nlambdas[0];
    hmm.nglambda2 = nlambdas[1];
#ifdef moot_USE_TRIGRAMS
    hmm.nglambda3 = nlambdas[2];
#else
    if (nlambdas[2] != 0) {
      moot_msg(vlevel,vlWarnings, "%s: Warning: use of trigrams disabled.\n", PROGNAME);
    }
#endif
    if (nlambdas[0]+nlambdas[1]+nlambdas[2] != 1) {
      moot_msg(vlevel,vlWarnings, "%s: Warning: N-gram smoothing constants do not sum to one: %s\n", PROGNAME, args.nlambdas_arg);
    }
  } else {
    moot_msg(vlevel,vlProgress,"%s: estimating n-gram lambdas...", PROGNAME);
    if (!hmm.estimate_lambdas(ngrams))
      moot_croak("\n%s: n-gram lambda estimation FAILED.\n", PROGNAME);
    moot_msg(vlevel,vlProgress," done.\n");
  }

  // -- parse lexical smoothing constants (wlambdas)
  if (args.wlambdas_arg) {
    double wlambdas[2] = {1,0};
    if (!moot_parse_doubles(args.wlambdas_arg, wlambdas, 2))
      moot_croak("%s: could not parse lexical smoothing constants '%s'\n", PROGNAME, args.wlambdas_arg);

    hmm.wlambda0 = wlambdas[0];
    hmm.wlambda1 = wlambdas[1];
    if (hmm.wlambda0 + hmm.wlambda1 != 1)
      moot_msg(vlevel,vlWarnings,"%s: Warning: Lexical smoothing constants do not sum to one: %s\n", PROGNAME, args.wlambdas_arg);
  } else {
    moot_msg(vlevel,vlProgress, "%s: estimating lexical lambdas...", PROGNAME);
    if (!hmm.estimate_wlambdas(lexfreqs))
      moot_croak("\n%s: lexical lambda estimation FAILED.\n", PROGNAME);
    moot_msg(vlevel,vlProgress," done.\n");
  }

  //-- parse/estimate lexical-class smoothing constants (clambdas)
  if (hmm.use_lex_classes) {
    if (args.clambdas_arg) {
      double clambdas[2] = {1,0};
      if (!moot_parse_doubles(args.clambdas_arg, clambdas, 2))
	moot_croak("%s: could not parse lexical-class smoothing constants '%s'\n",PROGNAME, args.clambdas_arg);
      hmm.clambda0 = clambdas[0];
      hmm.clambda1 = clambdas[1];
      if (hmm.clambda0 + hmm.clambda1 != 1)
	moot_msg(vlevel,vlWarnings, "%s: Warning: Lexical-class smoothing constants do not sum to one: %s\n", PROGNAME, args.clambdas_arg);
    } else {
      moot_msg(vlevel,vlProgress,"%s: estimating class lambdas...", PROGNAME);
      if (!hmm.estimate_clambdas(classfreqs))
	moot_croak("\n%s: class lambda estimation FAILED.\n", PROGNAME);
      moot_msg(vlevel,vlProgress," done.\n");
    }
  }

  //-- assign beam-width
  hmm.beamwd = args.beam_width_arg;

#ifdef MOOT_ENABLE_SUFFIX_TRIE
  //-- build suffix trie
  if (args.trie_depth_arg > 0) {
    moot_msg(vlevel,vlProgress,"%s: Building suffix trie ", PROGNAME);
    if (!hmm.build_suffix_trie(lexfreqs, ngrams, vlevel>1))
      moot_croak("\n%s: suffix trie construction FAILED.\n", PROGNAME);
    moot_msg(vlevel,vlProgress,": built.\n");
  }
#else
  if (args.trie_depth_given || args.trie_threshhold_given || args.trie_theta_given) {
    moot_msg(vlevel,vlWarnings,"%s: Warning: suffix trie support disabled: ignoring trie-related options\n", PROGNAME);
  }
#endif

  //-- compute logprobs
  moot_msg(vlevel,vlProgress,"%s: computing log-probabilities...", PROGNAME);
  if (!hmm.compute_logprobs())
    moot_croak("\n%s: log-probability computation FAILED.\n", PROGNAME);
  moot_msg(vlevel,vlProgress," done.\n");

  //-- dump binary model
  moot_msg(vlevel,vlProgress,"%s: saving binary HMM `%s' ...", PROGNAME, out.name.c_str());
  if (!hmm.save(out.name.c_str(), args.compress_arg))
    moot_croak("\n%s: binary HMM dump FAILED \n", PROGNAME);
  moot_msg(vlevel,vlProgress," saved.\n");

  //-- summary
  char cmts[3] = "%%";
  if (vlevel >= vlInfo) {
    fprintf(stderr, "\n%s %s Summary:\n", cmts, PROGNAME);
    fprintf(stderr, "%s   Unknown Token     : %s\n", cmts, hmm.tokids.id2name(0).c_str());
    fprintf(stderr, "%s   Unknown Tag       : %s\n", cmts, hmm.tagids.id2name(0).c_str());
    fprintf(stderr, "%s   Border Tag        : %s\n", cmts, hmm.tagids.id2name(hmm.start_tagid).c_str());
    fprintf(stderr, "%s   N-Gram lambdas    : lambda1=%g, lambda2=%g", cmts, hmm.nglambda1, hmm.nglambda2);
#ifdef moot_USE_TRIGRAMS
    fprintf(stderr, " lambda3=%g", hmm.nglambda3);
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "%s   Hash n-grams?     : %s\n", cmts, (hmm.hash_ngrams ? "yes" : "no"));
    fprintf(stderr, "%s   Lex. Threshhold   : %g\n", cmts, hmm.unknown_lex_threshhold);
    fprintf(stderr, "%s   Lexical lambdas   : lambdaw0=%g, lambdaw1=%g\n", cmts, hmm.wlambda0, hmm.wlambda0);
    fprintf(stderr, "%s   Use classes?      : %s\n", cmts, hmm.use_lex_classes ? "yes" : "no");
    fprintf(stderr, "%s   Use flavors?      : %s\n", cmts, hmm.use_flavors ? "yes" : "no");
    fprintf(stderr, "%s   Num/Flavors       : %u\n", cmts, hmm.taster.size());
    fprintf(stderr, "%s   Class Threshhold  : %g\n", cmts, hmm.unknown_class_threshhold);
    fprintf(stderr, "%s   Class lambdas     : lambdac0=%g, lambdac1=%g\n", cmts, hmm.clambda0, hmm.clambda1);
    fprintf(stderr, "%s   Beam Width        : %g\n", cmts, hmm.beamwd);
  }
 }
 catch (exception &e) {
   fprintf(stderr, "%s: Exception: %s\n", PROGNAME, e.what());
   abort();
 }

  return 0;
}
