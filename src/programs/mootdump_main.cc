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
 * File: mootdump_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
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
char *PROGNAME = "mootdump";

// options & file-churning
gengetopt_args_info  args;

// -- files
mofstream out;

// -- global classes/structs
mootHMM        hmm;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  cmdline_parser_envdefaults(&args);

  //-- sanity check
  if (args.inputs_num <= 0) {
    fprintf(stderr, "%s: no input model specified!\n", PROGNAME);
    exit(1);
  }
  else if (args.inputs_num > 1) {
    fprintf(stderr, "%s: multiple input models specified -- ignoring all but first!\n",
	    PROGNAME);
  }

  //-- show banner
  if (args.verbose_arg > 1)
    fprintf(stderr,
	    "\n%s version %s by Bryan Jurish <moocow@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  //-- output file
  if (!out.open(args.output_arg,"w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name.c_str(), strerror(errno));
    exit(1);
  }

  // -- assign "verbose" flag
  if (args.verbose_arg <= 0) hmm.verbose = mootHMM::vlSilent;
  else if (args.verbose_arg <= 1) hmm.verbose = mootHMM::vlErrors;
  else if (args.verbose_arg <= 2) hmm.verbose = mootHMM::vlWarnings;
  else if (args.verbose_arg <= 3) hmm.verbose = mootHMM::vlProgress;
  else hmm.verbose = mootHMM::vlEverything;
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  //-- the guts : load input model
  if (!hmm.load_model(args.inputs[0], "__$", PROGNAME)) {
    fprintf(stderr, "%s: load FAILED for model `%s' -- aborting!\n",
	    PROGNAME, args.inputs[0]);
    exit(1);
  }

  //-- produce text dump
  if (args.verbose_arg > 1)
    fprintf(stderr, "%s: writing HMM text dump to '%s' ...",
	    PROGNAME, out.name.c_str());

  hmm.txtdump(out.file);

  if (args.verbose_arg > 1)
    fprintf(stderr," dumped.\n");

  return 0;
}
