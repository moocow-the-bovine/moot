/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2012 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootUtils.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : useful utilities
 *--------------------------------------------------------------------------*/

/** @file mootUtils.h
 *  @brief useful utilities, especially for command-line programs
 */

#ifndef _moot_UTILS_H
#define _moot_UTILS_H

#include <mootCIO.h>

#include <list>
#include <vector>

namespace moot {
  using namespace std;
  using namespace mootio;

  /*----------------------------------------------------------------------*/
  /** \name Locale Utilities */
  //@{
  /** initialize the current locale from the environment */
  void moot_setlocale(void);

  /** get current value of LC_CTYPE, or the string "(unavailable)" */
  const char *moot_lc_ctype(void);
  //@}


  /*----------------------------------------------------------------------*/
  /** \name String Utilities */
  //@{
  
  /**
   * Parse a comma-separated list of doubles (at most 'ndbls') from str into dbls.
   * You should already have allocated space for \c ndbls doubles in \c dbls.
   */
  bool moot_parse_doubles(char *str, double *dbls, size_t ndbls);
  
  /*----------------------------------------------------------------------
   * normalize_ws_*()
   */
  /**
   * Append a whitespace-normalized C buffer to an STL string.
   * All whitespace substrings in \c s are replaced with a
   * single space in \c out.  \c out is not cleared.
   *
   * \@param buf source buffer
   * \@param len length of source buffer in bytes
   * \@param out destination STL string
   * \@param trim_left whether to trim all leading whitespace
   * \@param trim_right whether to trim all trailing whitespace
   */
  void moot_normalize_ws(const char *buf,
			 size_t len,
			 std::string &out,
			 bool trim_left=true,
			 bool trim_right=true);

  /**
   * Append a whitespace-normalized C++ string to another C++ string.
   * All whitespace substrings in \p in are replaced with a
   * single space in \p out.  \p out is not cleared.
   *
   * @param in source string
   * @param out destination string
   * @param trim_left whether to trim all leading whitespace
   * @param trim_right whether to trim all trailing whitespace
   */
  void moot_normalize_ws(const std::string &in,
			 std::string &out,
			 bool trim_left=true,
			 bool trim_right=true);

  /**
   * Append a whitespace-normalized NUL-terminated C string
   * to an STL string.
   *
   * @param s source string
   * @param out destination STL string
   * @param trim_left whether to trim all leading whitespace
   * @param trim_right whether to trim all trailing whitespace
   */
  inline void moot_normalize_ws(const char *s,
				std::string &out,
				bool trim_left=true,
				bool trim_right=true)
  {
    moot_normalize_ws(s, strlen(s), out, trim_left, trim_right);
  };

  /**
   * Create and return a whitespace-normalized STL string
   * from a C memory buffer.
   *
   * @param buf source buffer
   * @param len length of source buffer, in bytes
   * @param trim_left whether to trim all leading whitespace
   * @param trim_right whether to trim all trailing whitespace
   */
  inline std::string moot_normalize_ws(const char *buf,
				       size_t len,
				       bool trim_left=true,
				       bool trim_right=true)
  {
    std::string out;
    out.reserve(len);
    moot_normalize_ws(buf,len, out, trim_left,trim_right);
    return out;
  };

  /**
   * Create and return a whitespace-normalized STL string
   * from a NUL-terminated C string.
   *
   * @param s source string
   * @param trim_left whether to trim all leading whitespace
   * @param trim_right whether to trim all trailing whitespace
   */
  inline std::string moot_normalize_ws(const char *s,
				       bool trim_left=true,
				       bool trim_right=true)
  {
    return moot_normalize_ws(s,strlen(s), trim_left,trim_right);
  };

  /**
   * Create and return a whitespace-normalized STL string
   * from a different STL string.
   *
   * @param s source string
   * @param trim_left whether to trim all leading whitespace
   * @param trim_right whether to trim all trailing whitespace
   */
  inline std::string moot_normalize_ws(const std::string &s,
				       bool trim_left=true,
				       bool trim_right=true)
  {
    return moot_normalize_ws(s.data(),s.size(), trim_left,trim_right);
  };

  /*----------------------------------------------------------------------
   * remove_newlines()
   */
  /**
   * Remove all newlines from a C buffer.
   * Every newline is replaced with a single space.
   *
   * @param buf target buffer
   * @param len length of target buffer in bytes
   */
  void moot_remove_newlines(char *buf, size_t len);

  /** Remove all newlines from a NUL-terminated C string. */
  inline void moot_remove_newlines(char *s)
  { moot_remove_newlines(s, strlen(s)); };

  /** Remove all newlines from an STL string. */
  void moot_remove_newlines(std::string &s);

  /** Tokenize an STL string to an existing list.
   *  Multiple adjacent delimiters are treated as a single delimiter;
   *  i.e. no empty strings are returned.
   *
   * @param s source string
   * @param delim string of delimiter characters
   * @param out destination string list
   */
  void moot_strtok(const std::string &s, const std::string &delim, std::list<std::string> &out);

  /** Tokenize an STL string to a new list.
   *
   * @param s source string
   * @param delim string of delimiter characters
   */
  std::list<std::string> moot_strtok(const std::string &s, const std::string &delim);

  /** Split an STL string to an existing list.
   *  All delimiters are significant, i.e. empty output strings are allowed.
   *
   * @param s source string
   * @param delim string of delimiter characters
   * @param out destination string list
   */
  void moot_strsplit(const std::string &s, const std::string &delim, std::vector<std::string> &out);

  /** Tokenize an STL string to a new list.
   *
   * @param s source string
   * @param delim string of delimiter characters
   */
  std::vector<std::string> moot_strsplit(const std::string &s, const std::string &delim);


  /** Stupid wrapper for append+printf() onto C++ strings.
   *
   * @param s sink string
   * @param fmt printf format
   * @param ap printf args
   */
  int std_vsprintf(std::string &s, const char *fmt, va_list &ap);

  /** Stupid wrapper for append+printf() onto C++ strings.
   *
   * @param s sink string
   * @param fmt printf format
   * @param ap printf args
   */
  int std_sprintf(std::string &s, const char *fmt, ...);

  /** Stupid wrapper for printf() returning a C++ string
   *
   * @param fmt printf format
   * @param ap printf args
   */
  std::string std_vssprintf(const char *fmt, va_list &ap);

  /** Stupid wrapper for printf() returning a C++ string
   *
   * @param fmt printf format
   * @param ap printf args
   */
  std::string std_ssprintf(const char *fmt, ...);
  //@}

  /*----------------------------------------------------------------------*/
  /** \name Named File Utilities */
  //@{
  
  /** Check whether a file exists by trying to open it with 'fopen()' */
  bool moot_file_exists(const char *filename);

  /** Check whether a file exists by trying to open it with 'fopen()', std::string version */
  inline bool moot_file_exists(const std::string &filename)
  { return moot_file_exists(filename.c_str()); };
  
  /** Get path+basename of a file */
  std::string moot_unextend(const char *filename);

  /**
   * Get final extension of a filename (including leading '.'),
   * reading backwards from (filename+pos).  Returns a pointer
   * into \c filename.
   * If no next extension is found, returns NULL.
   */
  const char *moot_extension(const char *filename, size_t pos);

  /** Get extension of a filename (including leading '.') */
  inline const char *moot_extension(const char *filename)
  {
    return moot_extension(filename, strlen(filename));
  };


  /**
   * \brief Class for churning through many input files, given either
   *        directly or as a list-file.
   */
  class cmdutil_file_churner {
  public:
    // -- command-line data
    const char   *progname;  /**< Name of the running program (for error reporting) */
    char        **inputs;    /**< Input files/file-lists to be chruned (i.e. argv) */
    int           ninputs;   /**< Number of inputs given (i.e. argc) */

    // -- operation flags
    bool use_list;        /**< Whether inputs are filenames or list-filenames */
    bool paranoid;        /**< Whether to abort() for unreadable files */

    // -- file data
    mifstream           in;    /**< Current real input file, wrapped */
    mifstream           list;  /**< Current list input file, if in list-mode */

    // -- buffer data
    std::string         line; /**< Input line buffer */

  private:
    bool is_first_input;      /**< true until next_input_name() has been called at least once */

  public:
    /** Constructor */
    cmdutil_file_churner(const char *my_progname=NULL,
			 char **my_inputs=NULL,
			 int  my_ninputs=0,
			 bool my_use_list=false,
			 bool my_paranoid=true)
      : progname(my_progname),
	inputs(my_inputs),
	ninputs(my_ninputs),
	use_list(my_use_list),
	paranoid(my_paranoid),
	is_first_input(true)
    {};

    /** Destructor */
    ~cmdutil_file_churner() {};

    /** Step to the first (valid) input file; returns NULL if no input files were given
     *  : OBSOLETE: use next_input_file() instead.
     */
    FILE *first_input_file();

    /** Step to the first (valid) input file, without opening it
     *  : OBSOLETE: use next_input_name() instead.
     */
    std::string &first_input_name();

    /** Step the the next input file: returns NULL if no input files are left */
    FILE *next_input_file();

    /** Step the the next input file, without opening it */
    std::string &next_input_name();

  private:
    /** Step the the next input-list filename (list mode): returns NULL if no input files are left */
    FILE *next_list_file();
  };
  //@}

  /*----------------------------------------------------------------------*/
  /** \name Message and Command-line utilities */
  //@{

  /** Return a banner string for the library */
  std::string moot_banner(void);

  /** Return a full banner string for a program using the library. */
  std::string moot_program_banner(const std::string &prog_name,
				  const std::string &prog_version,
				  const std::string &prog_author,
				  bool is_free=true);

  /** verbose message to stderr, va_list version */
  void moot_vcarp(const char *fmt, va_list &ap);

  /** verbose message to stderr */
  void moot_carp(const char *fmt, ...);

  /** verbose message to stderr followed by abort(), va_list version */
  void moot_vcroak(const char *fmt, ...);

  /** verbose message to stderr followed by abort() */
  void moot_croak(const char *fmt, ...);

  /** conditional message to stderr (prints only if curLevel>=minLevel), va_list version
   *  @param curLevel current verbosity level
   *  @param minLevel minimum level for print
   *  @param fmt printf format
   *  @param ... printf arguments
   */
  void moot_vmsg(int curLevel, int minLevel, const char *fmt, va_list &ap);

  /** conditional message to stderr (prints only if curLevel>=minLevel), varargs version
   *  @param curLevel current verbosity level
   *  @param minLevel minimum level for print
   *  @param fmt printf format
   *  @param ... printf arguments
   */
  void moot_msg(int curLevel, int minLevel, const char *fmt, ...);
  //@}

}; /* namespace moot */

#endif /* _moot_UTILS_H */
