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
 * File: mootIO.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : low-level I/O abstractions
 *   + these are ugly, but there appears to be no better
 *     (read "faster in the general case") way to get
 *     C FILE*s to jive with C++ streams, and I REALLY
 *     like printf() and friends...
 *--------------------------------------------------------------------------*/

/**
\file mootIO.h
\brief generic I/O abstraction layer
*/

#ifndef _MOOT_IO_H
#define _MOOT_IO_H

#include <mootTypes.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/** \brief Namespace for I/O stream wrappers */
namespace mootio {

  /*====================================================================
   * mootio: types
   *====================================================================*/
  /// typedef for byte counts (should be signed, for compatibility)
  typedef int ByteCount;

  /// typedef for (byte) offsets (may be unsigned)
  typedef moot::OffsetT ByteOffset;

  /*====================================================================
   * mstream: base class
   *====================================================================*/
  /** \brief Abstract base class for I/O stream wrappers
   *
   * This looks at first like a very ugly hack (which it admittedly is),
   * since it reduces C++ iostreams to the low-level streambuf layer (which
   * is all anyone ever really needs anyways), and wraps native C I/O
   * calls in virtual methods (which are nasty icky OO and slow), so it
   * gets us essentially the worst of both worlds.  However:
   * \li we get to keep printf()
   * \li we get to use C++ streams, C FILE*s, zlib gzFile, and 
   *     C in-memory buffers (via mootio::CBuffer) using one single class,
   *     which is just Too Darned Handy for extentible I/O methods.
   * \li we get to keep printf()
   * \li Support for file descriptors (sockets) should work in both
   *     the C and C++ idioms.
   * \li we get to keep printf()
   * \li native FILE* / char* / iostream maniuplation can still
   *     be done by the user by accessing the relevant underlying
   *     pointers directly.
   * \li did I mention that we get to keep printf()?
   */
  class mstream {
  public:
    std::string name;  ///< symbolic name of this stream

  public:
    /*----------------------------------------------------------*/
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mstream(const std::string &myname="") : name(myname) {};

    /** Destructor */
    virtual ~mstream(void) {};
    //@}

    /*----------------------------------------------------------*/
    ///\name Integrity Tests
    //@{
    /** Test stream integrity */
    virtual bool valid(void) { return false; };

    /** Alias for valid() */
    inline operator bool(void) { return valid(); };

    /** Test for eof */
    virtual bool eof(void) { return true; };

    /** Get current error message, if any */
    virtual std::string errmsg(void) {
      return std::string(valid() ? "" : "Invalid stream");
    };
    //@}

    /*----------------------------------------------------------*/
    ///\name Open/Close
    //@{
    /** (Re-)open currently selected stream */
    virtual bool reopen(void) { return true; };

    /** Close currently selected stream */
    virtual bool close(void) { return true; };
    //@}
  }; //-- /mstream


  /*====================================================================
   * mistream
   *====================================================================*/
  /** \brief Abstract base class for input stream wrappers */
  class mistream : virtual public mstream {
  public:
    /*----------------------------------------------------------
     * mistream: constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mistream(void) : mstream() {};

    /** Destructor */
    virtual ~mistream(void) {};
    //@}

    /*----------------------------------------------------------
     * mistream: input
     */
    ///\name Input Methods
    //@{

    /** Read a single byte of data.  Returns EOF on eof. */
    virtual int getbyte(void) { return EOF; }

    /** Read up to \p n bytes of data into \p buf,
     *  returns number of bytes actually read. */
    virtual ByteCount read(char *buf, size_t n) {
      size_t nread = 0;
      int c;
      for (c = this->getbyte(); nread < n && c != EOF; nread++, c = this->getbyte()) {
	*buf++ = c;
      }
      return static_cast<ByteCount>(nread);
    };

    /** Read a single line of data into \p s, using delimiters from \p delim.
     *  Returns the number of bytes read, or \c EOF on eof
     *  The default implementation is quite inefficient.
     *  Implicitly clears \p s.
     */
    virtual ByteCount getline(std::string &s, const std::string &delim="\n\r") {
      ByteCount nread = 0;
      int c;
      s.clear();
      for (c = this->getbyte(); c != EOF; nread++, c = this->getbyte()) {
	s.push_back(static_cast<char>(c));
	if (delim.find(c) != delim.npos) break;
      }
      return nread ? nread : EOF;
    };
    //@}
  };


  /*====================================================================
   * mostream
   *====================================================================*/
  /** \brief Abstract base class for output stream wrappers */
  class mostream : virtual public mstream {
  public:
    /*----------------------------------------------------------
     * mostream: constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mostream(void) : mstream() {};

    /** Destructor */
    virtual ~mostream(void) {};
    //@}

    /*----------------------------------------------------------
     * mostream: output
     */
    ///\name Output methods
    //@{

    /** Flush all pending writes. */
    virtual bool flush(void) { return false; };

    /** Write \p n bytes from \p buf to the stream */
    virtual bool write(const char *buf, size_t n) { return false; };

    /** Write a single byte to the stream.
     *  Formerly known as 'putc()', but conflicts with macro on gcc-4.4
     *  (cf. http://odo.dwds.de/mantis/view.php?id=180)
     */
    virtual bool putbyte(unsigned char c) { return false; };

    /** Write a C string to the stream */
    virtual bool puts(const char *s) { return false; };

    /** Write a C++ string to the buffer */
    virtual bool puts(const std::string &s) { return false; };

    /** printf() to the stream, va_list version
     * Default implementation uses a dynamically allocated
     * temporary buffer and calls write()
     */
    virtual bool vprintf(const char *fmt, va_list &ap);

    /**
     * printf() to the stream, arglist version.  Default
     * version calls vprintf()
     */
    inline bool printf(const char *fmt, ...) {
      va_list ap;
      va_start(ap,fmt);
      bool rc = this->vprintf(fmt,ap);
      va_end(ap);
      return rc;
    };
    //@}
  };

}; //-- /namespace mootio


#endif //_MOOT_IO_H
