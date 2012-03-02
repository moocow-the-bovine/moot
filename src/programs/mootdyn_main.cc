/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2010-2012 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootdyn_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot dynamic HMM PoS tagger/disambiguator : main()
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

#include <moot.h>
#include <mootUtils.h>
#include "mootdyn_cmdparser.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "mootdyn";

// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;

// -- files
mofstream out;
size_t nfiles = 0;

// -- global classes/structs
mootDynHMM *hmmp = NULL;
mootDynHMMOptions hmm_opts;

// -- token i/o
int ifmt         = tiofNone;
//int ifmt_implied = tiofNone;
int ifmt_implied = tiofMediumRare;
int ifmt_default = tiofMediumRare|tiofCost;

int  ofmt         = tiofNone;
int  ofmt_implied = tiofTagged;
int &ofmt_default = ifmt;

TokenReader *reader = NULL;
TokenWriter *writer = NULL;

// -- for verbose timing info
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
  if (!args.no_banner_given)
    moot_msg(args.verbose_arg, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

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
  churner.paranoid = !args.recover_given;

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
    reinterpret_cast<TokenReaderExpat *>(reader)->setEncoding(static_cast<std::string>(args.input_encoding_arg));
  }
  //-- io: encoding: writer
  if (ofmt&tiofXML && args.output_encoding_given) {
    reinterpret_cast<TokenWriterExpat *>(writer)->setEncoding(static_cast<std::string>(args.output_encoding_arg));
  }
#endif

  //-- io: writer: sink
  writer->to_mstream(&out);

  //-- mootDynHMM generic options
  hmm_opts.invert_lexp = args.dyn_invert_arg;
  hmm_opts.newtag_str  = args.dyn_new_tag_arg;
  hmm_opts.Ftw_eps     = args.dyn_freq_eps_arg;
  hmm_opts.dynlex_base = args.dyn_base_arg;
  hmm_opts.dynlex_beta = args.dyn_beta_arg;
  hmm_opts.text_tags   = args.dyn_text_tags_flag;

  //-- create HMM (dynamic)
  hmmp = newDynHMM(args.dyn_class_arg, hmm_opts);
  if (hmmp == NULL) {
    fprintf(stderr, "%s: unknown dynamic HMM estimator class `%s'\n",
	    PROGNAME, args.dyn_class_arg);
    exit(1);
  }

  //-- assign "unknown" ids & other flags
  hmmp->hash_ngrams = args.hash_ngrams_arg;
  hmmp->relax = args.relax_arg;
  hmmp->use_lex_classes = args.use_classes_arg;
  hmmp->unknown_token_name(args.unknown_token_arg);
  hmmp->unknown_tag_name(args.unknown_tag_arg);
  hmmp->unknown_lex_threshhold = args.unknown_threshhold_arg;
  hmmp->unknown_class_threshhold = args.class_threshhold_arg;
#ifdef MOOT_ENABLE_SUFFIX_TRIE
  hmmp->suftrie.maxlen() = args.trie_depth_arg;
  hmmp->suftrie.maxcount = args.trie_threshhold_arg;
  hmmp->suftrie.theta    = args.trie_theta_arg;
#else
  if (args.trie_depth_given || args.trie_threshhold_given || args.trie_theta_given) {
    fprintf(stderr, "%s: suffix trie support disabled: ignoring trie-related options\n", PROGNAME);
  }
#endif
  hmmp->save_ambiguities = args.save_ambiguities_given;
  hmmp->save_mark_unknown = args.mark_unknown_given;

  // -- assign "verbose" flag
  hmmp->ndots = args.dots_arg;
  hmmp->verbose = args.verbose_arg;

  // -- parse and assign n-gram smoothing constants (nlambdas)
  if (args.nlambdas_given) {
    double nlambdas[3] = {0,1,0};
    if (!moot_parse_doubles(args.nlambdas_arg, nlambdas, 3)) {
      fprintf(stderr, "%s: could not parse N-Gram smoothing constants '%s'\n",
	      PROGNAME, args.nlambdas_arg);
      exit(1);
    }
    hmmp->nglambda1 = nlambdas[0];
    hmmp->nglambda2 = nlambdas[1];
#ifdef MOOT_USE_TRIGRAMS
    hmmp->nglambda3 = nlambdas[2];
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
  if (args.wlambdas_given) {
    double wlambdas[2] = {1,0};
    if (!moot_parse_doubles(args.wlambdas_arg, wlambdas, 2)) {
      fprintf(stderr, "%s: could not parse lexical smoothing constants '%s'\n",
	      PROGNAME, args.wlambdas_arg);
      exit(1);
    }
    hmmp->wlambda0 = wlambdas[0];
    hmmp->wlambda1 = wlambdas[1];
    if (hmmp->wlambda0 + hmmp->wlambda1 != 1) {
      fprintf(stderr, "%s: Warning: Lexical smoothing constants do not sum to one: %s\n",
	      PROGNAME, args.wlambdas_arg);
    }
  }

  // -- parse and assign lexical smoothing constants (clambdas)
  if (args.clambdas_given) {
    double clambdas[2] = {1,0};
    if (!moot_parse_doubles(args.clambdas_arg, clambdas, 2)) {
      fprintf(stderr, "%s: could not parse lexical-class smoothing constants '%s'\n",
	      PROGNAME, args.clambdas_arg);
      exit(1);
    }
    hmmp->clambda0 = clambdas[0];
    hmmp->clambda1 = clambdas[1];
    if (hmmp->clambda0 + hmmp->clambda1 != 1) {
      fprintf(stderr, "%s: Warning: Lexical-class smoothing constants do not sum to one: %s\n",
	      PROGNAME, args.clambdas_arg);
    }
  }

  //-- assign beam-width
  hmmp->beamwd = args.beam_width_arg;

  //-- load model spec
  if (!hmmp->load_model(args.model_arg,
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
  /*
  tm     now_tm;
  localtime_r(&now_time, &now_tm);
  */
  tm *now_tm = localtime(&now_time);

  //-- report to output-file
  if (!args.no_header_given) {
    writer->put_comment_block_begin();
    writer->printf_raw(" %s output file generated on %s", PROGNAME, asctime(now_tm));
    writer->printf_raw("\n");
    writer->printf_raw(" DynHMM Configuration:\n");
    hmmp->tw_put_info(writer);
    writer->printf_raw("\n");
    writer->printf_raw(" HMM Configuration:\n");
    writer->printf_raw("   Unknown Token     : %s\n", hmmp->tokids.id2name(0).c_str());
    writer->printf_raw("   Unknown Tag       : %s\n", hmmp->tagids.id2name(0).c_str());
    writer->printf_raw("   Border Tag        : %s\n", hmmp->tagids.id2name(hmmp->start_tagid).c_str());
    writer->printf_raw("   N-Gram lambdas    : lambda1=%g, lambda2=%g, lambda3=%g\n",
		       exp(hmmp->nglambda1), exp(hmmp->nglambda2), exp(hmmp->nglambda3));
    writer->printf_raw("\n");
    writer->printf_raw("   Hash n-grams?     : %s\n", (hmmp->hash_ngrams ? "yes" : "no"));
    writer->printf_raw("   Lex. Threshhold   : %g\n", hmmp->unknown_lex_threshhold);
    writer->printf_raw("   Lexical lambdas   : lambdaw0=%g, lambdaw1=%g\n",
		       exp(hmmp->wlambda0), exp(hmmp->wlambda1));
    writer->printf_raw("   Use classes?      : %s\n",
		       hmmp->use_lex_classes ? "yes" : "no");
    writer->printf_raw("   Class Threshhold  : %g\n", hmmp->unknown_class_threshhold);
    writer->printf_raw("   Class lambdas     : lambdac0=%g, lambdac1=%g\n",
		       exp(hmmp->clambda0), exp(hmmp->clambda1));
    writer->printf_raw("   Beam Width        : %g\n", exp(hmmp->beamwd));
#ifdef MOOT_ENABLE_SUFFIX_TRIE
    writer->printf_raw("   Suffix theta      : %g\n", hmmp->suftrie.theta);
    writer->printf_raw("   Suffix trie size  : %u\n", hmmp->suftrie.size());
#else
    writer->printf_raw("   Suffix theta      : (DISABLED)\n");
    writer->printf_raw("   Suffix trie size  : (DISABLED)\n");
#endif
    writer->printf_raw("\n");
    writer->put_comment_block_end();
  }
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
  tw->printf_raw("    - Sentences Processed : %9u sent\n", hmmp->nsents);
  tw->printf_raw("    - Tokens Processed    : %9u tok\n", hmmp->ntokens);
  tw->printf_raw("  + Analysis\n");
  tw->printf_raw("    - Token Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmmp->ntokens-hmmp->nnewtokens,
	  100.0*(static_cast<double>(hmmp->ntokens)-static_cast<double>(hmmp->nnewtokens))/static_cast<double>(hmmp->ntokens),
	  hmmp->nnewtokens,
	  100.0*static_cast<double>(hmmp->nnewtokens)/static_cast<double>(hmmp->ntokens));
  tw->printf_raw("    - Class Given (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmmp->ntokens-hmmp->nunclassed,
	  100.0*(static_cast<double>(hmmp->ntokens)-static_cast<double>(hmmp->nunclassed))/static_cast<double>(hmmp->ntokens),
	  hmmp->nunclassed,
	  100.0*static_cast<double>(hmmp->nunclassed)/static_cast<double>(hmmp->ntokens));
  tw->printf_raw("    - Class Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmmp->ntokens-hmmp->nnewclasses,
	  100.0*(static_cast<double>(hmmp->ntokens)-static_cast<double>(hmmp->nnewclasses))/static_cast<double>(hmmp->ntokens),
	  hmmp->nnewclasses,
	  100.0*static_cast<double>(hmmp->nnewclasses)/static_cast<double>(hmmp->ntokens));
  tw->printf_raw("    - Total Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmmp->ntokens-hmmp->nunknown,
	  100.0*(static_cast<double>(hmmp->ntokens)-static_cast<double>(hmmp->nunknown))/static_cast<double>(hmmp->ntokens),
	  hmmp->nunknown,
	  100.0*static_cast<double>(hmmp->nunknown)/static_cast<double>(hmmp->ntokens));
  tw->printf_raw("    - Fallbacks           : %9u (%6.2f%%)\n",
	  hmmp->nfallbacks,
	  100.0*static_cast<double>(hmmp->nfallbacks)/static_cast<double>(hmmp->ntokens));
  tw->printf_raw("  + Performance\n");
  tw->printf_raw("    - Initialize Time     : %12.2f sec\n", ielapsed);
  tw->printf_raw("    - Analysis Time       : %12.2f sec\n", aelapsed);
  tw->printf_raw("    - Throughput Rate     : %12.2f tok/sec\n", static_cast<double>(hmmp->ntokens)/aelapsed);
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
  if (args.verbose_arg >= vlInfo) {
    ielapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);
  }

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    if (args.verbose_arg >= vlInfo) nfiles++;
    if (args.verbose_arg >= vlProgress) {
      writer->printf_comment("\n File: %s\n", churner.in.name.c_str());
      fprintf(stderr,"%s: analyzing file '%s'...", PROGNAME, churner.in.name.c_str());
      fflush(stderr);
    }

    //hmmp->tag_file(churner.in.file, out.file, churner.in.name);

    reader->from_mstream(&churner.in);
    hmmp->tag_io(reader, writer);
    
    if (args.verbose_arg >= vlProgress) {
      fprintf(stderr," done.\n");
      fflush(stderr);
    }
    else if (hmmp->ndots) {
      fputc('\n', stderr);
    }
  }

  // -- summary
  if (args.verbose_arg >= vlInfo) {
    // -- timing
    aelapsed  = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC) - ielapsed; 

    if (out.file != stdout) print_summary(writer);
    print_summary_to_file(stderr);
  }
  writer->close();
  out.close();

  return 0;
}

