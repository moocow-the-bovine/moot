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
 * File: mootm_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Morphological analyzer for DWDS project : main()
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

#include <mootMorph.h>

#include "cmdutil.h"
#include "mootm_cmdparser.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mootm";

// options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

// -- filenames
char *symfile = NULL;
char *fstfile = NULL;

// -- files
cmdutil_file_info out;

// -- global classes/structs
mootMorph morph;

// -- for verbose timing info
timeval t1, t2, t3;
double elapsed_i, elapsed_t;

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

  //-- show banner
  if (args.verbose_arg >= vlProgress)
    fprintf(stderr,
	    "%s version %s by Bryan Jurish <moocow@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  //-- output file
  out.name = args.output_arg;
  if (strcmp(out.name,"-") == 0) out.name = "<stdout>";
  if (!out.open("w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name, strerror(errno));
    exit(1);
  }

  //-- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;

  //-- get initialization start-time
  if (args.verbose_arg >= vlTiming) gettimeofday(&t1, NULL);

  //-- mophology object setup: flags
  morph.want_avm           = args.avm_given;
  morph.want_mabbaw_format = args.mabbaw_given;
  morph.do_dequote         = args.dequote_given;

  if      (args.verbose_arg <= vlSilent)   morph.verbose = mootMorph::vlSilent;
  //else if (args.verbose_arg <= vlErrors)   morph.verbose = mootMorph::vlErrors;
  //else if (args.verbose_arg <= vlProgress) morph.verbose = mootMorph::vlErrors;
  else if (args.verbose_arg <= vlTiming)   morph.verbose = mootMorph::vlErrors;
  else if (args.verbose_arg <= vlWarnings) morph.verbose = mootMorph::vlWarnings;
  else                                     morph.verbose = mootMorph::vlEverything;

  //-- morphology object setup : symbols
  if (args.verbose_arg >= vlProgress)
    fprintf(stderr, "%s: loading morphological symbols-file '%s'...", PROGNAME, args.symbols_arg);
  if (!morph.load_morph_symbols(args.symbols_arg)) {
    fprintf(stderr,"\n%s: load FAILED for morphological symbols-file '%s'\n",
	    PROGNAME, args.symbols_arg);
    exit(1);
  } else if (args.verbose_arg >= vlProgress) {
    fprintf(stderr," loaded.\n");
  }

  //-- morphology object setup : morphology FST
  if (args.verbose_arg >= vlProgress)
    fprintf(stderr, "%s: loading morphological FST '%s'...", PROGNAME, args.morph_arg);
  if (!morph.load_morph_fst(args.morph_arg)) {
    fprintf(stderr,"\n%s: load FAILED for morphological FST '%s'\n", PROGNAME, args.morph_arg);
    exit(1);
  } else if (args.verbose_arg >= vlProgress) {
    fprintf(stderr," loaded.\n");
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

  //-- report
  if (args.verbose_arg >= vlProgress) {
    fprintf(stderr, "%s: Initialization complete.\n", PROGNAME);
  }
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  int nfiles = 0;

  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  if (args.verbose_arg >= vlTiming) gettimeofday(&t2, NULL);


  // -- the guts
  if (args.words_given) {
    //fprintf(out.file, "# %s: Analyzing command-line tokens\n\n", PROGNAME);
    morph.tag_strings(args.inputs_num, args.inputs, out.file, out.name);
  } else {
    // -- big loop
    for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
      nfiles++;
      if (args.verbose_arg >= vlProgress) {
	fprintf(stderr,"%s: analyzing file '%s'...", PROGNAME, churner.in.name);
	fflush(stderr);
      }
      fprintf(out.file, "\n%s %s: File: %s\n\n",
	      (args.mabbaw_given ? "#" : "%%"),
	      PROGNAME,
	      churner.in.name);

      morph.tag_stream(churner.in.file, out.file, churner.in.name);

      if (args.verbose_arg >= vlProgress) {
	fprintf(stderr," done.\n");
	fflush(stderr);
      }
    }
    out.close();
  }

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
      fprintf(stderr, "    - Tokens processed    : %d\n", morph.ntokens);
      fprintf(stderr, "    - Unknown tokens      : %d\n", morph.nunknown);
      if (morph.ntokens > 0) {
	// -- avoid div-by-zero errors
	fprintf(stderr, "    - Recognition Rate    : %.2f %%\n",
		100.0*(double)(morph.ntokens-morph.nunknown)/(double)morph.ntokens);
      } else {
	fprintf(stderr, "    - Recognition Rate    : -NaN-\n");
      }
      fprintf(stderr, "  + General\n");
      fprintf(stderr, "    - Files processed     : %d\n", nfiles);
      fprintf(stderr, "    - Initialize Time     : %.2f sec\n", elapsed_i);
      fprintf(stderr, "    - Analysis Time       : %.2f sec\n", elapsed_t);
      fprintf(stderr, "    - Throughput          : %.2f tok/sec\n", (float)morph.ntokens/elapsed_t);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}
