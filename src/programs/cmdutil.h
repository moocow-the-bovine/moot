/* -*- Mode: C++ -*- */
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
