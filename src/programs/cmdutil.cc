/* -*- Mode: C++ -*- */
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
 * File: cmdutil.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + command-line parsing utilities
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cmdutil.h"

extern "C" int getline(char **, size_t *, FILE *);

/*----------------------------------------------------------------------
 * cmdutil_file_info
 *----------------------------------------------------------------------*/

// cmdutil_file_info::cmdutil_file_info(name,file);
cmdutil_file_info::cmdutil_file_info(char *myname, FILE *myfile)
{
  name = myname;
  file = myfile;
}

// cmdutil_file_info::~cmdutil_file_info();
cmdutil_file_info::~cmdutil_file_info()
{
  //if (name) free(name);
  name = NULL;
  file = NULL;
}


// FILE *cmdutil_file_info::open(mode="r")
FILE *cmdutil_file_info::open(const char *mode)
{
  if (file) close();
  if (!name) {
    errno = ENOSTR;
    return NULL;
  }
  if (strcmp(name,"<stdin>") == 0) {
    file = stdin;
  } else if (strcmp(name,"<stdout>") == 0) {
    file = stdout;
  } else if (strcmp(name,"<stderr>") == 0) {
    file = stderr;
  } else {
    // normal filename
    file = fopen(name, mode);
  }
  return file;
}

// int cmdutil_file_info::close()
int cmdutil_file_info::close() {
  int i;
  if (!file) {
    // allow multiple close()s on one file
    return 0;
  }
  else if (file == stdin || file == stdout || file == stderr) {
    // dont' close the standard streams
    file = NULL;
    return 0;
  }
  i = fclose(file);
  file = NULL;
  return i;
}


/*----------------------------------------------------------------------
 * cmdutil_file_churner
 *----------------------------------------------------------------------*/

// cmdutil_file_churner::cmdutil_file_churner(progname, inputs, ninputs, use_list);
cmdutil_file_churner::cmdutil_file_churner(char *my_progname,
					   char **my_inputs,
					   int  my_ninputs,
					   bool my_use_list)
{
  // -- public data
  progname = my_progname;
  inputs = my_inputs;
  ninputs = my_ninputs;
  use_list = my_use_list;

  // -- private data
  linebuf = (char *)malloc(CMDUTIL_CHURN_DEFAULT_BUFSIZE);
  lbsize = CMDUTIL_CHURN_DEFAULT_BUFSIZE-1;
}


// cmdutil_file_churner::~cmdutil_file_churner();
cmdutil_file_churner::~cmdutil_file_churner()
{
  // -- public data

  // -- private data
  if (linebuf) free(linebuf);
  linebuf = NULL;
}

/*
 * file = cmdutil_file_churner::first_input_file();
 *   + get first input file
 */
FILE *cmdutil_file_churner::first_input_file() {
  if (use_list) {
    // -- args/inputs are file-LISTS
    if (ninputs <= 0) {
      // -- file-list on stdin
      list.name = "<stdin>";
    } else {
      // -- file-list(s) given in inputs[]
      list.name = inputs[0];
    }
    // -- sanity check
    if (!list.open("r")) {
      fprintf(stderr,"%s: open failed for input list-file '%s': %s\n",
	      progname, list.name, strerror(errno));
      exit(1);
    }

    // -- read next input filename from list-file
    while (getline(&linebuf, &lbsize, list.file) == -1) {
      if (feof(list.file)) {
	if (!next_list_file()) return NULL;
      } else {
	fprintf(stderr, "%s: Error reading input-list-file '%s': %s\n",
		progname, list.name, strerror(errno));
	exit(1);
      }
    }

    in.name = linebuf;
    in.name[strlen(in.name)-1] = '\0';  // eliminate trailing newline
    // -- check for stdin
    if (strcmp(in.name, "-") == 0) in.name = "<stdin>";

    if (!in.open("r")) {
      fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
	      progname, in.name, strerror(errno));
      exit(1);
    }
    return in.file;
  }

  // -- args/inputs are file-NAMES
  if (ninputs <= 0) {
    // -- read from stdin
    in.name = "<stdin>";
  } else {
    // -- read from user-specified filenames
    in.name = *inputs;
    if (strcmp(in.name,"-") == 0) in.name = "<stdin>";
  }
  if (!in.open("r")) {
    fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
	    progname, in.name, strerror(errno));
    exit(1);
  }
  return in.file;
}


/*
 * in = cmdutil_file_churner::next_input_file();
 */
FILE *cmdutil_file_churner::next_input_file() {
  in.close();
  if (use_list) {
    // -- read next file
    while (getline(&linebuf, &lbsize, list.file) == -1) {
      if (feof(list.file)) {
	if (!next_list_file()) return NULL;
      } else {
	fprintf(stderr, "%s: Error reading input-list-file '%s': %s\n",
		progname, list.name, strerror(errno));
	exit(1);
      }
    }

    in.name = linebuf;
    in.name[strlen(in.name)-1] = '\0';  // eliminate trailing newline
    if (!in.open("r")) {
      fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
	      progname, in.name, strerror(errno));
      exit(1);
    }
    return in.file;
  }

  // -- args/inputs are file-NAMES
  if (--ninputs <= 0) { return NULL; }
  in.name = *(++inputs);
  if (strcmp(in.name,"-") == 0) in.name = "<stdin>";
  if (!in.open("r")) {
    fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
	    progname, in.name, strerror(errno));
    exit(1);
  }
  return in.file;
}


/*
 * list.file = next_list_file();
 */
FILE *cmdutil_file_churner::next_list_file() {
  if (!use_list) return NULL;
  list.close();
  if (--ninputs <= 0) return NULL;
  
  list.name = *(++inputs);
  
  if (!list.open("r")) {
    fprintf(stderr,"%s: open failed for input list-file '%s': %s\n",
	    progname, list.name, strerror(errno));
    exit(1);
  }
  return list.file;
}


/*--------------------------------------------------------------------
 * Global utility functions
 *---------------------------------------------------------------------*/

bool file_exists(const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}
