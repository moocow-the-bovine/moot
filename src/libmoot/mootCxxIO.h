/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2004-2010 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootCxxIO.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : low-level I/O abstractions for C++ streams
 *   + these are ugly, but there appears to be no better
 *     (read "faster in the general case") way to get
 *     C FILE*s to jive with C++ streams, and I REALLY
 *     like printf() and friends...
 *--------------------------------------------------------------------------*/

/**
\file mootCxxIO.h
\brief ::mootio abstraction layer for C++ streams
*/

#ifndef _MOOT_CXXIO_H
#define _MOOT_CXXIO_H

#include <iostream>
#include <fstream>

#include <mootIO.h>
#include <mootArgs.h>

namespace mootio {
  using namespace std;

  /*====================================================================
   * mcxxstream : std::(i|o)stream i/o
   *====================================================================*/
  template <class _StreamClass>
  class mcxxstream : virtual public mstream {
  public:
    _StreamClass *streamp;  ///< pointer to underlying stream
  public:
    /*----------------------------------------------------------
     * mcxxstream: constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mcxxstream(_StreamClass *streamptr=NULL) : streamp(streamptr) {};

    /** Reference-based constructor */
    mcxxstream(_StreamClass &stream) : streamp(&stream) {};

    /** Destructor */
    virtual ~mcxxstream(void) {};
    //@}

    /*----------------------------------------------------------
     * mcxxstream: integrity
     */
    ///\name Integrity Tests
    //@{
    /** Test stream integrity */
    virtual bool valid(void) { return streamp && streamp->good(); };

    /** Test for eof */
    virtual bool eof(void) { return !streamp || streamp->eof(); };
    //@}
  }; //-- /mcxxstream


  /*====================================================================
   * micxxstream : std::istream input
   *====================================================================*/
  /** Wrapper for C++ input streams (std::istream) */
  class micxxstream
    : public mcxxstream<std::istream>,
      public mistream
  {
  public:
    /*----------------------------------------------------------
     * micxxstream: constructors
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    micxxstream(std::istream *streamptr=NULL)
      : mcxxstream<std::istream>(streamptr)
    {};

    /** Reference-based constructor */
    micxxstream(std::istream &istream)
      : mcxxstream<std::istream>(istream)
    {};

    /** Destructor */
    ~micxxstream(void) {};
    //@}

    /*----------------------------------------------------------
     * micxxstream: input
     */
    ///\name Input Methods
    //@{
    /** Read up to @n bytes of data into @buf,
     *  returns number of bytes actually read. */
    virtual ByteCount read(char *buf, size_t n) {
      return streamp ? streamp->read(buf,n).gcount() : 0;
    };

    /** Read a single byte of data.  Returns EOF on eof. */
    virtual int getbyte(void) {
      return streamp ? streamp->get() : EOF;
    };
    //@}
  }; //-- /micxxstream


  /*====================================================================
   * mocxxstream : std::ostream output
   *====================================================================*/
  /** Wrapper for C++ output streams (std::ostream) */
  class mocxxstream
    : public mcxxstream<std::ostream>,
      public mostream
  {
  public:
    char      *_printf_buffer; ///< persistent buffer for printf hack
    ByteCount  _printf_buflen; ///< allocated size of _printf_buffer

  public:
    /*----------------------------------------------------------
     * mocxxstream: output
     */
    ///\name Constructors etc.
    //@{
    /** Default constructor */
    mocxxstream(std::ostream *ostreamptr)
      : mcxxstream<std::ostream>(ostreamptr),
	_printf_buffer(NULL),
	_printf_buflen(0)
    {};

    /** Reference-based constructor */
    mocxxstream(std::ostream &ostream)
      : mcxxstream<std::ostream>(ostream),
	_printf_buffer(NULL),
	_printf_buflen(0)
    {};

    /** Destructor */
    virtual ~mocxxstream(void)
    {
      if (_printf_buffer) free(_printf_buffer);
    };
    //@}

    /*----------------------------------------------------------
     * mocxxstream: output
     */
    ///\name Output Methods
    //@{
    /** Flush all pending output */
    virtual bool mflush(void) { return streamp->flush().good(); };

    /** Write @n bytes from @buf to the stream */
    virtual bool write(const char *buf, size_t n) {
      return streamp ? streamp->write(buf,n).good() : false;
    };

    /** Write a single byte to the stream */
    virtual bool putbyte(unsigned char c) {
      return streamp ? streamp->put(c).good() : false;
    };

    /** Write a C string to the stream */
    virtual bool puts(const char *s) {
      return streamp ? ((*streamp) << s).good() : false;
    };
    /** Write a C++ string to the buffer */
    virtual bool puts(const std::string &s) {
      return streamp ? ((*streamp) << s).good() : false;
    };

    /** printf() to the stream, va_list version
     * (the real thing) */
    virtual bool vprintf(const char *fmt, va_list &ap) {
      if (!streamp) return false;
      va_list ap_tmp;
      moot_va_copy(ap_tmp,ap);
      ByteCount nchars = vsnprintf(_printf_buffer, _printf_buflen, fmt, ap);
      if (nchars >= _printf_buflen) {
	//-- oops: reallocate!
	_printf_buffer = reinterpret_cast<char *>(realloc(_printf_buffer, nchars+1));
	assert(_printf_buffer != NULL);
	_printf_buflen = nchars+1;
	//-- ... and try again
	moot_va_copy(ap,ap_tmp);
	nchars = vsnprintf(_printf_buffer, _printf_buflen, fmt, ap);
      }
      //-- we now have a full string buffer: write it
      return streamp->write(_printf_buffer, nchars).good();
    };
    //@}
  }; //-- /mocxxstream

}; //-- /namespace mootio


#endif //_MOOT_CXXIO_H
