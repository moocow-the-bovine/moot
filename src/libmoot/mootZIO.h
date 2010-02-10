/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2004-2010 by Bryan Jurish <jurish@uni-potsdam.de>

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
 * File: mootZIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : low-level I/O abstractions for libz gzFile
 *--------------------------------------------------------------------------*/

/** \file mootZIO.h
 *  \brief ::mootio I/O abstraction layer for zlib \a gzFile
 */

#ifndef _MOOT_ZIO_H
#define _MOOT_ZIO_H

#include <mootIO.h>

#ifdef MOOT_ZLIB_ENABLED

#include <zlib.h>
#define MOOT_DEFAULT_COMPRESSION Z_DEFAULT_COMPRESSION

namespace mootio {

  /*====================================================================
   * mcstream : FILE* i/o
   *====================================================================*/
  /** \brief Wrapper class for C FILE* streams. */
  class mzstream
    : virtual public mistream,
      virtual public mostream
  {
  public:
    gzFile zfile;  ///< underlying gzFile
  public:
    /*----------------------------------------------------------
     * mzstream: constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mzstream(gzFile zf=NULL) : zfile(zf) {};

    /** Destructor */
    ~mzstream(void) {};
    //@}

    /*----------------------------------------------------------
     * mcstream: integrity
     */
    ///\name Integrity Tests
    //@{
    /** Test stream integrity */
    virtual bool valid(void) {
      if (!zfile) return false;
      int errnum;
      gzerror(zfile,&errnum);
      return errnum == Z_OK;
    };

    /** Test for eof */
    virtual bool eof(void) { return !zfile || gzeof(zfile); };

    /** Get current error message */
    virtual std::string errmsg(void) {
      int errnum = Z_ERRNO;
      const char *zerrmsg = zfile ? gzerror(zfile,&errnum) : "NULL gzFile";
      return std::string(errnum == Z_OK
			 ? ""
			 : (zerrmsg != NULL ? zerrmsg : "unspecified zlib error"));
    };
    //@}

    /*----------------------------------------------------------
     * mcstream: open/close
     */
    ///\name Open/Close
    //@{
    /** Does nothing */
    virtual bool reopen(void) { return true; };

    /** Just resets internal gzFile to NULL, does nothing else */
    virtual bool close(void) { zfile=NULL; return true; };
    //@}

    /*----------------------------------------------------------
     * mcstream: input
     */
    ///\name Input Methods
    //@{
    /** Read up to \p n bytes of data into \p buf,
     *  returns number of bytes actually read. */
    virtual ByteCount read(char *buf, size_t n) {
      return zfile ? gzread(zfile, buf, n) : 0;
    };

    /** Read a single byte of data.  Returns EOF on eof. */
    virtual int getbyte(void) {
      int c = zfile ? gzgetc(zfile) : -1;
      return c==-1 ? EOF : c;
    };
    //@}

    /*----------------------------------------------------------
     * mzstream: output
     */
    ///\name Output Methods
    //@{
    /** Flush all pending writes (degrades compression ratio). */
    virtual bool flush(void) {
      return zfile && gzflush(zfile, Z_SYNC_FLUSH) == Z_OK;
    };

    /** Write \p n bytes from \p buf to the stream */
    virtual bool write(const char *buf, size_t n) {
      return zfile ? (gzwrite(zfile,buf,n) == static_cast<int>(n)) : false;
    };

    /** Write a single byte to the stream */
    virtual bool putbyte(unsigned char c) {
      return zfile ? (gzputc(zfile,c) != -1) : false;
    };

    /** Write a C string to the stream */
    virtual bool puts(const char *s) {
      return zfile ? (gzputs(zfile,s) >= 0) : false;
    };
    /** Write a C++ string to the stream */
    virtual bool puts(const std::string &s) {
      return (zfile	
	      ? (gzwrite(zfile,s.data(),s.size()) == static_cast<int>(s.size()))
	      : false);
    };

    /** printf() to the stream, va_list version */
    /*
    virtual bool vprintf(const char *fmt, va_list &ap) {
      return zfile ? (vfprintf(file,fmt,ap) >= 0) : false;
    };
    */
    //@}
  };


  /*====================================================================
   * mzfstream : named file i/o
   *====================================================================*/
  /** \brief Wrapper class for named file i/o using gzFile
   *
   * Uses mstream::name as filename identifier.  The name "-"
   * corresponds to either stdin or stdout, depending on open mode.
   */
  class mzfstream : virtual public mzstream {
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
    mzfstream(void)
      : mode("rb"),
	default_mode("rb")
    {};

    /** Constructor given filename, mode, and compression level */
    mzfstream(const char *filename,
	      const char *open_mode=NULL)
      : default_mode("rb")
    {
      open(filename,open_mode);
    };

    /** Destructor: auto-closes file */
    virtual ~mzfstream(void) { close(); };
    //@}

    /*------------------------------------------------------------------
     * mzfstream: open()
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
	if (mode.find('w') != mode.npos) zfile = gzdopen(fileno(stdout), mode.c_str());
	else zfile = gzdopen(fileno(stdin), mode.c_str());
      } else {
	zfile = gzopen(name.c_str(), mode.c_str());
      }
      return valid();
    };

    /** Close currently open file, if any */
    virtual bool close(void) {
      if (!zfile) return true;
      bool rc = gzclose(zfile) == Z_OK;
      zfile = NULL;
      return rc;
    };
    //@}

    /*------------------------------------------------------------------
     * mzfstream: Utilties
     */
    ///\name Utilities
    //@{
    /** Set gzFile parameters */
    inline void setparams(int level=Z_DEFAULT_COMPRESSION,
			  int strategy=Z_DEFAULT_STRATEGY)
    {
      if (level != Z_DEFAULT_COMPRESSION
	  && (level > Z_BEST_COMPRESSION || level < Z_NO_COMPRESSION))
	{
	  level = Z_DEFAULT_COMPRESSION;
	}
      if (zfile) gzsetparams(zfile, level, strategy);
    };
    //@}

  }; //-- /mzfstream

  /*====================================================================
   * mizfstream : named file input
   *====================================================================*/
  /** \brief Wrapper class for named file input using gzFile
   *
   * Uses mstream::name as filename identifier.
   * The name "-" corresponds to stdin.  Default open-mode is "rb".
   */
  class mizfstream : virtual public mzfstream {
  public:
    /*------------------------------------------------------------------
     * mfstream: Constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor as for mcstream */
    mizfstream(void) {
      default_mode = "rb";
    };

    /** Constructor given filename and mode */
    mizfstream(const char *filename, const char *mode=NULL) 
    {
      default_mode = "rb";
      open(filename,mode);
    };

    /** Constructor given filename and mode as C++ strings */
    mizfstream(const std::string &filename, const std::string &mode="") 
    {
      default_mode = "rb";
      open(filename,mode);
    };

    /** Destructor: auto-closes file */
    virtual ~mizfstream(void) { close(); };
    //@}
  }; //-- /mizfstream

  /*====================================================================
   * mozfstream : named file output
   *====================================================================*/
  /** \brief Wrapper class for named file output using gzFile
   *
   * Uses mstream::name as filename identifier.
   * The name "-" corresponds to stdout.  Default open-mode is "wb".
   */
  class mozfstream : virtual public mzfstream {
  public:
    /*------------------------------------------------------------------
     * mozfstream: Constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mozfstream(void) {
      default_mode = "wb";
    };

    /** Constructor given filename and mode */
    mozfstream(const char *filename, const char *mode=NULL)  {
      default_mode = "wb";
      open(filename,mode);
    };

    /** Constructor given filename and mode as C++ strings */
    mozfstream(const std::string &filename, const std::string &mode="") {
      default_mode = "wb";
      open(filename,mode);
    };

    /** Destructor: auto-closes file */
    virtual ~mozfstream(void) { close(); };
    //@}
  }; //-- /mofstream

}; //-- /namespace mootio

#else  // _MOOT_ZLIB_ENABLED

#include <mootCIO.h>

#define MOOT_DEFAULT_COMPRESSION -1

namespace mootio {

  typedef mcstream   mzstream;
  typedef mfstream   mzfstream;
  typedef mifstream  mizfstream;
  typedef mofstream  mozfstream;

}; //-- /namespace mootio

#endif // _MOOT_ZLIB_ENABLED

namespace mootio {

  /*====================================================================
   * mootio : common typedef aliases
   *====================================================================*/
  typedef mzstream mizstream; ///< Alias for gzFile input stream wrappers
  typedef mzstream mozstream; ///< Alias for gzFile output stream wrappers

}; //-- /namespace mootio

#endif //_MOOT_ZIO_H
