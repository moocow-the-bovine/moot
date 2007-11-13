/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2007 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootcompile_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
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
#include "mootcompile_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "mootcompile";

// options & file-churning
gengetopt_args_info  args;

// -- files
mofstream out;

// -- global classes/structs
mootHMM        hmm;
mootNgrams     ngrams;
mootLexfreqs   lexfreqs(32768);
mootClassfreqs classfreqs(512);

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  cmdline_parser_envdefaults(&args);

  //-- sanity check
  if (args.inputs_num <= 0) {
    fprintf(stderr, "%s: cowardly refusing to compile empty model!\n", PROGNAME);
    exit(1);
  }

  //-- show banner
  if (args.verbose_arg > 1)
    fprintf(stderr,
	    moot_program_banner(PROGNAME,
				PACKAGE_VERSION,
				"Bryan Jurish <jurish@ling.uni-potsdam.de>").c_str());

  //-- output file
  if (!out.open(args.output_arg,"wb")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name.c_str(), strerror(errno));
    exit(1);
  }
  out.close(); //-- close again: HMM will write it itself

  // -- assign "unknown" ids & other flags
  hmm.use_lex_classes = args.use_classes_arg;
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
bool load_model(char *modelname)
{
  string lexfile;
  string ngfile;
  string classfile;

  if (!hmm_parse_model_name_text(modelname, lexfile, ngfile, classfile)) {
    fprintf(stderr, "%s: could not parse model specification '%s'\n",
	    PROGNAME, modelname);
    exit(1);
  }

  // -- load model: lexical frequencies
  if (!lexfile.empty() && moot_file_exists(lexfile.c_str())) {
    if (args.verbose_arg > 1) {
      fprintf(stderr, "%s: loading lexical frequency file '%s'...",
	      PROGNAME, lexfile.c_str());
    }
    if (!lexfreqs.load(lexfile.c_str())) {
      fprintf(stderr,"\n%s: load FAILED for lexical frequency file '%s'\n",
	      PROGNAME, lexfile.c_str());
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," loaded.\n");
    }
  }

  // -- load model: n-gram frequencies
  if (!ngfile.empty() && moot_file_exists(ngfile.c_str())) {
    if (args.verbose_arg > 1) {
      fprintf(stderr, "%s: loading n-gram frequency file '%s'...",
	      PROGNAME, ngfile.c_str());
    }
    if (!ngrams.load(ngfile.c_str())) {
      fprintf(stderr,"\n%s: load FAILED for n-gram frequency file '%s'\n",
	      PROGNAME, ngfile.c_str());
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," loaded.\n");
    }
  }

  // -- load model: class frequencies
  if (!classfile.empty() && moot_file_exists(classfile.c_str())) {
    if (args.verbose_arg > 1) {
      fprintf(stderr, "%s: loading class frequency file '%s'...",
	      PROGNAME, classfile.c_str());
    }
    if (!classfreqs.load(classfile.c_str())) {
      fprintf(stderr,"\n%s: load FAILED for class frequency file '%s'\n",
	      PROGNAME, classfile.c_str());
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," loaded.\n");
    }
  }
  else {
    hmm.use_lex_classes = false;
    if (!classfile.empty() && args.verbose_arg > 1)
      fprintf(stderr, "%s: no class frequency file '%s' -- skipping.\n",
	      PROGNAME, classfile.c_str());
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
    if (!load_model(args.inputs[i])) {
      fprintf(stderr, "%s: load FAILED for model `%s' -- aborting!\n",
	      PROGNAME, args.inputs[i]);
      exit(1);
    }
  }

  //-- compile HMM
  if (classfreqs.lctable.size() <= 1 && hmm.use_lex_classes) {
    hmm.use_lex_classes = false;
    if (args.verbose_arg > 1)
      fprintf(stderr,
	      "%s: no class frequencies available: model won't use lexical classes!\n",
	      PROGNAME);
  }

  if (args.verbose_arg > 1)
    fprintf(stderr, "%s: compiling HMM...", PROGNAME);
    
  lexfreqs.compute_specials();

  if (!hmm.compile(lexfreqs, ngrams, classfreqs, args.eos_tag_arg)) {
    fprintf(stderr,"\n%s: HMM compilation FAILED\n", PROGNAME);
    exit(1);
  }
  else if (args.verbose_arg > 1) {
    fprintf(stderr," compiled.\n");
  }

  // -- parse n-gram smoothing constants (nlambdas)
  if (args.nlambdas_arg) {
    double nlambdas[3] = {0,1,0};
    if (!moot_parse_doubles(args.nlambdas_arg, nlambdas, 3)) {
      fprintf(stderr, "%s: could not parse N-Gram smoothing constants '%s'\n",
	      PROGNAME, args.nlambdas_arg);
      exit(1);
    }
    hmm.nglambda1 = nlambdas[0];
    hmm.nglambda2 = nlambdas[1];
#ifdef moot_USE_TRIGRAMS
    hmm.nglambda3 = nlambdas[2];
#else
    if (nlambdas[2] != 0) {
      fprintf(stderr, "%s: Warning: use of trigrams disabled.\n", PROGNAME);
    }
#endif
    if (nlambdas[0]+nlambdas[1]+nlambdas[2] != 1) {
      fprintf(stderr, "%s: Warning: N-gram smoothing constants do not sum to one: %s\n",
	      PROGNAME, args.nlambdas_arg);
    }
  } else {
    if (args.verbose_arg > 1)
      fprintf(stderr, "%s: estimating n-gram lambdas...", PROGNAME);
    if (!hmm.estimate_lambdas(ngrams)) {
      fprintf(stderr,"\n%s: n-gram lambda estimation FAILED.\n", PROGNAME);
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," done.\n");
    }
  }

  // -- parse lexical smoothing constants (wlambdas)
  if (args.wlambdas_arg) {
    double wlambdas[2] = {1,0};
    if (!moot_parse_doubles(args.wlambdas_arg, wlambdas, 2)) {
      fprintf(stderr, "%s: could not parse lexical smoothing constants '%s'\n",
	      PROGNAME, args.wlambdas_arg);
      exit(1);
    }
    hmm.wlambda0 = wlambdas[0];
    hmm.wlambda1 = wlambdas[1];
    if (hmm.wlambda0 + hmm.wlambda1 != 1) {
      fprintf(stderr, "%s: Warning: Lexical smoothing constants do not sum to one: %s\n",
	      PROGNAME, args.wlambdas_arg);
    }
  } else {
    if (args.verbose_arg > 1)
      fprintf(stderr, "%s: estimating lexical lambdas...", PROGNAME);
    if (!hmm.estimate_wlambdas(lexfreqs)) {
      fprintf(stderr,"\n%s: lexical lambda estimation FAILED.\n", PROGNAME);
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," done.\n");
    }
  }

  //-- parse/estimate lexical-class smoothing constants (clambdas)
  if (hmm.use_lex_classes) {
    if (args.clambdas_arg) {
      double clambdas[2] = {1,0};
      if (!moot_parse_doubles(args.clambdas_arg, clambdas, 2)) {
	fprintf(stderr, "%s: could not parse lexical-class smoothing constants '%s'\n",
		PROGNAME, args.clambdas_arg);
	exit(1);
      }
      hmm.clambda0 = clambdas[0];
      hmm.clambda1 = clambdas[1];
      if (hmm.clambda0 + hmm.clambda1 != 1) {
	fprintf(stderr, "%s: Warning: Lexical-class smoothing constants do not sum to one: %s\n",
		PROGNAME, args.clambdas_arg);
      }
    } else {
      if (args.verbose_arg > 1)
	fprintf(stderr, "%s: estimating class lambdas...", PROGNAME);
      if (!hmm.estimate_clambdas(classfreqs)) {
	fprintf(stderr,"\n%s: class lambda estimation FAILED.\n", PROGNAME);
	exit(1);
      } else if (args.verbose_arg > 1) {
	fprintf(stderr," done.\n");
      }
    }
  }

  //-- assign beam-width
  hmm.beamwd = args.beam_width_arg;

#ifdef MOOT_SUFFIX_TRIE_ENABLED
  //-- build suffix trie
  if (args.trie_depth_arg > 0) {
    if (args.verbose_arg > 1) fprintf(stderr, "%s: Building suffix trie ", PROGNAME);
    if (!hmm.build_suffix_trie(lexfreqs, ngrams, args.verbose_arg>1)) {
      fprintf(stderr,"\n%s: suffix trie construction FAILED.\n", PROGNAME);
      exit(1);
    }
    else if (args.verbose_arg > 1) {
      fprintf(stderr, ": built.\n");
    }
  }
#else
  if (args.trie_depth_given || args.trie_threshhold_given || args.trie_theta_given) {
    fprintf(stderr, "%s: suffix trie support disabled: ignoring trie-related options\n", PROGNAME);
  }
#endif

  //-- compute logprobs
  if (args.verbose_arg > 1)
    fprintf(stderr, "%s: computing log-probabilities...", PROGNAME);
  if (!hmm.compute_logprobs()) {
    fprintf(stderr,"\n%s: log-probability computation FAILED.\n", PROGNAME);
    exit(1);
  } else if (args.verbose_arg > 1) {
    fprintf(stderr," done.\n");
  }


  //-- dump binary model
  if (args.verbose_arg > 1)
    fprintf(stderr, "%s: saving binary HMM model `%s' ...",
	    PROGNAME, out.name.c_str());
  if (!hmm.save(out.name.c_str(), args.compress_arg)) {
    fprintf(stderr,"\n%s: binary HMM dump FAILED \n", PROGNAME);
    exit(1);
  } else if (args.verbose_arg > 1) {
    fprintf(stderr," saved.\n");
  }

  //-- summary
  char cmts[3] = "%%";
  if (args.verbose_arg > 2) {
    fprintf(stderr, "\n%s %s Summary:\n", cmts, PROGNAME);
    fprintf(stderr, "%s   Unknown Token     : %s\n", cmts, hmm.tokids.id2name(0).c_str());
    fprintf(stderr, "%s   Unknown Tag       : %s\n", cmts, hmm.tagids.id2name(0).c_str());
    fprintf(stderr, "%s   Border Tag        : %s\n", cmts, hmm.tagids.id2name(hmm.start_tagid).c_str());
    fprintf(stderr, "%s   N-Gram lambdas    : lambda1=%g, lambda2=%g",
	    cmts, hmm.nglambda1, hmm.nglambda2);
#ifdef moot_USE_TRIGRAMS
    fprintf(stderr, " lambda3=%g", hmm.nglambda3);
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "%s   Lex. Threshhold   : %g\n", cmts, hmm.unknown_lex_threshhold);
    fprintf(stderr, "%s   Lexical lambdas   : lambdaw0=%g, lambdaw1=%g\n",
	    cmts, hmm.wlambda0, hmm.wlambda0);
    fprintf(stderr, "%s   Use classes?      : %s\n",
	    cmts, hmm.use_lex_classes ? "yes" : "no");
    fprintf(stderr, "%s   Class Threshhold  : %g\n", cmts, hmm.unknown_class_threshhold);
    fprintf(stderr, "%s   Class lambdas     : lambdac0=%g, lambdac1=%g\n",
	    cmts, hmm.clambda0, hmm.clambda1);
    fprintf(stderr, "%s   Beam Width        : %g\n",
	    cmts, hmm.beamwd);
  }
 }
 catch (exception &e) {
   fprintf(stderr, "%s: Exception: %s\n", PROGNAME, e.what());
   abort();
 }

  return 0;
}
