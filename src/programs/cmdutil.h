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
 * File: cmdutil.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + command-line parsing utilities : headers
 *--------------------------------------------------------------------------*/

#ifndef _CMDUTIL_H
#define _CMDUTIL_H

#define CMDUTIL_CHURN_DEFAULT_BUFSIZE 2048

#include <stdio.h>


class cmdutil_file_info {
public:
  // -- data
  char *name;  // - filename: may also be "<stdin>", "<stdout>", or "<stderr>"
  FILE *file;

  // -- methods: constructor/destructor
  cmdutil_file_info(char *myname=NULL, FILE *myfile=NULL);
  ~cmdutil_file_info();

  // -- methods: open/close
  FILE *open(const char *mode="r");
  int close(void);
};



class cmdutil_file_churner {
public:
  // -- command-line data
  char  *progname;
  char **inputs;
  int    ninputs;

  // -- operation flags
  bool use_list;

  // -- file data
  cmdutil_file_info in;
  cmdutil_file_info list;

private:
  // -- for file-lists
  char *linebuf;
  size_t lbsize;

public:
  // -- constructor
  cmdutil_file_churner(char *my_progname=NULL, char **inputs=NULL, int ninputs=0, bool use_list=false);
  ~cmdutil_file_churner();
  // -- file-stepping
  FILE *first_input_file();
  FILE *next_input_file();

private:
  // -- list-file stepping
  FILE *next_list_file();
};


/*--------------------------------------------------------------------
 * Global utility functions
 *---------------------------------------------------------------------*/

/** Check whether a file exists by trying to open it with 'fopen()' */
bool file_exists(const char *filename);

#endif // _CMDUTIL_H
