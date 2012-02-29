/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2004-2010 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootchurn_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's tagger: churner: main()
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <stdexcept>

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <mootTokenIO.h>
#include <mootTokenXmlDoc.h>
#include <mootTokenExpatIO.h>

#include <mootCIO.h>
#include <mootUtils.h>
#include "mootchurn_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "mootchurn";

// files
mifstream in;
mofstream out;

// options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

//-- token i/o
int ifmt = tiofNone;
int ifmt_implied = tiofNone;
int ifmt_default = tiofWellDone|tiofConserve|tiofCost;

int  ofmt = tiofNone;
int  ofmt_implied = tiofNone;
int &ofmt_default = ifmt;

TokenReader *reader = NULL;
TokenWriter *writer = NULL;

//-- information
size_t ntokens = 0;     //-- total number of tokens
size_t nsents = 0;      //-- total number of sentences
size_t nfiles = 0;      //-- total number of input files

int ndots = 0;

// -- for verbose timing info
double  elapsed;

//-- verbosity levels
typedef enum {
  vlSilent = 0,
  vlErrors = 1,
  vlWarnings = 2,
  vlSummary = 3,
  vlProgress = 4,
  vlEverything = 5
} verbosityLevel;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- show banner
  if (args.verbose_arg > 0) {
    fprintf(stderr,
	    moot_program_banner(PROGNAME,
				PACKAGE_VERSION,
				"Bryan Jurish <moocow@cpan.org>").c_str());
  }

  //-- options: verbosity
  //(ignored)

  //-- options: output file
  if (!out.open(args.output_arg,"wb")) {
    fprintf(stderr,"%s: open failed for output-file \"%s\": %s\n",
	    PROGNAME, out.name.c_str(), strerror(errno));
    exit(1);
  }

  //-- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;

  //-- i/o format : input
  ifmt = TokenIO::parse_format_request(args.input_format_arg,
				       (args.inputs_num>0 ? args.inputs[0] : NULL),
				       ifmt_implied,
				       ifmt_default);

  //-- i/o format : output
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
    reinterpret_cast<TokenReaderExpat *>(reader)->setEncoding(args.input_encoding_arg);
  }
  //-- io: encoding: writer
  if (ofmt&tiofXML && args.output_encoding_given) {
    reinterpret_cast<TokenWriterExpat *>(writer)->setEncoding(args.input_encoding_arg);
  }
#endif // MOOT_EXPAT_ENABLED

  //-- io: writer sink
  writer->to_mstream(&out);

  //-- dot printing
  ndots = args.dots_arg;
}

/*--------------------------------------------------------------------------
 * Summary
 *--------------------------------------------------------------------------*/
void print_summary(FILE *file)
{
  // -- print summary
  fprintf(file,
	  "\n%%%%---------------------------------------------------------------------\n");
  fprintf(file, "%%%% %s Summary:\n", PROGNAME);
  fprintf(file, "%%%%  + Formats\n");
  fprintf(file, "%%%%    - Input Format        : \"%s\"\n",
	  TokenIO::format_canonical_string(ifmt).c_str());
  fprintf(file, "%%%%    - Output Format       : \"%s\"\n",
	  TokenIO::format_canonical_string(ofmt).c_str());
  fprintf(file, "%%%%  + General\n");
  fprintf(file, "%%%%    - Files Processed     : %9u file(s)\n", static_cast<unsigned int>(nfiles));
  fprintf(file, "%%%%    - Sentences Processed : %9u sent\n",    static_cast<unsigned int>(nsents));
  fprintf(file, "%%%%    - Tokens Processed    : %9u tok\n",     static_cast<unsigned int>(ntokens));
  fprintf(file, "%%%%  + Performance\n");
  fprintf(file, "%%%%    - Processing Time     : %12.2f sec\n", elapsed);
  fprintf(file, "%%%%    - Throughput Rate     : %12.2f tok/sec\n", static_cast<double>(ntokens)/elapsed);
  fprintf(file,
	  "%%%%---------------------------------------------------------------------\n");
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  try {
    GetMyOptions(argc,argv);
  }
  catch (exception &e) {
    fprintf(stderr, "%s: Exception: %s\n", PROGNAME, e.what());
    abort();
  }

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file())
    {
      if (args.verbose_arg >= vlProgress) {
	//fprintf(out.file, "\n%%%% File: %s\n\n", churner.in.name);
	fprintf(stderr,"%s: churning file '%s'...",
		PROGNAME, churner.in.name.c_str());
	fflush(stderr);
      }
      nfiles++;

      reader->from_mstream(&churner.in);

      int lxtok;
      mootSentence *sent;
      if (!args.tokens_given) {
	//-- read input sentence-wise
	while ((lxtok = reader->get_sentence()) != TokTypeEOF) {
	  if ( !(sent = reader->sentence()) ) continue;

	  if (!(ofmt&tiofNull)) writer->put_sentence(*sent);
	
	  nsents++;
	  for (mootSentence::const_iterator si = sent->begin();
	       si != sent->end();
	       si++)
	    {
	      ntokens++;
	      if (ndots && (ntokens % ndots) == 0 ) fputc('.', stderr);
	    }
	}
      }
      else {
	//-- read input token-wise
	mootToken *tok;
	while ((lxtok = reader->get_token()) != TokTypeEOF) {
	  if ( !(tok = reader->token()) ) continue;

	  switch (lxtok) {
	  case TokTypeVanilla:
	  case TokTypeLibXML:
	    ntokens++;
	    if (ndots && (ntokens % ndots) == 0 ) fputc('.', stderr);
	  case TokTypeComment:
	    if (!(ofmt&tiofNull)) writer->put_token(*tok);
	    break;
	  case TokTypeEOS:
	    if (!(ofmt&tiofNull)) writer->put_token(*tok);
	    nsents++;
	    break;
	  default:
	    // ignored
	    break;
	  }
	}
      }
    }

  //-- dots
  if (ndots) fputc('\n', stderr);

  //-- timing

  //-- report summary
  if (args.verbose_arg >= vlSummary) {
    elapsed = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);
    print_summary(stderr);
  }

  //-- cleanup
  writer->close();
  out.close();
  if (reader) delete reader;
  if (writer) delete writer;

  return 0;
}
