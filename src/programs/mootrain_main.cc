/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: moottrain_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + CHMM PoS tagger/disambiguator for DWDS project: trainer : main()
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

#include <mootNgrams.h>
#include <mootLexfreqs.h>
#include <mootClassfreqs.h>
#include <mootHMMTrainer.h>

#include <mootUtils.h>
#include "mootrain_cmdparser.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mootrain";

// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;

// -- files
cmdutil_file_info lfout;
cmdutil_file_info ngout;
cmdutil_file_info lcout;

// -- global classes/structs
mootHMMTrainer  hmmt;
mootLexfreqs   &lexfreqs = hmmt.lexfreqs;
mootNgrams     &ngrams   = hmmt.ngrams;
mootClassfreqs &lcfreqs  = hmmt.lcfreqs;

typedef enum {
  vlSilent=0,
  vlErrors=1,
  vlProgress=2,
  //vlTiming=3,
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

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = 0;

  // -- parse model spec
  string lexfile;
  string ngfile;
  string lcfile;
  if (args.output_given) {
    if (!hmm_parse_model_name_text(args.output_arg, lexfile, ngfile, lcfile)) {
      fprintf(stderr, "%s: could not parse output model specification '%s'\n",
	      PROGNAME, args.output_arg);
      exit(1);
    }
  } else if (args.inputs_num > 0) {
    string mymodel = unextend(args.inputs[0]);

    if (!hmm_parse_model_name_text(mymodel, lexfile, ngfile, lcfile)) {
      fprintf(stderr, "%s: could not get output model from corpus-name '%s'\n",
	      PROGNAME, args.inputs[0]);
      exit(1);
    }
  } else {
    fprintf(stderr, "%s: You must specify either a corpus or an output model!\n",
	    PROGNAME);
    exit(1);
  }

  // -- assign various flags
  if (args.lex_given || args.ngrams_given || args.classes_given) {
    hmmt.want_lexfreqs = args.lex_given;
    hmmt.want_ngrams = args.ngrams_given;
    hmmt.want_classfreqs = args.classes_given;
  } else {
    hmmt.want_lexfreqs = true;
    hmmt.want_ngrams = true;
    hmmt.want_classfreqs = true;
  }
  hmmt.eos_tag = args.eos_tag_arg;

  //-- sanity check(s)
  //(none)

  // -- open output files
  if (hmmt.want_lexfreqs) {
    lfout.name = strdup(lexfile.c_str());
    if (!lfout.open("w")) {
      fprintf(stderr, "%s: open failed for lexical frequency file '%s': %s\n",
	      PROGNAME, lfout.name, strerror(errno));
      exit(1);
    }
  }
  if (hmmt.want_ngrams) {
    ngout.name = strdup(ngfile.c_str());
    if (!ngout.open("w")) {
      fprintf(stderr, "%s: open failed for ngram frequency file '%s': %s\n",
	      PROGNAME, ngout.name, strerror(errno));
      exit(1);
    }
  }
  if (hmmt.want_classfreqs) {
    lcout.name = strdup(lcfile.c_str());
    if (!lcout.open("w")) {
      fprintf(stderr, "%s: open failed for class frequency file '%s': %s\n",
	      PROGNAME, lcout.name, strerror(errno));
      exit(1);
    }
  }

  //-- report
  if (args.verbose_arg >= vlProgress) {
    //fprintf(stderr, "%s: kmax               : %d\n", PROGNAME, hmmt.kmax);
    fprintf(stderr, "%s: EOS tag            : %s\n", PROGNAME, hmmt.eos_tag.c_str());
    fprintf(stderr, "%s: Lexical frequenies : %s\n", PROGNAME,
	    lfout.name ? lfout.name : "(null)");
    fprintf(stderr, "%s: Ngram frequencies  : %s\n", PROGNAME,
	    ngout.name ? ngout.name : "(null)");
    fprintf(stderr, "%s: Class frequencies  : %s\n", PROGNAME,
	    lcout.name ? lcout.name : "(null)");
  }
}
  


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  size_t nfiles = 0;
  GetMyOptions(argc,argv);

  //-- get comment-string
  char cmts[3] = "%%";

  //-- get time
  time_t now_time = time(NULL);
  tm     now_tm;
  localtime_r(&now_time, &now_tm);

  //-- initialize summary info
  if (hmmt.want_lexfreqs) {
    fprintf(lfout.file, "%s %s lexical frequency file generated on %s",
	    cmts, PROGNAME, asctime(&now_tm));
  }
  if (hmmt.want_ngrams) {
    fprintf(ngout.file, "%s %s ngram frequency file generated on %s",
	    cmts, PROGNAME, asctime(&now_tm));
  }
  if (hmmt.want_classfreqs) {
    fprintf(lcout.file, "%s %s class frequency file generated on %s",
	    cmts, PROGNAME, asctime(&now_tm));
  }

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    nfiles++;
    if (args.verbose_arg >= vlProgress) {
      fprintf(stderr,"%s: training from file '%s'...", PROGNAME, churner.in.name);
      fflush(stderr);
    }
    if (lfout.file)
      fprintf(lfout.file, "%s  Corpus     : %s\n", cmts, churner.in.name);
    if (ngout.file)
      fprintf(ngout.file, "%s  Corpus      : %s\n", cmts, churner.in.name);
    if (lcout.file)
      fprintf(lcout.file, "%s  Corpus        : %s\n", cmts, churner.in.name);

    hmmt.train_from_stream(churner.in.file, churner.in.name);
    
    if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," done.\n");
      fflush(stderr);
    }
  }


  //-- save: lexfreqs
  if (hmmt.want_lexfreqs) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: computing counts for special lexemes...", PROGNAME);

    hmmt.lexfreqs.compute_specials();

    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "done.\n");

    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: saving lexical frequency file '%s'...", PROGNAME, lfout.name);

    //-- print summary to file
    fprintf(lfout.file, "%s  Num/Tokens : %g\n", cmts, hmmt.lexfreqs.n_tokens);
    fprintf(lfout.file, "%s  Num/Types  : %u\n", cmts, hmmt.lexfreqs.lftable.size());
    fprintf(lfout.file, "%s  Num/Tags   : %u\n", cmts, hmmt.lexfreqs.tagtable.size());
    fprintf(lfout.file, "%s  Num/Pairs  : %u tok*tag\n", cmts, hmmt.lexfreqs.n_pairs());

    //-- lexfreqs: save: guts
    if (!hmmt.lexfreqs.save(lfout.file, lfout.name)) {
      fprintf(stderr, "\n%s: save FAILED for lexical frequency file '%s'\n",
	      PROGNAME, lfout.name);
      exit(2);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr, " saved.\n");
    }
    lfout.close();
  }


  // -- save: n-grams
  if (hmmt.want_ngrams) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: saving ngram frequency file '%s'...",
	      PROGNAME, ngout.name);

    //-- finish summary
    fprintf(ngout.file, "%s  EOS Tag     : %s\n", cmts, hmmt.eos_tag.c_str());
    //fprintf(ngout.file, "%s  kmax        : %u\n", cmts, hmmt.kmax);
    fprintf(ngout.file, "%s  Num/Tokens  : %g\n", cmts, hmmt.lexfreqs.n_tokens);
    fprintf(ngout.file, "%s  Num/1-grams : %u\n", cmts, hmmt.ngrams.ngtable.size());
    fprintf(ngout.file, "%s  Num/2-grams : %u\n", cmts, hmmt.ngrams.n_bigrams());
    fprintf(ngout.file, "%s  Num/3-grams : %u\n", cmts, hmmt.ngrams.n_trigrams());

    //-- guts
    if (!hmmt.ngrams.save(ngout.file, ngout.name, !args.verbose_ngrams_given)) {
      fprintf(stderr, "\n%s: save FAILED for n-gram frequency file '%s'\n",
	      PROGNAME, ngout.name);
      exit(2);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr, " saved.\n");
    }
    ngout.close();
  }


  //-- save: classfreqs
  if (hmmt.want_classfreqs) {
    if (args.verbose_arg >= vlProgress)
      fprintf(stderr, "%s: saving class frequency file '%s'...", PROGNAME, lcout.name);

    //-- print summary to file
    fprintf(lcout.file, "%s  Num/Tokens    : %g\n", cmts, hmmt.lcfreqs.totalcount);
    fprintf(lcout.file, "%s  Num/Classes   : %u\n", cmts, hmmt.lcfreqs.lctable.size());
    fprintf(lcout.file, "%s  Num/Tags      : %u\n", cmts, hmmt.lcfreqs.tagtable.size());
    fprintf(lcout.file, "%s  Num/Pairs     : %u class*tag\n", cmts, hmmt.lcfreqs.n_pairs());
    fprintf(lcout.file, "%s  - Impossibles : %u class*tag\n", cmts, hmmt.lcfreqs.n_impossible());

    //-- classfreqs: save: guts
    if (!hmmt.lcfreqs.save(lcout.file, lcout.name)) {
      fprintf(stderr, "\n%s: save FAILED for class frequency file '%s'\n",
	      PROGNAME, lcout.name);
      exit(2);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr, " saved.\n");
    }
    lcout.close();
  }


  return 0;
}
