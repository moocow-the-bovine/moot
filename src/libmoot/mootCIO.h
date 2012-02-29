/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2004-2010 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootCIO.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : low-level I/O abstractions for C FILE *s
 *   + these are ugly, but there appears to be no better
 *     (read "faster in the general case") way to get
 *     C FILE*s to jive with C++ streams, and I REALLY
 *     like printf() and friends...
 *--------------------------------------------------------------------------*/

/** \file mootCIO.h
 *  \brief ::mootio abstraction layer for C \c FILE*s
 *  \details
 *     these are ugly, but there appears to be no better
 *     (read "faster in the general case") way to get
 *     C \c FILE*s to jive with C++ streams, and I \b REALLY
 *     like printf() and friends...
 */

#ifndef _MOOT_CIO_H
#define _MOOT_CIO_H

#include <mootIO.h>
/*#include <stdio.h>*/ //-- included by mootIO.h

namespace mootio {

  /*====================================================================
   * mcstream : FILE* i/o
   *====================================================================*/
  /** \brief Wrapper class for C FILE* streams. */
  class mcstream
    : public mistream,
      public mostream
  {
  public:
    FILE *file;  ///< underlying FILE*
  public:
    /*----------------------------------------------------------
     * mcstream: constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mcstream(FILE *f=NULL) : file(f) {};

    /** Destructor */
    ~mcstream(void) {};
    //@}

    /*----------------------------------------------------------
     * mcstream: integrity
     */
    ///\name Integrity Tests
    //@{
    /** Test stream integrity */
    virtual bool valid(void) { return file && !ferror(file); };

    /** Test for eof */
    virtual bool eof(void) { return !file || feof(file); };

    /** Get current error message */
    virtual std::string errmsg(void) {
      return std::string(file && !ferror(file) ? "" : strerror(errno));
    };
    //@}

    /*----------------------------------------------------------
     * mcstream: open/close
     */
    ///\name Open/Close
    //@{
    /** Does nothing */
    virtual bool reopen(void) { return true; };

    /** Just resets internal FILE* to NULL, does nothing else */
    virtual bool close(void) { file=NULL; return true; };
    //@}

    /*----------------------------------------------------------
     * mcstream: input
     */
    ///\name Input Methods
    //@{
    /** Read up to \p n bytes of data into \p buf,
     *  returns number of bytes actually read. */
    virtual ByteCount read(char *buf, size_t n) {
      return file ? fread(buf, 1, n, file) : 0;
    };

    /** Read a single byte of data.  Returns EOF on eof. */
    virtual int getbyte(void) {
      return file ? fgetc(file) : EOF;
    };
    //@}

    /*----------------------------------------------------------
     * mcstream: output
     */
    ///\name Output Methods
    //@{
    /** Flush all pending writes. */
    virtual bool flush(void) { return file && fflush(file) != EOF; };

    /** Write \p n bytes from \p buf to the stream */
    virtual bool write(const char *buf, size_t n) {
      return file ? (fwrite(buf,1,n,file) == n) : false;
    };

    /** Write a single byte to the stream */
    virtual bool putbyte(unsigned char c) {
      return file ? (fputc(c,file) != EOF) : false;
    };

    /** Write a C string to the stream */
    virtual bool puts(const char *s) {
      return file ? (fputs(s,file) >= 0) : false;
    };
    /** Write a C++ string to the stream */
    virtual bool puts(const std::string &s) {
      return file ? (fwrite(s.data(),1,s.size(),file) == s.size()) : false;
    };

    /** printf() to the stream, va_list version
     * (the real thing) */
    virtual bool vprintf(const char *fmt, va_list &ap) {
      return file ? (vfprintf(file,fmt,ap) >= 0) : false;
    };
    //@}
  };


  /*====================================================================
   * mfstream : named file i/o
   *====================================================================*/
  /** \brief Wrapper class for named file i/o using C FILE*s.
   *
   * Uses mstream::name as filename identifier.  The name "-"
   * corresponds to either stdin or stdout, depending on open mode.
   */
  class mfstream : public mcstream {
  public:
    std::string mode;          ///< open mode
    std::string default_mode;  ///< default open mode
  public:
    /*------------------------------------------------------------------
     * mfstream: Constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor as for mcstream */
    mfstream(void)
      : mode("r"),
	default_mode("r")
    {};

    /** Constructor given filename and mode */
    mfstream(const char *filename, const char *open_mode=NULL)
      : default_mode("r")
    {
      open(filename,open_mode);
    };

    /** Destructor: auto-closes file */
    virtual ~mfstream(void) { close(); };
    //@}

    /*------------------------------------------------------------------
     * mfstream: open()
     */
    ///\name Open / Close
    //@{
    /** Open a named file, fopen() style.
     *  "-" may be use to specify stdin or stdout, depending on 'mode' */
    inline bool open(const char *filename, const char *open_mode=NULL) {
      name = filename;
      if (open_mode) mode = open_mode;
      return reopen();
    };
    /** Open a named file, fopen() style.
     *  "-" may be use to specify stdin or stdout, depending on \p open_mode */
    inline bool open(const std::string &filename, const std::string &open_mode="") {
      mode = open_mode;
      name = filename;
      return reopen();
    };

    /** (Re-)open file \a name with mode \a mode */
    virtual bool reopen(void) {
      close();
      if (mode.empty()) mode = default_mode;
      //-- check for standard stream aliases
      if (name == "-") {
	if (mode.find('w') != mode.npos) file = stdout;
	else file = stdin;
      } else {
	file = fopen(name.c_str(), mode.c_str());
      }
      return file && !ferror(file);
    };

    /** Close currently open file, if any */
    virtual bool close(void) {
      if (!file) return true;
      else if (file==stdin || file==stdout || file==stderr) {
	//-- don't close standard streams
	file = NULL;
	return true;
      }
      bool rc = fclose(file) == 0;
      file = NULL;
      return rc;
    };
    //@}

    /*------------------------------------------------------------------
     * mfstream: params
     */
    ///\name Parameter setting (does nothing)
    //@{
    /** Does nothing.  For compatibility with (possibly missing) zlib */
    inline void setparams(int level=0, int strategy=0) {};
    //@}
  }; //-- /mfstream

  /*====================================================================
   * mifstream : named file input
   *====================================================================*/
  /** \brief Wrapper class for named file input using C FILE*s.
   *
   * Uses mstream::name as filename identifier.
   * The name "-" corresponds to stdin.  Default open-mode is "r".
   */
  class mifstream : public mfstream {
  public:
    /*------------------------------------------------------------------
     * mifstream: Constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor as for mcstream */
    mifstream(void) {
      default_mode = "r";
    };

    /** Constructor given filename and mode */
    mifstream(const char *filename, const char *mode=NULL) 
    {
      default_mode = "r";
      open(filename,mode);
    };

    /** Constructor given filename and mode as C++ strings */
    mifstream(const std::string &filename, const std::string &mode="") 
    {
      default_mode = "r";
      open(filename,mode);
    };

    /** Destructor: auto-closes file */
    virtual ~mifstream(void) { close(); };
    //@}
  }; //-- /mifstream

  /*====================================================================
   * mofstream : named file output
   *====================================================================*/
  /** \brief Wrapper class for named file output using C FILE*s.
   *
   * Uses mstream::name as filename identifier.
   * The name "-" corresponds to stdout.  Default open-mode is "w".
   */
  class mofstream : public mfstream {
  public:
    /*------------------------------------------------------------------
     * mofstream: Constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mofstream(void) {
      default_mode = "w";
    };

    /** Constructor given filename and mode */
    mofstream(const char *filename, const char *mode=NULL)  {
      default_mode = "w";
      open(filename,mode);
    };

    /** Constructor given filename and mode as C++ strings */
    mofstream(const std::string &filename, const std::string &mode="") {
      default_mode = "w";
      open(filename,mode);
    };

    /** Destructor: auto-closes file */
    virtual ~mofstream(void) { close(); };
    //@}
  }; //-- /mofstream


  /*====================================================================
   * mootio : typedef aliases
   *====================================================================*/
  typedef mcstream micstream; ///< Alias for FILE* input stream wrappers
  typedef mcstream mocstream; ///< Alias for FILE* output stream wrappers

}; //-- /namespace mootio


#endif //_MOOT_CIO_H
