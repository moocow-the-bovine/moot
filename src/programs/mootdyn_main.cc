/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2010-2014 Jurish <moocow@cpan.org>

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

#include <string>

#include <moot.h>
#include <mootUtils.h>

#include "computils.h"
#include "mootdyn_cmdparser.h"

using namespace std;
using namespace moot;

//----------------------------------------------------------------------
// spec overrides
namespace moot {
  template<> int  HmmSpec<gengetopt_args_info,mootDynHMM>::use_flavors_arg() { return false; };
  template<> bool HmmSpec<gengetopt_args_info,mootDynHMM>::use_flavors_given() { return false; };
};

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;

// -- files
mofstream out;
size_t nfiles = 0;

// -- global classes/structs
mootDynHMM *hmmp = NULL;
mootDynHMMOptions hmm_opts;
HmmSpec<gengetopt_args_info,mootDynHMM> spec;

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

  //-- initialize locale
  moot_setlocale();

  //-- get verbosity level
  vlevel = args.verbose_arg;

  //-- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

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

  //-- standard model options
  spec.args = args;
  spec.hmmp = hmmp;
  if (!spec.load_hmm())
    moot_croak("%s: load FAILED for model `%s'\n", PROGNAME, spec.model_arg());

  // -- report
  moot_msg(vlevel,vlProgress,"%s: Initialization complete\n", PROGNAME);

  //-- report to output-file
  if (!args.no_header_given)
    put_hmm_header(writer, *hmmp);
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
  PROGNAME = "mootdyn";
  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  if (vlevel >= vlInfo)
    ielapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    ++nfiles;
    if (vlevel >= vlProgress) {
      writer->printf_comment("\n File: %s\n", churner.in.name.c_str());
      moot_carp("%s: analyzing file '%s'...", PROGNAME, churner.in.name.c_str());
    }

    //hmmp->tag_file(churner.in.file, out.file, churner.in.name);

    reader->from_mstream(&churner.in);
    hmmp->tag_io(reader, writer);
    
    if (vlevel >= vlProgress) {
      moot_carp(" done.\n");
    }
    else if (hmmp->ndots) {
      fputc('\n', stderr);
    }
  }

  // -- summary
  if (vlevel >= vlInfo) {
    // -- timing
    aelapsed  = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC) - ielapsed; 

    if (out.file != stdout) print_summary(writer);
    print_summary_to_file(stderr);
  }
  writer->close();
  out.close();

  return 0;
}

