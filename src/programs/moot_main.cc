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
 * File: moot_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot HMM PoS tagger/disambiguator : main()
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <string>

#include <mootHMM.h>
#include <mootLexfreqs.h>
#include <mootClassfreqs.h>
#include <mootNgrams.h>
#include <mootTokenIO.h>
#include <mootTokenExpatIO.h>

#include <mootCIO.h>
#include <mootUtils.h>
#include "moot_cmdparser.h"

using namespace std;
using namespace moot;

typedef enum {
  vlSilent = 0,
  vlErrors = 1,
  vlWarnings = 2,
  vlSummary = 3,
  vlProgress = 4,
  vlEverything = 5
} verbosityLevel;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "moot";

// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;

// -- files
mofstream out;
size_t nfiles = 0;

// -- global classes/structs
mootHMM        hmm;


// -- token i/o
int ifmt         = tiofNone;
int ifmt_implied = tiofNone;
int ifmt_default = tiofMediumRare;

int  ofmt         = tiofNone;
int  ofmt_implied = tiofTagged;
int &ofmt_default = ifmt;

TokenReader *reader = NULL;
TokenWriter *writer = NULL;

// -- for verbose timing info
timeval istarted, astarted, astopped;
double  ielapsed, aelapsed;

/*--------------------------------------------------------------------------
 * Protos
 *--------------------------------------------------------------------------*/
void print_summary(TokenWriter *tw);

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  cmdline_parser_envdefaults(&args);

  //-- show banner
  if (args.verbose_arg > vlSilent)
    fprintf(stderr,
	    moot_program_banner(PROGNAME,
				PACKAGE_VERSION,
				"Bryan Jurish <jurish@ling.uni-potsdam.de>").c_str());

  //-- output file
  if (!out.open(args.output_arg,"w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name.c_str(), strerror(errno));
    exit(1);
  }

  //-- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;

  //-- get initialization start-time
  if (args.verbose_arg >= vlSummary) gettimeofday(&istarted, NULL);


  //-- i/o format : input
  ifmt = TokenIO::parse_format_request(args.input_format_arg,
				       (args.inputs_num>0 ? args.inputs[0] : NULL),
				       ifmt_implied,
				       ifmt_default);

  //-- i/o format : output
  if (args.save_ambiguities_given) ofmt_implied |= tiofAnalyzed;
  ofmt = TokenIO::parse_format_request(args.output_format_arg,
				       args.output_arg,
				       ofmt_implied,
				       ofmt_default);

  //-- io: new_reader, new_writer
  reader = TokenIO::new_reader(ifmt);
  writer = TokenIO::new_writer(ofmt);

#ifdef MOOT_EXPAT_ENABLED
  //-- io: encoding: reader
  if (ifmt&tiofXML && args.input_encoding_given) {
    ((TokenReaderExpat *)reader)->setEncoding(string(args.input_encoding_arg));
  }
  //-- io: encoding: writer
  if (ofmt&tiofXML && args.output_encoding_given) {
    ((TokenWriterExpat *)writer)->setEncoding(string(args.output_encoding_arg));
  }
#endif

  //-- io: writer: sink
  writer->to_mstream(&out);

  //-- assign "unknown" ids & other flags
  hmm.use_lex_classes = args.use_classes_arg;
  hmm.unknown_token_name(args.unknown_token_arg);
  hmm.unknown_tag_name(args.unknown_tag_arg);
  hmm.unknown_lex_threshhold = args.unknown_threshhold_arg;
  hmm.unknown_class_threshhold = args.class_threshhold_arg;
  hmm.suftrie.maxlen() = args.trie_depth_arg;
  hmm.suftrie.maxcount = args.trie_threshhold_arg;
  hmm.suftrie.theta    = args.trie_theta_arg;
  hmm.save_ambiguities = args.save_ambiguities_given;
  hmm.save_mark_unknown = args.mark_unknown_given;

  // -- assign "verbose" flag
  hmm.ndots = args.dots_arg;
  if (args.verbose_arg <= vlSilent) hmm.verbose = mootHMM::vlSilent;
  else if (args.verbose_arg <= vlErrors) hmm.verbose = mootHMM::vlErrors;
  else if (args.verbose_arg <= vlWarnings) hmm.verbose = mootHMM::vlWarnings;
  else if (args.verbose_arg <= vlSummary) hmm.verbose = mootHMM::vlWarnings;
  else if (args.verbose_arg <= vlProgress) hmm.verbose = mootHMM::vlProgress;
  else hmm.verbose = mootHMM::vlEverything;

  // -- parse and assign n-gram smoothing constants (nlambdas)
  if (args.nlambdas_arg) {
    double nlambdas[3] = {0,1,0};
    if (!moot_parse_doubles(args.nlambdas_arg, nlambdas, 3)) {
      fprintf(stderr, "%s: could not parse N-Gram smoothing constants '%s'\n",
	      PROGNAME, args.nlambdas_arg);
      exit(1);
    }
    hmm.nglambda1 = nlambdas[0];
    hmm.nglambda2 = nlambdas[1];
#ifdef MOOT_USE_TRIGRAMS
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
  }

  // -- parse and assign lexical smoothing constants (wlambdas)
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
  }

  // -- parse and assign lexical smoothing constants (clambdas)
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
  }

  //-- assign beam-width
  hmm.beamwd = args.beam_width_arg;

  //-- load model spec
  if (!hmm.load_model(args.model_arg,
		      args.eos_tag_arg,
		      PROGNAME,
		      !args.nlambdas_given,
		      !args.wlambdas_given,
		      !args.clambdas_given))
    {
      fprintf(stderr, "%s: could not load model `%s'\n", PROGNAME, args.model_arg);
    }

  // -- report
  if (args.verbose_arg >= vlProgress) {
    fprintf(stderr, "%s: Initialization complete\n", PROGNAME);
  }

  //-- get time
  time_t now_time = time(NULL);
  tm     now_tm;
  localtime_r(&now_time, &now_tm);

  //-- report to output-file
  writer->put_comment_block_begin();
  writer->printf_raw("\n %s output file generated on %s", PROGNAME, asctime(&now_tm));
  writer->printf_raw(" Configuration:\n");
  writer->printf_raw("   Unknown Token     : %s\n", hmm.tokids.id2name(0).c_str());
  writer->printf_raw("   Unknown Tag       : %s\n", hmm.tagids.id2name(0).c_str());
  writer->printf_raw("   Border Tag        : %s\n", hmm.tagids.id2name(hmm.start_tagid).c_str());
#ifdef MOOT_USE_TRIGRAMS
  writer->printf_raw("   N-Gram lambdas    : lambda1=%g, lambda2=%g, lambda3=%g\n",
	    exp(hmm.nglambda1), exp(hmm.nglambda2), exp(hmm.nglambda3));
#else
  writer->printf_raw("   N-Gram lambdas    : lambda1=%g, lambda2=%g\n",
	    exp(hmm.nglambda1), exp(hmm.nglambda2));
#endif
  writer->printf_raw("\n");
  writer->printf_raw("   Lex. Threshhold   : %g\n", hmm.unknown_lex_threshhold);
  writer->printf_raw("   Lexical lambdas   : lambdaw0=%g, lambdaw1=%g\n",
	  exp(hmm.wlambda0), exp(hmm.wlambda1));
  writer->printf_raw("   Use classes?      : %s\n",
	  hmm.use_lex_classes ? "yes" : "no");
  writer->printf_raw("   Class Threshhold  : %g\n", hmm.unknown_class_threshhold);
  writer->printf_raw("   Class lambdas     : lambdac0=%g, lambdac1=%g\n",
	  exp(hmm.clambda0), exp(hmm.clambda1));
  writer->printf_raw("   Beam Width        : %g\n", exp(hmm.beamwd));
  writer->printf_raw("   Suffix theta      : %g\n", hmm.suftrie.theta);
  writer->printf_raw("   Suffix trie size  : %u\n", hmm.suftrie.size());
  writer->printf_raw("\n");
  writer->put_comment_block_end();
}

/*--------------------------------------------------------------------------
 * Summary
 *--------------------------------------------------------------------------*/
void print_summary(TokenWriter *tw)
{
  // -- print summary
  tw->put_comment_block_begin();
  tw->printf_raw("=====================================================================\n");
  tw->printf_raw(" %s Summary:\n", PROGNAME);
  tw->printf_raw("  + General\n");
  tw->printf_raw("    - Input format        : \"%s\"\n",
		 TokenIO::format_canonical_string(ifmt).c_str());
  tw->printf_raw("    - Output format       : \"%s\"\n",
		 TokenIO::format_canonical_string(ofmt).c_str());
  tw->printf_raw("    - Files Processed     : %9u file(s)\n", nfiles);
  tw->printf_raw("    - Sentences Processed : %9u sent\n", hmm.nsents);
  tw->printf_raw("    - Tokens Processed    : %9u tok\n", hmm.ntokens);
  tw->printf_raw("  + Analysis\n");
  tw->printf_raw("    - Token Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nnewtokens,
	  100.0*((double)hmm.ntokens-(double)hmm.nnewtokens)/(double)hmm.ntokens,
	  hmm.nnewtokens,
	  100.0*(double)hmm.nnewtokens/(double)hmm.ntokens);
  tw->printf_raw("    - Class Given (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nunclassed,
	  100.0*((double)hmm.ntokens-(double)hmm.nunclassed)/(double)hmm.ntokens,
	  hmm.nunclassed,
	  100.0*(double)hmm.nunclassed/(double)hmm.ntokens);
  tw->printf_raw("    - Class Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nnewclasses,
	  100.0*((double)hmm.ntokens-(double)hmm.nnewclasses)/(double)hmm.ntokens,
	  hmm.nnewclasses,
	  100.0*(double)hmm.nnewclasses/(double)hmm.ntokens);
  tw->printf_raw("    - Total Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nunknown,
	  100.0*((double)hmm.ntokens-(double)hmm.nunknown)/(double)hmm.ntokens,
	  hmm.nunknown,
	  100.0*(double)hmm.nunknown/(double)hmm.ntokens);
  tw->printf_raw("    - Fallbacks           : %9u (%6.2f%%)\n",
	  hmm.nfallbacks,
	  100.0*(double)hmm.nfallbacks/(double)hmm.ntokens);
  tw->printf_raw("  + Performance\n");
  tw->printf_raw("    - Initialize Time     : %12.2f sec\n", ielapsed);
  tw->printf_raw("    - Analysis Time       : %12.2f sec\n", aelapsed);
  tw->printf_raw("    - Throughput Rate     : %12.2f tok/sec\n", (double)hmm.ntokens/aelapsed);
  tw->printf_raw("=====================================================================\n");
  tw->put_comment_block_end();
}

void print_summary_to_file(FILE *file)
{
  TokenWriterNative twn(tiofWellDone);
  twn.to_file(file);
  print_summary(&twn);
  twn.close();
}


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  if (args.verbose_arg >= vlSummary) gettimeofday(&astarted, NULL);

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    if (args.verbose_arg >= vlSummary) nfiles++;
    if (args.verbose_arg >= vlProgress) {
      writer->printf_comment("\n     File: %s\n", churner.in.name.c_str());
      fprintf(stderr,"%s: analyzing file '%s'...", PROGNAME, churner.in.name.c_str());
      fflush(stderr);
    }

    //hmm.tag_file(churner.in.file, out.file, churner.in.name);

    reader->from_mstream(&churner.in);
    hmm.tag_io(reader, writer);
    
    if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," done.\n");
      fflush(stderr);
    }
    else if (hmm.ndots) {
      fputc('\n', stderr);
    }
  }

  // -- summary
  if (args.verbose_arg >= vlSummary) {
      // -- timing
      gettimeofday(&astopped, NULL);

      aelapsed =
	astopped.tv_sec-astarted.tv_sec
	+ (double)(astopped.tv_usec-astarted.tv_usec)/1000000.0;

      ielapsed =
	astarted.tv_sec-istarted.tv_sec
	+ (double)(astarted.tv_usec-istarted.tv_usec)/1000000.0;

      if (out.file != stdout) print_summary(writer);
      print_summary_to_file(stderr);
  }
  writer->close();
  out.close();

  return 0;
}

