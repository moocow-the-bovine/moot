/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2009 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootngsmooth_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + HMM PoS tagger/disambiguator for DWDS project : n-gram smoother
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
#include <mootUtils.h>
#include <mootCIO.h>
#include <mootUtils.h>

#include "mootngsmooth_cmdparser.h"

using namespace std;
using namespace moot;
using namespace mootio;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
const char *PROGNAME = "mootngsmooth";

// options & file-churning
gengetopt_args_info  args;

//-- files
mofstream out;

//-- global classes/structs
mootNgrams ngrams;

std::string newtag;

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
    fprintf(stderr, "%s: no input n-gram model specified!\n", PROGNAME);
    exit(1);
  }
  else if (args.inputs_num > 1) {
    fprintf(stderr, "%s: multiple input models specified -- ignoring all but first!\n",
	    PROGNAME);
  }

  //-- show banner
  if (args.verbose_arg > 1)
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

  //-- assign 'new-tag' option
  newtag = args.newtag_arg;
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  //-- load n-gram file
  if (!ngrams.load(args.inputs[0])) {
    fprintf(stderr, "%s: load FAILED for n-gram model `%s' -- aborting!\n",
	    PROGNAME, args.inputs[0]);
    exit(1);
  }

  //-- smooth n-grams
  ngrams.smooth_add_newtag(newtag);

  //-- dump
  mofstream ngout;
  if (!ngout.open(args.output_arg,"w")) {
    fprintf(stderr, "%s: open failed for output file '%s': %s\n",
	    PROGNAME, args.output_arg, strerror(errno));
    exit(2);
  }
  char cmts[3] = "%%";
  time_t now_time = time(NULL);
  tm *now_tm = localtime(&now_time);
  ngout.printf("%s %s ngram frequency file generated on %s",
	       cmts, PROGNAME, asctime(now_tm));
  ngout.printf("%s source file: %s\n", cmts, args.inputs[0]);
  ngout.printf("%s 'new' tag: %s\n", cmts, newtag.c_str());
  if (!ngrams.save(ngout.file, ngout.name.c_str(), !args.verbose_ngrams_given)) {
      fprintf(stderr, "\n%s: save FAILED for smoothed n-gram frequency file '%s'\n",
	      PROGNAME, ngout.name.c_str());
      exit(2);
  }
  ngout.close();

  return 0;
}
