/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2012 by Bryan Jurish <moocow@cpan.org>

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
 * File: moottrain_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
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
#include <mootTokenIO.h>
#include <mootTokenExpatIO.h>
#include <mootCIO.h>
#include <mootUtils.h>

#include "mootrain_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "mootrain";

// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;

// -- files
mofstream lfout;
mofstream ngout;
mofstream lcout;

// -- global classes/structs
mootHMMTrainer  hmmt;
mootLexfreqs   &lexfreqs = hmmt.lexfreqs;
mootNgrams     &ngrams   = hmmt.ngrams;
mootClassfreqs &lcfreqs  = hmmt.lcfreqs;

//-- token i/o
int ifmt         = tiofNone;
int ifmt_implied = tiofTagged;
int ifmt_default = tiofWellDone;

TokenReader *reader = NULL;

//-- verbosity level
typedef enum {
  vlSilent=0,
  vlErrors=1,
  vlProgress=2,
  //vlTiming=3,
  vlWarnings=4,
  vlEverything=5
} VerbosityLevel;
int vlevel = vlEverything;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- load environmental defaults
  cmdline_parser_envdefaults(&args);

  // -- verbosity
  vlevel = args.verbose_arg;

  // -- show banner
  moot_msg(vlevel, vlProgress,  moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = 0;
  churner.paranoid = true;

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
    string mymodel = moot_unextend(args.inputs[0]);

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
    if (!lfout.open(lexfile, "w")) {
      fprintf(stderr, "%s: open failed for lexical frequency file '%s': %s\n",
	      PROGNAME, lfout.name.c_str(), strerror(errno));
      exit(1);
    }
  }
  if (hmmt.want_ngrams) {
    if (!ngout.open(ngfile,"w")) {
      fprintf(stderr, "%s: open failed for ngram frequency file '%s': %s\n",
	      PROGNAME, ngout.name.c_str(), strerror(errno));
      exit(1);
    }
  }
  if (hmmt.want_classfreqs) {
    if (!lcout.open(lcfile,"w")) {
      fprintf(stderr, "%s: open failed for class frequency file '%s': %s\n",
	      PROGNAME, lcout.name.c_str(), strerror(errno));
      exit(1);
    }
  }

  //-- i/o format : input
  ifmt = TokenIO::parse_format_request(args.input_format_arg,
				       (args.inputs_num>0 ? args.inputs[0] : NULL),
				       ifmt_implied,
				       ifmt_default);

  //-- io: new_reader
  reader = TokenIO::new_reader(ifmt);
  if (!reader) {
    fprintf(stderr, "%s: Error: could not set up TokenReader!\n", PROGNAME);
    exit(1);
  }

#ifdef MOOT_EXPAT_ENABLED
  //-- io: encoding: reader
  if (ifmt&tiofXML && args.input_encoding_given) {
    reinterpret_cast<TokenReaderExpat *>(reader)->setEncoding(static_cast<std::string>(args.input_encoding_arg));
  }
#endif

  //-- report
  if (args.verbose_arg >= vlProgress) {
    //fprintf(stderr, "%s: kmax               : %d\n", PROGNAME, hmmt.kmax);
    fprintf(stderr, "%s: EOS tag            : %s\n", PROGNAME, hmmt.eos_tag.c_str());
    fprintf(stderr, "%s: Lexical frequenies : %s\n", PROGNAME,
	    !lfout.name.empty() ? lfout.name.c_str() : "(null)");
    fprintf(stderr, "%s: Ngram frequencies  : %s\n", PROGNAME,
	    !ngout.name.empty() ? ngout.name.c_str() : "(null)");
    fprintf(stderr, "%s: Class frequencies  : %s\n", PROGNAME,
	    !lcout.name.empty() ? lcout.name.c_str() : "(null)");
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
  /*
  tm     now_tm;
  localtime_r(&now_time, &now_tm);
  */
  tm *now_tm = localtime(&now_time);

  //-- initialize summary info
  if (hmmt.want_lexfreqs) {
    lfout.printf("%s %s lexical frequency file generated on %s",
		 cmts, PROGNAME, asctime(now_tm));
  }
  if (hmmt.want_ngrams) {
    ngout.printf("%s %s ngram frequency file generated on %s",
		 cmts, PROGNAME, asctime(now_tm));
  }
  if (hmmt.want_classfreqs) {
    lcout.printf("%s %s class frequency file generated on %s",
		 cmts, PROGNAME, asctime(now_tm));
  }

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    nfiles++;
    moot_msg(vlevel, vlProgress, "%s: training from file '%s'...", PROGNAME, churner.in.name.c_str());
    if (lfout.valid())
      lfout.printf("%s  Corpus     : %s\n", cmts, churner.in.name.c_str());
    if (ngout.valid())
      ngout.printf("%s  Corpus      : %s\n", cmts, churner.in.name.c_str());
    if (lcout.valid())
      lcout.printf("%s  Corpus        : %s\n", cmts, churner.in.name.c_str());

    reader->from_mstream(&churner.in);
    reader->reader_name(churner.in.name);
    hmmt.train_from_reader(reader);
    reader->close();

    moot_msg(vlevel, vlProgress, "done.\n");
  }

  //-- finish
  moot_msg(vlevel, vlProgress, "%s: finalizing model...", PROGNAME);
  hmmt.train_finish();
  moot_msg(vlevel, vlProgress, "done.\n");

  //-- save: lexfreqs
  if (hmmt.want_lexfreqs) {
    moot_msg(vlevel, vlProgress, "%s: saving lexical frequency file '%s'...", PROGNAME, lfout.name.c_str());

    //-- print summary to file
    if (lfout.valid()) {
      lfout.printf("%s  Num/Tokens : %g\n", cmts, hmmt.lexfreqs.n_tokens);
      lfout.printf("%s  Num/Types  : %u\n", cmts, hmmt.lexfreqs.lftable.size());
      lfout.printf("%s  Num/Tags   : %u\n", cmts, hmmt.lexfreqs.tagtable.size());
      lfout.printf("%s  Num/Pairs  : %u tok*tag\n", cmts, hmmt.lexfreqs.n_pairs());
    }

    //-- lexfreqs: save: guts
    if (!hmmt.lexfreqs.save(lfout.file, lfout.name.c_str())) {
      fprintf(stderr, "\n%s: save FAILED for lexical frequency file '%s'\n",
	      PROGNAME, lfout.name.c_str());
      exit(2);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr, " saved.\n");
    }
    lfout.close();
  }

  // -- save: n-grams
  if (hmmt.want_ngrams) {
    moot_msg(vlevel, vlProgress, "%s: saving ngram frequency file '%s'...", PROGNAME, ngout.name.c_str());

    //-- finish summary
    ngout.printf("%s  EOS Tag     : %s\n", cmts, hmmt.eos_tag.c_str());
    ngout.printf("%s  Num/Tokens  : %g\n", cmts, hmmt.lexfreqs.n_tokens);
    ngout.printf("%s  Num/1-grams : %u\n", cmts, hmmt.ngrams.ngtable.size());
    ngout.printf("%s  Num/2-grams : %u\n", cmts, hmmt.ngrams.n_bigrams());
    ngout.printf("%s  Num/3-grams : %u\n", cmts, hmmt.ngrams.n_trigrams());

    //-- guts
    if (!hmmt.ngrams.save(ngout.file, ngout.name.c_str(), !args.verbose_ngrams_given)) {
      fprintf(stderr, "\n%s: save FAILED for n-gram frequency file '%s'\n",
	      PROGNAME, ngout.name.c_str());
      exit(2);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr, " saved.\n");
    }
    ngout.close();
  }


  //-- save: classfreqs
  if (hmmt.want_classfreqs) {
    moot_msg(vlevel,vlProgress, "%s: saving class frequency file '%s'...", PROGNAME, lcout.name.c_str());

    //-- print summary to file
    lcout.printf("%s  Num/Tokens    : %g\n", cmts, hmmt.lcfreqs.totalcount);
    lcout.printf("%s  Num/Classes   : %u\n", cmts, hmmt.lcfreqs.lctable.size());
    lcout.printf("%s  Num/Tags      : %u\n", cmts, hmmt.lcfreqs.tagtable.size());
    lcout.printf("%s  Num/Pairs     : %u class*tag\n", cmts, hmmt.lcfreqs.n_pairs());
    lcout.printf("%s  - Impossibles : %u class*tag\n", cmts, hmmt.lcfreqs.n_impossible());

    //-- classfreqs: save: guts
    if (!hmmt.lcfreqs.save(lcout.file, lcout.name.c_str())) {
      fprintf(stderr, "\n%s: save FAILED for class frequency file '%s'\n",
	      PROGNAME, lcout.name.c_str());
      exit(2);
    } else if (args.verbose_arg >= vlProgress) {
      fprintf(stderr, " saved.\n");
    }
    lcout.close();
  }


  return 0;
}
