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
#include <mootModelSpec.h>

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
mofstream flout;

// -- global classes/structs
mootHMMTrainer  hmmt;
mootModelSpec   ms;
mootTaster     &taster   = hmmt.taster;
mootLexfreqs   &lexfreqs = hmmt.lexfreqs;
mootNgrams     &ngrams   = hmmt.ngrams;
mootClassfreqs &lcfreqs  = hmmt.lcfreqs;

const char *flavor_src = "???";

//-- token i/o
int ifmt         = tiofNone;
int ifmt_implied = tiofTagged;
int ifmt_default = tiofWellDone;

TokenReader *reader = NULL;

//-- verbosity level
int vlevel = vlEverything;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- locale
  moot_setlocale();

  // -- load environmental defaults
  cmdline_parser_envdefaults(&args);

  // -- verbosity
  vlevel = args.verbose_arg;

  // -- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo,  moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = 0;
  churner.paranoid = true;

  // -- parse model spec
  if (args.output_given) {
    if (!ms.parse(args.output_arg, false)) 
      moot_croak("%s: could not parse output model specification '%s'\n", PROGNAME, args.output_arg);
  }
  else if (args.inputs_num > 0) {
    string mymodel = moot_unextend(args.inputs[0]);
    if (!ms.parse(mymodel, false))
      moot_croak("%s: could not get output model from corpus-name '%s'\n", PROGNAME, args.inputs[0]);
  }
  else {
    moot_croak("%s: you must specify either a corpus or an output model!\n", PROGNAME);
  }

  // -- assign various flags
  if (args.lex_given || args.ngrams_given || args.classes_given || args.flavors_given) {
    hmmt.want_lexfreqs = args.lex_given;
    hmmt.want_ngrams = args.ngrams_given;
    hmmt.want_classfreqs = args.classes_given;
    hmmt.want_flavors = args.flavors_given;
  } else {
    hmmt.want_lexfreqs = true;
    hmmt.want_ngrams = true;
    hmmt.want_classfreqs = true;
    hmmt.want_flavors = true;
  }
  hmmt.lexfreqs.unknown_threshhold = args.unknown_threshhold_arg;
  hmmt.eos_tag = args.eos_tag_arg;

  //-- sanity check(s)
  //(none)

  // -- open output files
  if (hmmt.want_lexfreqs && !lfout.open(ms.lexfile, "w"))
    moot_croak("%s: open failed for lexical frequency file '%s': %s\n", PROGNAME, lfout.name.c_str(), strerror(errno));
  if (hmmt.want_ngrams && !ngout.open(ms.ngfile,"w"))
    moot_croak("%s: open failed for ngram frequency file '%s': %s\n", PROGNAME, ngout.name.c_str(), strerror(errno));
  if (hmmt.want_classfreqs && !lcout.open(ms.lcfile,"w"))
    moot_croak("%s: open failed for class frequency file '%s': %s\n", PROGNAME, lcout.name.c_str(), strerror(errno));
  if (hmmt.want_flavors && !flout.open(ms.flafile,"w"))
    moot_croak("%s: open failed for output flavor file '%s': %s\n", PROGNAME, flout.name.c_str(), strerror(errno));

  //-- i/o format : input
  ifmt = TokenIO::parse_format_request(args.input_format_arg,
				       (args.inputs_num>0 ? args.inputs[0] : NULL),
				       ifmt_implied,
				       ifmt_default);

  //-- io: new_reader
  reader = TokenIO::new_reader(ifmt);
  if (!reader)
    moot_croak("%s: Error: could not set up TokenReader!\n", PROGNAME);

#ifdef MOOT_EXPAT_ENABLED
  //-- io: encoding: reader
  if (ifmt&tiofXML && args.input_encoding_given) {
    reinterpret_cast<TokenReaderExpat *>(reader)->setEncoding(static_cast<std::string>(args.input_encoding_arg));
  }
#endif

  //-- flavors
  if (!args.flavors_from_given) {
    //-- flavor file unspecified: use backwards-compatible built-in rules
    //taster.set_default_rules();  //-- default anyway
    flavor_src = "(built-in)";
  }
  else if (args.flavors_from_arg[0]) {
    //-- flavor file specified and non-empty: load file
    mifstream tin;
    if (!tin.open(args.flavors_from_arg,"r"))
      moot_croak("%s: open failed for input flavor file '%s': %s\n", PROGNAME, tin.name.c_str(), strerror(errno));
    taster.clear();
    taster.load(&tin);
    flavor_src = args.flavors_from_arg;
  }
  else {
    //-- flavor file specified as empty string: no flavors at all
    taster.clear();
    flavor_src = "(none)";
  }

  //-- report
  if (args.verbose_arg >= vlInfo) {
    //fprintf(stderr, "%s: kmax               : %d\n", PROGNAME, hmmt.kmax);
    fprintf(stderr, "%s: EOS tag            : %s\n", PROGNAME, hmmt.eos_tag.c_str());
    fprintf(stderr, "%s: Flavors (in)       : %s : %u rules\n", PROGNAME, flavor_src, taster.size());
    fprintf(stderr, "%s: Lexical frequenies : %s\n", PROGNAME, !lfout.name.empty() ? lfout.name.c_str() : "(null)");
    fprintf(stderr, "%s: Ngram frequencies  : %s\n", PROGNAME, !ngout.name.empty() ? ngout.name.c_str() : "(null)");
    fprintf(stderr, "%s: Class frequencies  : %s\n", PROGNAME, !lcout.name.empty() ? lcout.name.c_str() : "(null)");
    fprintf(stderr, "%s: Flavors (out)      : %s\n", PROGNAME, !flout.name.empty() ? flout.name.c_str() : "(null)");
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
  if (hmmt.want_lexfreqs)
    lfout.printf("%s %s lexical frequency file generated on %s", cmts, PROGNAME, asctime(now_tm));
  if (hmmt.want_ngrams)
    ngout.printf("%s %s ngram frequency file generated on %s", cmts, PROGNAME, asctime(now_tm));
  if (hmmt.want_classfreqs)
    lcout.printf("%s %s class frequency file generated on %s", cmts, PROGNAME, asctime(now_tm));
  if (hmmt.want_flavors)
    flout.printf("%s %s flavor rule file generated on %s", cmts, PROGNAME, asctime(now_tm));

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
      lfout.printf("%s  UnknownMaxF: %g\n", cmts, hmmt.lexfreqs.unknown_threshhold);
      lfout.printf("%s  LC_CTYPE   : %s\n", cmts, moot_lc_ctype());
      if (hmmt.want_flavors) {
	lfout.printf("%s  Flavors    : %s [%u rules]\n", cmts, flavor_src, taster.size());
	if (vlevel >= vlDebug) 
	  taster.save(&lfout, string(cmts)+"\t");
      } else {
	lfout.printf("%s  Flavors    : (none)\n", cmts);
      }
    }

    //-- lexfreqs: save: guts
    if (!hmmt.lexfreqs.save(lfout.file, lfout.name.c_str())) {
      moot_croak("\n%s: save FAILED for lexical frequency file '%s'\n", PROGNAME, lfout.name.c_str());
    }
    moot_msg(vlevel,vlProgress, " saved.\n");

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
      moot_croak("\n%s: save FAILED for n-gram frequency file '%s'\n", PROGNAME, ngout.name.c_str());
    } 
    moot_msg(vlevel,vlProgress, " saved.\n");

    ngout.close();
  }


  //-- save: classfreqs
  if (hmmt.want_classfreqs) {
    moot_msg(vlevel,vlProgress, "%s: saving class frequency file '%s'...", PROGNAME, lcout.name.c_str());

    //-- sanity check
    if (hmmt.lcfreqs.lctable.size() <= 2)
      moot_msg(vlevel,vlWarnings, "%s: Warning: found only %u lexical classes in training data: are you sure your data is pre-analyzed?\n", PROGNAME, hmmt.lcfreqs.lctable.size());

    //-- print summary to file
    lcout.printf("%s  Num/Tokens    : %g\n", cmts, hmmt.lcfreqs.totalcount);
    lcout.printf("%s  Num/Classes   : %u\n", cmts, hmmt.lcfreqs.lctable.size());
    lcout.printf("%s  Num/Tags      : %u\n", cmts, hmmt.lcfreqs.tagtable.size());
    lcout.printf("%s  Num/Pairs     : %u class*tag\n", cmts, hmmt.lcfreqs.n_pairs());
    lcout.printf("%s  - Impossibles : %u class*tag\n", cmts, hmmt.lcfreqs.n_impossible());

    //-- classfreqs: save: guts
    if (!hmmt.lcfreqs.save(lcout.file, lcout.name.c_str())) {
      moot_croak("\n%s: save FAILED for class frequency file '%s'\n", PROGNAME, lcout.name.c_str());
    }
    moot_msg(vlevel,vlProgress," saved.\n");

    lcout.close();
  }

  //-- save: flavors
  if (hmmt.want_flavors) {
    moot_msg(vlevel,vlProgress, "%s: saving flavor rule file '%s'...", PROGNAME, flout.name.c_str());

    //-- print summary to file
    flout.printf("%s  LC_CTYPE=%s\n", cmts, moot_lc_ctype());

    //-- flacors: save: guts
    if (!taster.save(&flout)) {
      moot_croak("\n%s: save FAILED for flavor rule file '%s'\n", PROGNAME, flout.name.c_str());
    }
    moot_msg(vlevel,vlProgress," saved.\n");

    flout.close();
  }

  return 0;
}
