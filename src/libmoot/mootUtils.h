/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootUtils.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : useful utilities
 *--------------------------------------------------------------------------*/

#ifndef _moot_UTILS_H
#define _moot_UTILS_H

#include <stdio.h>
#include <string>

namespace moot {
  using namespace std;

/** Check whether a file exists by trying to open it with 'fopen()' */
bool file_exists(const char *filename);

/** Get basename of a file */
string unextend(const char *filename);

/**
 * Parse a comma-separated list of doubles (at most 'ndbls') from str into dbls.
 * You should already have allocated 'dbls'.
 */
bool moot_parse_doubles(char *str, double *dbls, size_t ndbls);

  /**
   * Utility for mootHMM::load_model() and friends: parse a model name according
   * to the conventions described in mootfiles(5).
   * @param modelname name of the model
   * @param binfile output string for binary model filename
   * @param lexfile output string for lexical frequency text-format filename
   * @param ngfile output string for n-gram frequency text-format filename
   * @param lcfile output string for class frequency text-format filename
   */
  bool hmm_parse_model_name(const string &modelname,
			    string &binfile,
			    string &lexfile,
			    string &ngfile,
			    string &lcfile);

  /**
   * Utility for mootHMM::load_model() and friends: parse a text-model name according
   * to the conventions described in mootfiles(5).
   * @param modelname name of the model
   * @param lexfile output string for lexical frequency text-format filename
   * @param ngfile output string for n-gram frequency text-format filename
   * @param lcfile output string for class frequency text-format filename
   */
  bool hmm_parse_model_name_text(const string &modelname,
				 string &lexfile,
				 string &ngfile,
				 string &lcfile);


/**
 * Abstract file-information class, because it's handy to
 * keep filenames and the actual stream pointers in the same place.
 */
class cmdutil_file_info {
public:
  //-- data
  char *name;  /**< filename: may also be "<stdin>", "<stdout>", or "<stderr>" */
  FILE *file;  /**< actual file pointer: we use C-streams because they're cooler */

  // -- methods: constructor/destructor
  /** Default constructor */
  cmdutil_file_info(char *myname=NULL, FILE *myfile=NULL);

  /** Destructor: closes file if it's open */
  ~cmdutil_file_info();

  /** (re-)open the current file.  Really just an alias for fopen() */
  FILE *open(const char *mode="r");

  /** (re-)close the current file.  Really just an alias for fclose() */
  int close(void);
};


/**
 * Class for churning through many input files, given either
 * directly or as a list-file.
 */
class cmdutil_file_churner {
public:
  // -- command-line data
  char  *progname;  /**< Name of the running program (for error reporting) */
  char **inputs;    /**< Input files/file-lists to be chruned (i.e. argv) */
  int    ninputs;   /**< Number of inputs given (i.e. argc) */

  // -- operation flags
  bool use_list;    /**< Whether inputs are filenames or list-filenames */

  // -- file data
  cmdutil_file_info in;    /** Current real input file */
  cmdutil_file_info list;  /** Current list input file, if in list-mode */

private:
  // -- for file-lists
  char *linebuf;
  size_t lbsize;

public:
  /** Constructor */
  cmdutil_file_churner(char *my_progname=NULL, char **inputs=NULL, int ninputs=0, bool use_list=false);
  /** Destructor */
  ~cmdutil_file_churner();

  /** Step to the first input file; returns NULL if no input files were given */
  FILE *first_input_file();

  /** Step the the next input file: returns NULL if no input files are left */
  FILE *next_input_file();

private:
  /** Step the the next input file (list-mode): returns NULL if no input files are left */
  FILE *next_list_file();
};


}; /* namespace moot */

#endif /* _moot_UTILS_H */
