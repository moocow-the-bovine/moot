/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2014 Jurish <moocow@cpan.org>

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
 * File: moottaste_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot HMM PoS tagger/disambiguator : typifier: main()
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include <string>

#include <mootFlavor.h>
#include <mootTokenIO.h>
#include <mootTokenExpatIO.h>
#include <mootCIO.h>
#include <mootUtils.h>
#include "moottaste_cmdparser.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "moottaste";

// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;
int vlevel;

// -- files
mofstream out;

// -- token i/o
int ifmt         = tiofNone;
int ifmt_implied = tiofNone;
int ifmt_default = tiofMediumRare;

int  ofmt         = tiofNone;
int  ofmt_implied = tiofTagged;
int &ofmt_default = ifmt;

TokenReader *reader = NULL;
TokenWriter *writer = NULL;

//-- taster
mootTaster taster;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- locale
  moot_setlocale();

  //-- load environmental defaults
  cmdline_parser_envdefaults(&args);

  //-- verbosity
  vlevel = args.verbose_arg;

  //-- show banner
  if (!args.no_banner_given)
    moot_msg(vlevel, vlInfo,  moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

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
    ((TokenReaderExpat *)reader)->setEncoding(string(args.input_encoding_arg));
  }
  //-- io: encoding: writer
  if (ofmt&tiofXML && args.output_encoding_given) {
    ((TokenWriterExpat *)writer)->setEncoding(string(args.output_encoding_arg));
  }
#endif

  //-- io: writer: sink
  writer->to_mstream(&out);

  //-- taster
  if (args.flavors_given && args.flavors_arg[0]) {
    //-- flavors given and non-empty: load file
    mifstream tin;
    if (!tin.open(args.flavors_arg,"r"))
      moot_croak("%s: open failed for flavor file '%s': %s\n", PROGNAME, tin.name.c_str(), strerror(errno));
    taster.clear();
    taster.load(&tin);
    tin.close();
    if (vlevel >= vlProgress) {
      writer->printf_comment("  Flavors: %s\n", args.flavors_arg);
      fprintf(stderr,"%s: loaded %zu flavor rules from file '%s'\n", PROGNAME, taster.size(), args.flavors_arg);
      fflush(stderr);
    }
  }
  else if (args.flavors_given) {
    //-- flavors given and empty: disable
    taster.clear();
    if (vlevel >= vlProgress) {
      writer->printf_comment("  Flavors: (disabled)\n");
      fprintf(stderr,"%s: disabled flavors\n", PROGNAME);
      fflush(stderr);
    }
  }
  else {
    //-- flavors not given: use default rules
    taster.set_default_rules();
    if (vlevel >= vlProgress) {
      writer->printf_comment("  Flavors: (built-in)\n");
      fprintf(stderr,"%s: using %zu built-in flavor rules\n", PROGNAME, taster.size());
      fflush(stderr);
    }
  }

  if (args.default_flavor_given) {
    taster.set_default_label(args.default_flavor_arg);
  }
}


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    if (vlevel >= vlProgress) {
      writer->printf_comment("\n     File: %s\n", churner.in.name.c_str());
      fprintf(stderr,"%s: analyzing file '%s'...", PROGNAME, churner.in.name.c_str());
      fflush(stderr);
    }

    //hmm.tag_file(churner.in.file, out.file, churner.in.name);

    reader->from_mstream(&churner.in);
    int rtok;
    while (reader && (rtok = reader->get_sentence()) != TokTypeEOF) {
      mootSentence *sent = reader->sentence();
      for (mootSentence::iterator si=sent->begin(); si != sent->end(); si++) {
	const std::string &flavor = taster.flavor(si->text());
	si->besttag(flavor);
      }
      if (writer) writer->put_sentence(*sent);
    }
    
    if (vlevel >= vlProgress) {
      fprintf(stderr," done.\n");
      fflush(stderr);
    }
  }

  writer->close();
  out.close();
  return 0;
}

