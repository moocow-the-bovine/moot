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
 * File: moot_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
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

#include <string>
#include <mootTokenIO.h>
#include <mootTokenExpatIO.h>

#include "computils.h"
#include "moot_cmdparser.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/

// options & file-churning
gengetopt_args_info  args;
HmmSpec<gengetopt_args_info,mootHMM> spec;
mootHMM hmm;
cmdutil_file_churner churner;

// -- files
mofstream out;
size_t nfiles = 0;

//-- global classes/structs (see also computils.h)

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

  //-- show banner?
  if (!args.no_banner_given)
    moot_msg(args.verbose_arg, vlInfo,  moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

  //-- output file
  if (!out.open(args.output_arg,"w"))
    moot_croak("%s: open failed for output-file '%s': %s\n", PROGNAME, out.name.c_str(), strerror(errno));

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

  //-- load model
  spec.args = args;
  spec.hmmp = &hmm;
  if (!spec.load_hmm())
    moot_croak("%s: load FAILED for model `%s'\n", PROGNAME, spec.model_arg());

  //-- report
  moot_msg(vlevel,vlProgress,"%s: Initialization complete\n", PROGNAME);
  moot_msg(vlevel,vlInfo,"+ save_mark_unknown = %d\n", hmm.save_mark_unknown);

  //-- report to output-file
  if (!args.no_header_given)
    put_hmm_header(writer, hmm);
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
  tw->printf_raw("    - Input format        : \"%s\"\n", TokenIO::format_canonical_string(ifmt).c_str());
  tw->printf_raw("    - Output format       : \"%s\"\n", TokenIO::format_canonical_string(ofmt).c_str());
  tw->printf_raw("    - Files Processed     : %9u file(s)\n", nfiles);
  tw->printf_raw("    - Sentences Processed : %9u sent\n", hmm.nsents);
  tw->printf_raw("    - Tokens Processed    : %9u tok\n", hmm.ntokens);
  tw->printf_raw("  + Analysis\n");
  tw->printf_raw("    - Token Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nnewtokens,
	  100.0*(static_cast<double>(hmm.ntokens)-static_cast<double>(hmm.nnewtokens))/static_cast<double>(hmm.ntokens),
	  hmm.nnewtokens,
	  100.0*static_cast<double>(hmm.nnewtokens)/static_cast<double>(hmm.ntokens));
  tw->printf_raw("    - Class Given (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nunclassed,
	  100.0*(static_cast<double>(hmm.ntokens)-static_cast<double>(hmm.nunclassed))/static_cast<double>(hmm.ntokens),
	  hmm.nunclassed,
	  100.0*static_cast<double>(hmm.nunclassed)/static_cast<double>(hmm.ntokens));
  tw->printf_raw("    - Class Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nnewclasses,
	  100.0*(static_cast<double>(hmm.ntokens)-static_cast<double>(hmm.nnewclasses))/static_cast<double>(hmm.ntokens),
	  hmm.nnewclasses,
	  100.0*static_cast<double>(hmm.nnewclasses)/static_cast<double>(hmm.ntokens));
  tw->printf_raw("    - Total Known (+/-)   : %9u (%6.2f%%) / %9u (%6.2f%%)\n",
	  hmm.ntokens-hmm.nunknown,
	  100.0*(static_cast<double>(hmm.ntokens)-static_cast<double>(hmm.nunknown))/static_cast<double>(hmm.ntokens),
	  hmm.nunknown,
	  100.0*static_cast<double>(hmm.nunknown)/static_cast<double>(hmm.ntokens));
  tw->printf_raw("    - Fallbacks           : %9u (%6.2f%%)\n",
	  hmm.nfallbacks,
	  100.0*static_cast<double>(hmm.nfallbacks)/static_cast<double>(hmm.ntokens));
  tw->printf_raw("  + Performance\n");
  tw->printf_raw("    - Initialize Time     : %12.2f sec\n", ielapsed);
  tw->printf_raw("    - Analysis Time       : %12.2f sec\n", aelapsed);
  tw->printf_raw("    - Throughput Rate     : %12.2f tok/sec\n", static_cast<double>(hmm.ntokens)/aelapsed);
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
  PROGNAME = "moot";
  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  if (vlevel >= vlInfo)
    ielapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    ++nfiles;
    if (vlevel >= vlProgress) {
      writer->printf_comment("\n     File: %s\n", churner.in.name.c_str());
      moot_carp("%s: analyzing file '%s'...", PROGNAME, churner.in.name.c_str());
    }

    //hmm.tag_file(churner.in.file, out.file, churner.in.name);

    reader->from_mstream(&churner.in);
    if (args.stream_given) {
      hmm.tag_stream(reader,writer);
    } else {
      hmm.tag_io(reader, writer);
    }
    
    if (vlevel >= vlProgress) {
      moot_carp(" done.\n");
    } else if (hmm.ndots) {
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
