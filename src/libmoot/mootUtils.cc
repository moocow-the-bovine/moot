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
 * File: mootUtils.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : useful utilities
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <mootUtils.h>
#include <mootIO.h>
#include <mootCIO.h>

namespace moot {
  using namespace std;

/*----------------------------------------------------------------------
 * String Utilities
 *---------------------------------------------------------------------*/

/*----------------------------------------------------------------------
 * String Utilities: parse_doubles()
 */
bool moot_parse_doubles(char *str, double *dbls, size_t ndbls)
{
  size_t i;
  char  *s, *comma;
  if (!str) return false;
  for (i = 0, s = str; i < ndbls; i++, s = comma+1) {
    dbls[i] = strtod(s, &comma);
    comma = strchr(comma, ',');
    if (!comma) break;
  }
  return true;
}


/*----------------------------------------------------------------------
 * String Utilities: normalize_ws()
 */
void moot_normalize_ws(const char *buf,
		       size_t len,
		       std::string &out,
		       bool trim_left,
		       bool trim_right)
{
  size_t i, j;     //-- begin-, end-markers for relevant substrings: buf[i..j(
  char sp = ' ';
  out.reserve(out.size()+len);
  for (i = 0; i < len; i = j) {
    if (isspace(buf[i])) {
      //-- whitespace substring: (maybe) append single space
      for (j = i+1; j < len && isspace(buf[j]); j++) ;
      if ((i != 0 || !trim_left) && (j != len || !trim_right)) {
	out.push_back(sp); //-- what the?
      }
    } else {
      //-- Non-whitespace substring: append substring in one swell foop
      for (j = i+1; j < len && !isspace(buf[j]); j++) ;
      out.append(buf+i, j-i);
    }
  }
}

/*----------------------------------------------------------------------
 * String Utilities: normalize_ws()
 */
void moot_normalize_ws(const std::string &in,
		       std::string &out,
		       bool trim_left,
		       bool trim_right)
{
  //-- begin-, end-markers for relevant substrings: in[i..j(
  string::const_iterator i,j;
  for (i = in.begin(); i < in.end(); i++) {
    if (isspace(*i)) {
      //-- whitespace substring: (maybe) append single space
      for (j = i+1; j != in.end() && isspace(*j); j++) ;
      if ((i != in.begin()    || !trim_left)
	  && (j != in.end()-1 || !trim_right)) {
	out.push_back(' ');
      }
    } else {
      //-- Non-whitespace substring: append substring in one swell foop
      for (j = i+1; j != in.end() && !isspace(*j); j++) ;
      out.append(i,j);
    }
  }
}

/*----------------------------------------------------------------------
 * String Utilities: remove_newlines(): C
 */
//(inlined)

/*----------------------------------------------------------------------
 * String Utilities: remove_newlines(): STL
 */
//(inlined)  

/*----------------------------------------------------------------------
 * String Utilities: moot_strtok()
 */
void moot_strtok(const std::string &s,
		 const std::string &delim,
		 std::list<std::string> &out)
{
  out.clear();
  size_t beg, end;
  for (beg = s.find_first_not_of(delim,0)   , end = s.find_first_of(delim,beg);
       beg < s.size();
       beg = s.find_first_not_of(delim,end) , end = s.find_first_of(delim,beg))
    {
      out.push_back(string());
      out.back().assign(s,beg,end-beg);
    }
}


/*--------------------------------------------------------------------
 * Named File Utilities
 *---------------------------------------------------------------------*/

/*------------------------------------------------------
 * Named File Utilities : moot_file_exists
 */
bool moot_file_exists(const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}

/*------------------------------------------------------
 * Named File Utilities : moot_unextend
 */
std::string moot_unextend(const char *filename)
{
  string fn = filename;
  fn.erase(fn.rfind('.'));
  return fn;
}

/*------------------------------------------------------
 * Named File Utilities : extension
 */
//(inlined)

/*--------------------------------------------------------------------------
 * Named File utilities: parse_model_name()
 */
bool hmm_parse_model_name(const string &modelname,
			  string &binfile,
			  string &lexfile,
			  string &ngfile,
			  string &lcfile)
{
  if (moot_file_exists(modelname.c_str())) {
    binfile = modelname;
    lexfile.clear();
    ngfile.clear();
    lcfile.clear();
  }
  else {
    binfile.clear();
    return hmm_parse_model_name_text(modelname,lexfile,ngfile,lcfile);
  }

  return !(binfile.empty() && lexfile.empty() && ngfile.empty() && lcfile.empty());
}

/*--------------------------------------------------------------------------
 * Named File utilities: parse_model_name_text()
 */
bool hmm_parse_model_name_text(const string &modelname,
			       string &lexfile,
			       string &ngfile,
			       string &lcfile)
{
  size_t icomma1 = modelname.find(',',0);

  //-- lexfile
  if (icomma1 != modelname.npos) {
    lexfile.assign(modelname,0,icomma1);

    //-- ngfile
    size_t icomma2 = modelname.find(',',icomma1+1);
    if (icomma2 != modelname.npos) {
      ngfile.assign(modelname,icomma1+1,icomma2-icomma1-1);

      //-- lcfile
      lcfile.assign(modelname,icomma2+1,modelname.size()-(icomma2+1));
    }
    else {
      ngfile.assign(modelname,icomma1+1,modelname.size()-(icomma1+1));
      lcfile.clear();
    }
  }
  else {
    //-- no commas: expand modelname as basename
    lexfile = modelname;
    lexfile.append(".lex");
    //if (!moot_file_exists(lexfile.c_str())) lexfile.clear();  //-- not here: (used for training!)

    ngfile = modelname;
    ngfile.append(".123");
    //if (!moot_file_exists(ngfile.c_str())) ngfile.clear(); //-- not here: (used for training!)

    lcfile = modelname;
    lcfile.append(".clx");
    //if (!moot_file_exists(lcfile.c_str())) lcfile.clear(); //-- not here: (used for training!)
  }

  return !(lexfile.empty() && ngfile.empty() && lcfile.empty());
}

/*----------------------------------------------------------------------
 * cmdutil_file_churner
 *----------------------------------------------------------------------*/


/*
 * file = cmdutil_file_churner::first_input_file();
 *   + get first input file
 */
FILE *cmdutil_file_churner::first_input_file() {
  if (first_input_name().empty()) return NULL;
  if (!in.reopen()) {
    fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
	    progname, in.name.c_str(), strerror(errno));
    abort();
  }
  return in.file;
}

/*
 * in = cmdutil_file_churner::next_input_file();
 */
FILE *cmdutil_file_churner::next_input_file() {
  if (next_input_name().empty()) return NULL;
  if (!in.reopen()) {
    fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
	    progname, in.name.c_str(), strerror(errno));
    abort();
  }
  return in.file;
}

/*
 * name = cmdutil_file_churner::first_input_name();
 *   + get first input name
 */
std::string &cmdutil_file_churner::first_input_name() {
  if (use_list) {
    // -- args/inputs are file-LISTS
    if (ninputs <= 0) {
      // -- file-list on stdin
      list.name = "-";
    } else {
      // -- file-list(s) given in inputs[]
      list.name = inputs[0];
    }
    // -- sanity check
    if (!list.reopen()) {
      fprintf(stderr,"%s: open failed for input list-file '%s': %s\n",
	      progname, list.name.c_str(), strerror(errno));
      abort();
    }

    // -- read next input filename from list-file
    while (list.getline(line) == EOF) { //-- no more data left
      if (list.eof()) {
	if (!next_list_file()) return in.name = "";
      } else {
	fprintf(stderr, "%s: Error reading input-list-file '%s': %s\n",
		progname, list.name.c_str(), strerror(errno));
	abort();
      }
    }
    in.name = line;
    in.name.erase(in.name.size()-1);  // eliminate trailing newline
    return in.name;
  }

  // -- args/inputs are file-NAMES
  if (ninputs <= 0) {
    // -- read from stdin
    in.name = "-";
  } else {
    // -- read from user-specified filenames
    in.name = *inputs;
  }
  return in.name;
}


/*
 * in = cmdutil_file_churner::next_input_name();
 */
std::string &cmdutil_file_churner::next_input_name() {
  in.close();
  if (use_list) {
    // -- read next file
    while (list.getline(line) == EOF) { //-- no more data left
      if (list.eof()) {
	if (!next_list_file()) return in.name = "";
      } else {
	fprintf(stderr, "%s: Error reading input-list-file '%s': %s\n",
		progname, list.name.c_str(), strerror(errno));
	abort();
      }
    }
    in.name = line;
    in.name.erase(in.name.size()-1);  // eliminate trailing newline
    return in.name;
  }

  //-- args/inputs are file-NAMES
  if (--ninputs <= 0) { return in.name = ""; }
  in.name = *(++inputs);
  return in.name;
}


/*
 * list.file = next_list_file();
 */
FILE *cmdutil_file_churner::next_list_file() {
  if (!use_list) return NULL;
  list.close();
  if (--ninputs <= 0) return NULL;
  
  list.name = *(++inputs);
  
  if (!list.reopen()) {
    fprintf(stderr,"%s: open failed for input list-file '%s': %s\n",
	    progname, list.name.c_str(), strerror(errno));
    abort();
  }
  return list.file;
}

}; //-- namespace moot

