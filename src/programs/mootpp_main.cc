/*
   moot-utils version 1.0.4 : moocow's part-of-speech tagger
   Copyright (C) 2002-2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootpp_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's tagger: preprocessor : main()
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

#include <mootPPLexer.h>
#include "mootpp_cmdparser.h"
#include "cmdutil.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mootpp";

// files
cmdutil_file_info out;

// options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- show banner
  if (args.verbose_arg > 0)
    fprintf(stderr,
	    "\n%s version %s by Bryan Jurish <jurish@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  // -- output file
  out.name = args.output_arg;
  if (strcmp(out.name,"-") == 0) out.name = "<stdout>";
  if (!out.open("w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name, strerror(errno));
    exit(1);
  }

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  mootPPLexer lexer;
  int nfiles = 0;
  timeval started,stopped;
  double elapsed;

  GetMyOptions(argc,argv);
  lexer.verbose = args.verbose_arg;

  // -- get start time
  if (args.verbose_arg > 0) {
      gettimeofday(&started, NULL);
  }

  // -- big loop
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
      if (args.verbose_arg > 0) {
	nfiles++;
	if (args.verbose_arg > 1) {
	  fprintf(stderr,"%s: processing file '%s'... ", PROGNAME, churner.in.name);
	  fflush(stderr);
	}
      }
      fprintf(out.file, "\n%%%% %s: File: %s\n\n", PROGNAME, churner.in.name);

      lexer.tokenize_stream(churner.in.file, out.file);

      if (args.verbose_arg > 1) {
	fprintf(stderr," done.\n");
	fflush(stderr);
      }
  }
  out.close();

  // -- summary
  if (args.verbose_arg > 0) {
      // -- timing
      gettimeofday(&stopped, NULL);
      elapsed = stopped.tv_sec-started.tv_sec + (stopped.tv_usec-started.tv_usec)/1000000.0;

      // -- print summary
      fprintf(stderr, "\n-----------------------------------------------------\n");
      fprintf(stderr, "%s Summary:\n", PROGNAME);
      fprintf(stderr, "  + Files processed : %d\n", nfiles);
      fprintf(stderr, "  + Tokens found    : %d\n", lexer.ntokens);
      fprintf(stderr, "  + Time Elsapsed   : %.2f sec\n", elapsed);
      fprintf(stderr, "  + Throughput      : %.2f toks/sec\n", (float)lexer.ntokens/elapsed);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}

