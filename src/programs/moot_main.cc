/*
   moot-utils version 1.0.4 : moocow's part-of-speech tagger
   Copyright (C) 2002-2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: moot_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + CHMM PoS tagger/disambiguator for DWDS project : main()
 *--------------------------------------------------------------------------*/

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

#include <string>

#include <mootMorph.h>
#include <mootCHMM.h>
#include <mootLexfreqs.h>
#include <mootNgrams.h>

#include "cmdutil.h"
#include "hmmutil.h"
#include "moot_cmdparser.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "moot";

// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;

// -- files
cmdutil_file_info out;

// -- global classes/structs
mootCHMM     hmm;
mootMorph   &morph = hmm.morph;
mootNgrams   ngrams;
mootLexfreqs lexfreqs;

// -- for verbose timing info
timeval t1, t2, t3;
double  elapsed_i, elapsed_t;

typedef enum {
  vlSilent=0,
  vlErrors=1,
  vlProgress=2,
  vlTiming=3,
  vlWarnings=4,
  vlEverything=5
} VerbosityLevel;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- load environmental defaults
  cmdline_parser_envdefaults(&args);

  // -- show banner
  if (args.verbose_arg >= vlProgress)
    fprintf(stderr,
	    "%s version %s by Bryan Jurish <moocow@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  // -- output file
  out.name = args.output_arg;
  if (strcmp(out.name,"-") == 0) out.name = "<stdout>";
  if (!out.open("w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name, strerror(errno));
    exit(1);
  }

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;

  // -- get initialization start-time
  if (args.verbose_arg >= vlTiming) gettimeofday(&t1, NULL);

  // -- assign "unknown" ids & other flags
  hmm.unknown_token_name(args.unknown_token_arg);
  hmm.unknown_tag_name(args.unknown_tag_arg);
  hmm.unknown_lex_threshhold = args.unknown_threshhold_arg;
  hmm.morph_cache_threshhold = args.morph_cache_threshhold_arg;
  hmm.want_pos_only = args.pos_only_given;

  // -- parse model spec
  char *binfile=NULL;
  char *lexfile=NULL;
  char *ngfile=NULL;
  if (!hmm_parse_model(args.model_arg, &binfile, &lexfile, &ngfile)) {
    fprintf(stderr, "%s: could not parse model specification '%s'\n",
	    PROGNAME, args.model_arg);
    exit(1);
  }

  // -- load model: binary
  if (binfile) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: loading binary HMM model file '%s'...", PROGNAME, binfile);
    if (!hmm.load(binfile)) {
      fprintf(stderr,"\n%s: load FAILED for binary HMM model file '%s'\n",
	      PROGNAME, binfile);
      exit(1);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," loaded.\n");
    }
  }

  // -- load model: lexical frequencies
  if (lexfile) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: loading lexical frequency file '%s'...", PROGNAME, lexfile);
    if (!lexfreqs.load(lexfile)) {
      fprintf(stderr,"\n%s: load FAILED for lexical frequency file '%s'\n",
	      PROGNAME, lexfile);
      exit(1);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," loaded.\n");
    }
  }

  // -- load model: n-gram frequencies
  if (ngfile) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: loading n-gram frequency file '%s'...", PROGNAME, ngfile);
    if (!ngrams.load(ngfile)) {
      fprintf(stderr,"\n%s: load FAILED for n-gram frequency file '%s'\n",
	      PROGNAME, ngfile);
      exit(1);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," loaded.\n");
    }
  }

  // -- morphology setup: compile-only flags
  if (!binfile) {
    morph.want_avm           = args.avm_given;
    morph.do_dequote         = args.dequote_given;
  }

  // -- morphology setup: common flags
  morph.want_mabbaw_format = args.mabbaw_given;

  if      (args.verbose_arg <= vlSilent)   morph.verbose = mootMorph::vlSilent;
  else if (args.verbose_arg <= vlErrors)   morph.verbose = mootMorph::vlErrors;
  else if (args.verbose_arg <= vlProgress) morph.verbose = mootMorph::vlErrors;
  else if (args.verbose_arg <= vlTiming)   morph.verbose = mootMorph::vlErrors;
  else if (args.verbose_arg <= vlWarnings) morph.verbose = mootMorph::vlWarnings;
  else                                     morph.verbose = mootMorph::vlEverything;


  //-- morphology object setup : symbols
  if (args.symbols_given || !binfile) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: loading morphological symbols-file '%s'...", PROGNAME, args.symbols_arg);
    if (!morph.load_morph_symbols(args.symbols_arg)) {
      fprintf(stderr,"\n%s: load FAILED for morphological symbols-file '%s'\n",
	      PROGNAME, args.symbols_arg);
      exit(1);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," loaded.\n");
    }
  }

  //-- morphology object setup : morphology FST
  if (args.morph_given || !binfile) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: loading morphological FST '%s'...", PROGNAME, args.morph_arg);
    if (!morph.load_morph_fst(args.morph_arg)) {
      fprintf(stderr,"\n%s: load FAILED for morphological FST '%s'\n", PROGNAME, args.morph_arg);
      exit(1);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," loaded.\n");
    }
  }

  //-- morphology object setup : tag-extraction FST
  if (args.tagx_given) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: loading tag-extracion FST '%s'...", PROGNAME, args.tagx_arg);
    if (!morph.load_tagx_fst(args.tagx_arg)) {
      fprintf(stderr,"\n%s: load FAILED for morphological FST '%s'\n", PROGNAME, args.tagx_arg);
      exit(1);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," loaded.\n");
    }
  }


  // -- compile HMM
  if (!binfile) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: compiling HMM...", PROGNAME);
    if (!hmm.compile(lexfreqs, ngrams, args.eos_tag_arg))
      {
	fprintf(stderr,"\n%s: HMM compilation FAILED\n", PROGNAME);
	exit(1);
      }
    else if (args.verbose_arg >= vlProgress)
      {
	fprintf(stderr," compiled.\n");
      }

    // -- parse n-gram smoothing constants (nlambdas)
    if (args.nlambdas_arg) {
      double nlambdas[3] = {0,1,0};
      if (!hmm_parse_doubles(args.nlambdas_arg, nlambdas, 3)) {
	fprintf(stderr, "%s: could not parse N-Gram smoothing constants '%s'\n",
		PROGNAME, args.nlambdas_arg);
	exit(1);
      }
      hmm.nglambda1 = nlambdas[0];
      hmm.nglambda2 = nlambdas[1];
#ifdef moot_USE_TRIGRAMS
      hmm.nglambda3 = nlambdas[2];
#endif
    } else {
      if (args.verbose_arg >= vlProgress)
	fprintf(stderr, "%s: estimating n-gram lambdas...", PROGNAME);
      if (!hmm.estimate_lambdas(ngrams)) {
	fprintf(stderr,"\n%s: n-gram lambda estimation FAILED.\n", PROGNAME);
	exit(1);
      } else if (args.verbose_arg >= vlProgress) {
	fprintf(stderr," done.\n");
      }
    }

    // -- parse lexical smoothing constants (wlambdas)
    if (args.wlambdas_arg) {
      double wlambdas[2] = {1,0};
      if (!hmm_parse_doubles(args.wlambdas_arg, wlambdas, 2)) {
	fprintf(stderr, "%s: could not parse lexical smoothing constants '%s'\n",
		PROGNAME, args.wlambdas_arg);
	exit(1);
      }
      hmm.wlambda1 = wlambdas[0];
      hmm.wlambda2 = wlambdas[1];
    } else {
      if (args.verbose_arg >= vlProgress)
	fprintf(stderr, "%s: estimating lexical lambdas...", PROGNAME);
      if (!hmm.estimate_wlambdas(lexfreqs)) {
	fprintf(stderr,"\n%s: lexical lambda estimation FAILED.\n", PROGNAME);
	exit(1);
      } else if (args.verbose_arg >= vlProgress) {
	fprintf(stderr," done.\n");
      }
    }

    if (args.compile_given) {
      if (args.verbose_arg >= vlProgress)
	fprintf(stderr, "%s: saving binary HMM model '%s' ...", PROGNAME, args.compile_arg);
      if (!hmm.save(args.compile_arg, args.compress_arg)) {
	fprintf(stderr,"\n%s: binary HMM dump FAILED\n", PROGNAME);
	exit(1);
      } else if (args.verbose_arg >= vlProgress) {
	fprintf(stderr," saved.\n");
      }
      exit(0);
    }
  }
    
  // -- report
  if (args.verbose_arg >= vlProgress) {
    fprintf(stderr, "%s: Initialization complete\n", PROGNAME);
  }

  //-- dump if requested
  if (args.dump_given) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: dumping HMM debugging output to '%s' ...", PROGNAME, out.name);

    hmm.txtdump(out.file);

    if (args.verbose_arg >= vlProgress)
      fprintf(stderr," dumped.\n");

    exit(0);
  }


  //-- get comment-string
  char cmts[3] = "% ";
  if (args.mabbaw_given) cmts[0] = '#';
  cmts[1] = cmts[0];

  //-- get time
  time_t now_time = time(NULL);
  tm     now_tm;
  localtime_r(&now_time, &now_tm);

  //-- report to output-file
  fprintf(out.file, "%s %s output file generated on %s", cmts, PROGNAME, asctime(&now_tm));
  fprintf(out.file, "%s Configuration:\n", cmts);
  fprintf(out.file, "%s   Symbols           : %s\n", cmts, morph.syms_filename.c_str());
  fprintf(out.file, "%s   Morphology FST    : %s\n", cmts, morph.mfst_filename.c_str());
  fprintf(out.file, "%s   Tag-Extractor     : %s\n", cmts,
	  morph.xfst ? morph.xfst_filename.c_str() : "(none)");
  fprintf(out.file, "%s   ULex Threshhold   : %g\n", cmts, hmm.unknown_lex_threshhold);
  fprintf(out.file, "%s   MCache Threshhold : %g\n", cmts, hmm.morph_cache_threshhold);
  fprintf(out.file, "%s   MCache Size       : %u\n", cmts, hmm.morphcache.size());
  fprintf(out.file, "%s   Unknown Token     : %s\n", cmts, hmm.tokids.id2name(0).c_str());
  fprintf(out.file, "%s   Unknown Tag       : %s\n", cmts, hmm.tagids.id2name(0).c_str());
  fprintf(out.file, "%s   Boundary Tag      : %s\n", cmts, hmm.tagids.id2name(hmm.start_tagid).c_str());
  fprintf(out.file, "%s   N-Gram lambdas    : lambda1=%g, lambda2=%g",
	  cmts, hmm.nglambda1, hmm.nglambda2);
#ifdef moot_USE_TRIGRAMS
  fprintf(out.file, " lambda3=%g", hmm.nglambda3);
#endif
  fprintf(out.file, "\n");
  fprintf(out.file, "%s   Lexical lambdas  : lambdaw1=%g, lambdaw2=%g\n",
	  cmts, hmm.wlambda1, hmm.wlambda2);
}
  


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  size_t nfiles = 0;

  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  if (args.verbose_arg >= vlTiming) gettimeofday(&t2, NULL);

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    nfiles++;
    if (args.verbose_arg >= vlProgress) {
      fprintf(stderr,"%s: analyzing file '%s'...", PROGNAME, churner.in.name);
      fflush(stderr);
    }
    fprintf(out.file, "\n%%%% File: %s\n\n", churner.in.name);

    hmm.tag_stream(churner.in.file, out.file, churner.in.name);
    
    if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," done.\n");
      fflush(stderr);
    }
  }
  out.close();

  // -- summary
  if (args.verbose_arg >= vlTiming) {
      // -- timing
      gettimeofday(&t3, NULL);

      elapsed_i = (t2.tv_sec-t1.tv_sec) + (double)(t2.tv_usec-t1.tv_usec)/1000000.0;
      elapsed_t = (t3.tv_sec-t2.tv_sec) + (double)(t3.tv_usec-t2.tv_usec)/1000000.0;

      // -- print summary
      fprintf(stderr, "\n-----------------------------------------------------\n");
      fprintf(stderr, "%s Summary:\n", PROGNAME);
      fprintf(stderr, "  + Morphology\n");
      fprintf(stderr, "    - Tokens analyzed     : %u tok\n", morph.ntokens);
      fprintf(stderr, "    - Unknown tokens      : %u tok\n", morph.nunknown);
      if (morph.ntokens != 0) {
	// -- avoid div-by-zero errors
	fprintf(stderr, "    - Recognition Rate    : %.2f %%\n",
		100.0*(double)(hmm.ntokens-morph.nunknown)/(double)hmm.ntokens);
      } else {
	fprintf(stderr, "    - Recognition Rate    : -NaN-\n");
      }
      fprintf(stderr, "    - MorphCache Lookups  : %u tok\n", hmm.ntokens-hmm.morph.ntokens);
      if (hmm.ntokens != 0) {
	fprintf(stderr, "    - MorphCache Rate     : %.2f %%\n",
		100.0*(hmm.ntokens-morph.ntokens)/(float)hmm.ntokens);
      } else {
	fprintf(stderr, "    - MorphCache Rate     : -NaN-\n");
      }
      fprintf(stderr, "  + General\n");
      fprintf(stderr, "    - Files Processed     : %u files\n", nfiles);
      fprintf(stderr, "    - Tokens Processed    : %u tok\n", hmm.ntokens);
      fprintf(stderr, "    - Initialize Time     : %.3f sec\n", elapsed_i);
      fprintf(stderr, "    - Analysis Time       : %.3f sec\n", elapsed_t);
      fprintf(stderr, "    - Throughput Rate     : %.2f tok/sec\n", (double)hmm.ntokens/elapsed_t);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}

