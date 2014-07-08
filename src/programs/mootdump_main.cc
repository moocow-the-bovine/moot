/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2014 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootdump_main.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + HMM PoS tagger/disambiguator for DWDS project : model dumper
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <string>

#include <mootHMM.h>
#include <mootLexfreqs.h>
#include <mootClassfreqs.h>
#include <mootNgrams.h>

#include <mootUtils.h>
#include <mootCIO.h>

#include "mootdump_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "mootdump";

// options & file-churning
gengetopt_args_info  args;

// -- files
mofstream out;

// -- global classes/structs
mootHMM        hmm;

//-- dump options
bool dump_constants = true;
bool dump_lexprobs = true;
bool dump_classprobs = true;
bool dump_suftrie = true;
bool dump_ngprobs = true;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  cmdline_parser_envdefaults(&args);

  //-- locale
  moot_setlocale();

  //-- sanity check
  if (args.inputs_num <= 0) {
    fprintf(stderr, "%s: no input model specified!\n", PROGNAME);
    exit(1);
  }
  else if (args.inputs_num > 1) {
    fprintf(stderr, "%s: multiple input models specified -- ignoring all but first!\n", PROGNAME);
  }

  //-- show banner
  if (!args.no_banner_given)
    moot_msg(args.verbose_arg, vlInfo, moot_program_banner(PROGNAME, PACKAGE_VERSION, "Bryan Jurish <moocow@cpan.org>").c_str());

  //-- output file
  if (!out.open(args.output_arg,"w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name.c_str(), strerror(errno));
    exit(1);
  }

  // -- assign "verbose" flag
  hmm.verbose = args.verbose_arg;

  //-- assign dump options
  if (args.const_given || args.lex_given || args.class_given || args.suffix_given || args.ngrams_given) {
    dump_constants = args.const_flag;
    dump_lexprobs = args.lex_flag;
    dump_classprobs = args.class_flag;
    dump_suftrie = args.suffix_flag;
    dump_ngprobs = args.ngrams_flag;
  } else {
    dump_constants = true;
    dump_lexprobs = true;
    dump_classprobs = true;
    dump_suftrie = true;
    dump_ngprobs = true;
  }

  hmm.hash_ngrams = args.hash_ngrams_arg;
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  //-- the guts : load input model
  if (!hmm.load_model(args.inputs[0], "__$", PROGNAME)) {
    fprintf(stderr, "%s: load FAILED for model `%s' -- aborting!\n", PROGNAME, args.inputs[0]);
    exit(1);
  }

  //-- produce text dump
  if (args.verbose_arg > 1)
    fprintf(stderr, "%s: writing HMM text dump to '%s' ...", PROGNAME, out.name.c_str());

  hmm.txtdump(out.file, dump_constants, dump_lexprobs, dump_classprobs, dump_suftrie, dump_ngprobs);

  if (args.verbose_arg > 1)
    fprintf(stderr," dumped.\n");

  return 0;
}
