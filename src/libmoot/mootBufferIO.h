/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootBufferIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : low-level I/O routines: C buffer I/O
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_BUFFERIO_H
#define _MOOT_BUFFERIO_H

#include <stdio.h>   // for EOF
#include <string.h>  // for memcpy() and friends
#include <assert.h>  // for sanity checks

#include <mootUtils.h> // for trimming
#include <mootIO.h>    // for everything else


namespace mootio {
  //using namespace std;

  /*====================================================================
   * mootio: classes
   *====================================================================*/

  /** \brief Streambuf-like class for input from C char* buffers
   *
   * This class provides an API for in-memory input which does
   * not resort to C++ sstream : this is a Good Thing, because:
   * \li we can pass around user-specified data buffers without
   *     extraneous copying, which nice for expat and just about
   *     every other C lib on the planet.
   */
  class micbuffer : virtual public mistream
  {
  public:
    const char  *cb_rdata;   ///< underlying character data buffer
    size_t       cb_offset;  ///< current read offset position in buffer
    size_t       cb_used;    ///< used length of buffer (in bytes)

  public:
    /*----------------------------------------------------------
     * micbuffer: constructors
     */
    /// \name Constructors etc.
    //@{
    /** Constructor from a user-specified buffer
     * Read operations will be performed on the
     * buffer specified.  User is responsible
     * for freeing memory associated with @data passed
     * to this constructor.
     */
    micbuffer(const void *data, size_t len)
      : cb_rdata((const char*)data),
	cb_offset(0),
	cb_used(len)
    {};

    /** Copy constructor */
    micbuffer(const micbuffer &cb)
      : cb_rdata(cb.cb_rdata),
	cb_offset(cb.cb_offset),
	cb_used(cb.cb_used)
    {};

    /** Destructor */
    virtual ~micbuffer(void) {};

    /** Clear buffer */
    inline void clear(void)
    {
      cb_offset = 0;
      cb_used = 0;
    };

    /** Free locally allocated data buffer, if any.  Implicitly calls clear() */
    inline void release(void)
    {
      clear();
      cb_rdata = NULL;
    };

    /** Asignment to a different data buffer */
    inline void assign(const void *data, size_t len) {
      cb_rdata  = (const char *)data;
      cb_used   = len;
      cb_offset = 0;
    };
    //@}

    /*----------------------------------------------------------
     * micbuffer: Integrity
     */
    /// \name Checks and Properties
    //@{
    /** Check for buffer validity */
    virtual bool valid(void) { return true; };

    /** Check for end-of-buffer (only meaningful for read operations) */
    virtual bool eof(void) { return cb_offset >= cb_used; };
    //@}

    /*----------------------------------------------------------
     * micbuffer: Input
     */
    /// \name Input Methods
    //@{

    /** Read up to @n bytes of data into @buf,
     *  returns number of bytes actually read. */
    virtual ByteCount read(char *buf, size_t n) {
      if (n==0 || cb_offset >= cb_used) return 0;
      else if (n < cb_used - cb_offset) {
	//-- normal case: copy local data to buf
	memcpy(buf, cb_rdata+cb_offset, n);
	cb_offset += n;
	return (ByteCount)n;
      }
      else {
	//-- partial read: copy some data to buf
	memcpy(buf, cb_rdata+cb_offset, cb_used-cb_offset);
	ByteCount nread = (ByteCount)(cb_used-cb_offset);
	cb_offset = cb_used;
	return nread;
      }
    };

    /** Read a single byte of data */
    virtual int getc(void) {
      if (cb_offset >= cb_used) return EOF;
      return cb_rdata[cb_offset++];
    };
    //@}

    /*----------------------------------------------------------
     * micbuffer: Utilties
     */
    ///\name Utilities
    //@{

    /** Get pointer to current read buffer (whole thing) */
    inline const char* data(void) const { return cb_rdata; };

    /** Get current used size of data buffer */
    inline size_t size(void) const { return cb_used; };

    /** Get current allocated size of data buffer */
    inline size_t capacity(void) const { return cb_used; };

    /** Get current read-offset of data buffer */
    inline size_t offset(void) const { return cb_offset; };

    /**
     * Return the unread portion of the buffer as a new STL string.
     * \@param normalize_ws Whether to normalize whitespace
     *         (replace all whitespace substrings with a single space).
     * \@param trim_left whether to trim all leading whitespace
     * \@param trim_right whether to trim all trailing whitespace
     */
    inline std::string as_string(bool normalize_ws=false,
				 bool trim_left=false,
				 bool trim_right=false)
      const
    {
      std::string str;
      str.reserve(cb_used-cb_offset);
      to_string(str, normalize_ws, trim_left, trim_right);
      return str;
    };

    /**
     * Append the unread portion of the buffer to an existing STL string.
     * \@param str destination string
     * \@param normalize_ws Whether to normalize whitespace
     *         (replace all whitespace substrings with a single space).
     * \@param trim_left whether to trim all leading whitespace
     * \@param trim_right whether to trim all trailing whitespace
     */
    inline void to_string(std::string &str,
			  bool normalize_ws =false,
			  bool trim_left    =false,
			  bool trim_right   =false)
      const
    {
      if (!cb_rdata) return;
      if (!normalize_ws)
	str.append(cb_rdata+cb_offset, cb_used-cb_offset);
      else
	moot::moot_normalize_ws(cb_rdata+cb_offset, cb_used-cb_offset,
				str,
				trim_left, trim_right);
    };
    //@}
  }; //-- /micbuffer


  /*====================================================================
   * mootio: input/output: mcbuffer
   *====================================================================*/

  /** \brief Streambuf-like class for I/O on C char* buffers
   *
   * This class provides an API for in-memory I/O which does
   * not resort to C++ sstream : this is a Good Thing, because:
   * \li we get to keep "real" printf()
   * \li we can pass around user-specified data buffers without
   *     extraneous copying, which nice for expat and just about
   *     every other C lib on the planet.
   */
  class mcbuffer
    : virtual public micbuffer,
      virtual public mostream
  {
  public:
    /// Default initial buffer size
    static const size_t CB_DEFAULT_SIZE = 32;

    /// Default number of extra bytes to get on reserve()
    static const size_t CB_DEFAULT_GET = 32;

  public:
    char   *cb_wdata;   ///< underlying character data buffer (write)
    size_t  cb_alloc;   ///< allocated size of buffer (in bytes)
    size_t  cb_get;     ///< number of extra bytes to get on internal reserve() calls
    bool    cb_created; ///< whether we allocated the @cb_wdata buffer ourself

  public:
    /*----------------------------------------------------------
     * mcbuffer: constructors
     */
    /// \name Constructors etc.
    //@{
    /** Default constructor, given optional initial buffer size */
    mcbuffer(size_t size=CB_DEFAULT_SIZE)
      : micbuffer(NULL,0),
	cb_wdata(NULL),
	cb_alloc(0),
	cb_get(CB_DEFAULT_GET),
	cb_created(true)
    {
      if (size) reserve(size);
    };

    /** Constructor from a user-specified buffer
     * Read/write operations will be performed on this
     * buffer as long as possible.  On write overflow,
     * a new local buffer will be allocated (which will
     * be freed on release()).  User is responsible
     * for freeing memory associated with @data passed
     * to this constructor.
     */
    mcbuffer(void *data,
	    size_t used,
	    size_t alloc=0,
	    size_t get=CB_DEFAULT_GET)
      : micbuffer((const char *)data,used),
	cb_wdata((char *)data),
	cb_alloc(alloc),
	cb_get(get),
	cb_created(false)
    {
      if (!alloc) cb_alloc = cb_used;
      cb_rdata = cb_wdata;
    };

    /** Copy constructor */
    mcbuffer(const micbuffer &cb)
      : micbuffer(NULL,0),
	cb_wdata(NULL),
	cb_created(true)
    {
      reserve(cb.cb_used);
      memcpy(cb_wdata, cb.cb_rdata, cb.cb_used);
      cb_offset = cb.cb_offset;
      cb_used   = cb.cb_used;
      cb_rdata  = cb_wdata;
    };

    /** Asignment to a different data buffer (always copies) */
    inline void assign(const void *data, size_t len) {
      reserve(len);
      memcpy(cb_wdata, data, len);
      cb_rdata   = cb_wdata;
      cb_offset  = 0;
      cb_used    = len;
      cb_created = true;
    };

    /** Destructor */
    virtual ~mcbuffer(void) { release(); };

    /** Clear buffer */
    inline void clear(void)
    {
      cb_offset = 0;
      cb_used = 0;
    };

    /** Free locally allocated data buffer, if any.  Implicitly calls clear() */
    inline void release(void)
    {
      clear();
      if (cb_created && cb_wdata) free(cb_wdata);
      cb_wdata = NULL;
      cb_rdata = NULL;
      cb_alloc = 0;
    };
    //@}

    /*----------------------------------------------------------
     * mcbuffer: Output
     */
    /// \name Output Methods
    //@{
    /** "Flush" all data before offset() from the buffer. */
    virtual bool flush(void) {
      if (!cb_offset || !cb_wdata) return true;
      memmove(cb_wdata, cb_wdata+cb_offset, cb_used-cb_offset);
      cb_used -= cb_offset;
      cb_offset = 0;
      return true;
    };

    /** Write @n bytes from @buf to the buffer */
    virtual bool write(const char *buf, size_t n) {
      if (!reserve(n+cb_used, cb_get)) return false;
      memcpy(cb_wdata+cb_used, buf, n);
      cb_used += n;
      return true;
    };

    /** Write a single byte to the buffer */
    virtual bool putc(unsigned char c) {
      if (!reserve(1+cb_used, cb_get)) return false;
      cb_wdata[cb_used++] = c;
      return true;
    };

    /** Write a C string to the buffer */
    virtual bool puts(const char *s) {
      return write(s,strlen(s));
    };
    /** Write a C++ string to the buffer */
    virtual bool puts(const std::string &s) {
      return write(s.data(),s.size());
    };

    /** printf() to the buffer, va_list version */
    virtual bool vprintf(const char *fmt, va_list &ap)
    {
      size_t nchars = vsnprintf(cb_wdata+cb_used, cb_alloc-cb_used, fmt, ap);
      if (nchars >= cb_alloc-cb_used) {
	if (!reserve(1+nchars+cb_used, cb_get)) return false;
	vsnprintf(cb_wdata+cb_used, cb_alloc-cb_used, fmt, ap);
      }
      cb_used += nchars;
      return true;
    };
    //@}

    /*----------------------------------------------------------
     * mcbuffer: Utilties
     */
    /// \name Utilities
    //@{
    /** Grow the buffer to fit at least @size + @pad bytes in it */
    inline bool reserve(size_t size, size_t pad=0) {
      if (size > cb_alloc) {
	size_t newalloc = size+pad;
	if (cb_created) {
	  //-- local buffer: we can just realloc()
	  if (cb_wdata) cb_wdata = (char *)realloc(cb_wdata, newalloc);
	  else cb_wdata = (char *)malloc(newalloc);
	} else {
	  //-- user buffer: we need to slurp it
	  char *newdata = (char *)malloc(newalloc);
	  memcpy(newdata, cb_wdata, cb_used);
	  cb_wdata = newdata;
	  cb_created = true;
	}
	assert(cb_wdata != NULL);
	cb_rdata = cb_wdata;
	cb_alloc = newalloc;
      }
      return true;
    };
    //@}

  }; //-- /mcbuffer

  /*====================================================================
   * mootio: c-buffers: aliases
   *====================================================================*/
  typedef micbuffer mibuffer;   ///< alias for input-only buffers
  typedef mcbuffer  mobuffer;   ///< alias for (input+)output buffers
  typedef mcbuffer  mocbuffer;  ///< alias for (input+)output buffers
  typedef mcbuffer  miocbuffer; ///< alias for i/o buffers
  typedef mcbuffer  miobuffer;  ///< alias for i/o buffers
  typedef mcbuffer  mbuffer;    ///< alias for i/o buffers

}; //-- /namespace mootio


#endif //_MOOT_BUFFERIO_H
