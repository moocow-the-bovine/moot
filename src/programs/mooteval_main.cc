/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mooteval_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's tagger: evaluator: main()
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <mootTokenIO.h>
#include "mooteval_cmdparser.h"
#include "cmdutil.h"

using namespace std;
using namespace moot;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "mooteval";

// files
cmdutil_file_info out;

// options & file-churning
gengetopt_args_info args;

//-- token reader
TokenReader treader(false,false);

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
}

/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GetMyOptions(argc,argv);

  //-- do something here!

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

