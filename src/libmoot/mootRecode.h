/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootRecode.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : librecode interface
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_RECODE_H
#define _MOOT_RECODE_H

#include <mootConfig.h>

#ifdef MOOT_RECODE_ENABLED
#include <stdbool.h>
#include <recodext.h>
#endif // MOOT_RECODE_ENABLED

#include <assert.h>
#include <string>

#include <mootIO.h>

namespace moot {

using namespace std;

/** \brief Interface to librecode character-conversion routines */
class mootRecoder {
public:
  /*--------------------------------------------------------------
   * DATA
   */
#ifdef MOOT_RECODE_ENABLED
  RECODE_OUTER   rc_outer;   ///< outer context for librecode
  RECODE_REQUEST rc_request; ///< actual recode request
#else
  void *         rc_outer;   ///< outer context for librecode (dummy)
  void *         rc_request; ///< actual recode request (dummy)
#endif // MOOT_RECODE_ENABLED
  std::string    rc_reqstr;  ///< request string



public:
  /*--------------------------------------------------------------
   * METHODS
   */
  /*----------------------------------------------------*/
  /** \name Constructors etc. */
  //@{
  /** Default constructor, given optional request string */
  mootRecoder(const std::string &requestString="", bool diacritics_only=false)
    : rc_outer(NULL), rc_request(NULL)
  {
    if (!requestString.empty()) scan_request(requestString,diacritics_only);
  };

  /** Default constructor, given source and destination encodings */
  mootRecoder(const std::string &src, const std::string &dst, bool diacritics_only=false)
    : rc_outer(NULL), rc_request(NULL)
  {
    scan_request(src, dst, diacritics_only);
  };

  /** Default destructor */
  ~mootRecoder(void)
  {
#ifdef MOOT_RECODE_ENABLED
    if (rc_request) recode_delete_request(rc_request); 
    if (rc_outer) recode_delete_outer(rc_outer);
#endif // MOOT_RECODE_ENABLED
  };

  /** ensure outer context is valid */
  void ensure_outer(void) {
#ifdef MOOT_RECODE_ENABLED
    if (!rc_outer) rc_outer = recode_new_outer(true); // (AUTO_ABORT)
#endif // MOOT_RECODE_ENABLED
  };
  //@}

  /*----------------------------------------------------*/
  /** \name Requests */
  //@{
  /** Scan a recode request string (i.e. one of the form "SRC..DST") */
  inline void scan_request(const std::string &reqstr, bool diacritics_only=false) {
    ensure_outer();
    rc_reqstr = reqstr;
#ifdef MOOT_RECODE_ENABLED
    if (!rc_request) rc_request = recode_new_request(rc_outer);
    assert(rc_request != NULL);
    if (!reqstr.empty()) {
      rc_request->diacritics_only = diacritics_only;  // this MUST happen before scan_request() !
      if (!recode_scan_request(rc_request, reqstr.c_str())) {
	fprintf(stderr, "mootRecode::scan_request(): failed to scan request `%s'\n",
		reqstr.c_str());
	if (rc_request) {
	  recode_delete_request(rc_request);
	  rc_request = NULL;
	}
      }
      return;
    }
    else if (rc_request) {
      recode_delete_request(rc_request);
      rc_request = NULL;
    }
#else
    fprintf(stderr, "mootRecode::scan_request(): librecode disabled! (request=\"%s\"\n",
	    reqstr.c_str());
#endif // MOOT_RECODE_ENABLED
  };

  /** Generate and scan a simple request */
  inline void scan_request(const std::string &src, const std::string &dst, bool diacritics_only=false)
  {
    if (src.empty() && dst.empty()) scan_request(src,diacritics_only);
    else {
      std::string reqstr = src;
      reqstr.append("..");
      reqstr.append(dst);
      scan_request(reqstr,diacritics_only);
    }
  };
  //@}

  /*----------------------------------------------------*/
  /** \name Recoding: *2file */
  //@{
  /** Recode a C string to a C stream */
  inline bool string2file(const char *in, FILE *out)
  {
#ifdef MOOT_RECODE_ENABLED
    if (rc_request)
      return recode_string_to_file(rc_request,in,out);
#endif
    fputs(in,out);
    return !ferror(out);
  };

  /** Recode a C buffer to a C stream */
  inline bool buffer2file(const char *buf, size_t buflen, FILE *out)
  {
#ifdef MOOT_RECODE_ENABLED
    if (rc_request)
      return recode_buffer_to_file(rc_request,buf,buflen,out);
#endif
    fwrite(buf,buflen,1,out);
    return !ferror(out);
  };

  /** Recode an STL string to a C stream */
  inline bool string2file(const std::string &in, FILE *out)
  {
#ifdef MOOT_RECODE_ENABLED
    if (rc_request)
      return recode_buffer_to_file(rc_request,in.data(),in.size(),out);
#endif
    fwrite(in.data(), in.size(), 1, out);
    return !ferror(out);
  };
  //@}

  /*----------------------------------------------------*/
  /** \name Recoding: *2buffer */
  //@{
  /** Recode a C buffer to a different C buffer */
  inline bool buffer2buffer(const char *in, size_t in_size, char **out, size_t *out_used, size_t *out_alloc)
  {
#ifdef MOOT_RECODE_ENABLED
    if (rc_request)
      return recode_buffer_to_buffer(rc_request, in, in_size, out, out_used, out_alloc);
#endif
    if (!*out) *out = (char *)malloc(in_size);
    else if (*out_alloc < in_size) {
      free(*out);
      *out = (char *)malloc(in_size);
    }
    assert(*out != NULL);
    *out_alloc = in_size;
    memcpy(*out, in, in_size);
    *out_used = in_size;
    return true;
  };

  /** Recode a C string to a C buffer */
  inline bool string2buffer(const char *s, char **out, size_t *out_used, size_t *out_alloc)
  {
    return buffer2buffer(s,strlen(s), out,out_used,out_alloc);
  };

  /** Recode an STL string to a C buffer */
  inline bool string2buffer(const std::string &in, char **out, size_t *out_used, size_t *out_alloc)
  {
    return buffer2buffer(in.data(),in.size(), out,out_used,out_alloc);
  };
  //@}

  /*----------------------------------------------------*/
  /** \name Recoding: *2string */
  //@{
  /** Recode a C buffer to an STL string (append) */
  inline bool buffer2string(const char *in, size_t in_size, std::string &out)
  {
    char *tmp = NULL;
    size_t out_used = 0, out_alloc = 0;
    bool rv = buffer2buffer(in,in_size, &tmp, &out_used, &out_alloc);
    if (tmp) {
      out.append(tmp, out_used);
      free(tmp);
    }
    return rv;
  }

  /** Recode a C buffer to an STL string */
  inline bool string2string(const char *s, std::string &out)
  {
    return buffer2string(s,strlen(s), out);
  };

  /** Recode an STL string to an STL string */
  inline bool string2string(const std::string &in, std::string &out)
  {
    return buffer2string(in.data(),in.size(), out);
  };
  /* ... etc. */
  //@}

  /*----------------------------------------------------*/
  /** \name Recoding: *2mstream */
  //@{
  /** Recode a C string to a C stream */
  inline bool string2mstream(const char *in, mootio::mostream *out)
  {
    std::string s;
    bool rv = string2string(in,s);
    return out && out->puts(s) && rv;
  };

  /** Recode a C buffer to a C stream */
  inline bool buffer2mstream(const char *buf, size_t buflen, mootio::mostream *out)
  {
    std::string s;
    bool rv = buffer2string(buf,buflen,s);
    return out && out->puts(s) && rv;
  };

  /** Recode an STL string to a C stream */
  inline bool string2mstream(const std::string &in, mootio::mostream *out)
  {
    std::string s;
    bool rv = string2string(in,s);
    return out && out->puts(s) && rv;
  };
  //@}

}; //-- /class mootRecoder



/**
 * \brief Special 2-phase recoder object for XML text
 */ 
class mootXMLRecoder {
public:
  /*--------------------------------------------------------------
   * DATA
   */
  bool        standalone;         ///< whether we should bother

  mootRecoder rc1;                ///< first-stage recoder
  mootRecoder rc2;                ///< second-stage recoder

  char       *buf1;               ///< buffer for intermediate recoding results
  size_t      buf1_used;          ///< number of bytes used in buf1
  size_t      buf1_alloc;         ///< allocated size of buf1

  char       *buf2;               ///< buffer for final recoding results
  size_t      buf2_used;          ///< number of bytes used in buf2
  size_t      buf2_alloc;         ///< allocated size of buf2

public:
  /*--------------------------------------------------------------
   * METHODS
   */

  /*----------------------------------------------------*/
  /** \name Constructors etc. */
  //@{
  /** Default constructor, given optional requests */
  mootXMLRecoder(const std::string &src="", const std::string &dst="")
    : buf1(NULL), buf1_used(0), buf1_alloc(0),
      buf2(NULL), buf2_used(0), buf2_alloc(0)
  {
    //-- share outer context
    rc1.ensure_outer();
    rc2.rc_outer = rc1.rc_outer;

    scan_request(src,dst);
  };

  /** Default destructor */
  ~mootXMLRecoder(void)
  {
    rc2.rc_outer = NULL; ///< don't double-free shared outer
    if (buf1) free(buf1);
    if (buf2) free(buf2);
  };
  //@{

  /*----------------------------------------------------*/
  /** \name Requests */
  //@{
  /** Scan a recode request string (i.e. one of the form "SRC..DST") */
  inline void scan_request(const std::string &reqstr)
  {
    size_t dst_begin    = reqstr.rfind("..");
    string src(reqstr, 0, dst_begin);
    string dst(reqstr, dst_begin);
    standalone = (dst == "XML-standalone" || dst == "h0");
    scan_request(src,dst);
  };

  /** Generate and scan a simple request */
  inline void scan_request(const std::string &src, const std::string &dst) {
    standalone = (dst.empty() || dst == "XML-standalone" || dst == "h0");
    if (standalone) {
      if (src.empty()) {
	rc1.scan_request("",false);
	return;
      }
      rc1.scan_request(src,dst,false);
    } else {
      rc1.scan_request(src,"HTML_4.0",false);
      rc2.scan_request("HTML_4.0",dst,true);
    }
  };
  //@}

  /*----------------------------------------------------*/
  /** \name Recoding: *2file() */
  //@{
  /** Recode a C buffer to a C stream */
  inline bool buffer2file(const char *in, size_t in_size, FILE *out)
  {
    if (standalone) return rc1.buffer2file(in,in_size, out);
    //-- temp bools are ugly, but librecode seems to be returning weird...
    bool rv1 = rc1.buffer2buffer(in,in_size, &buf1,&buf1_used,&buf1_alloc);
    bool rv2 = rc2.buffer2file(buf1,buf1_used, out);
    return rv1 && rv2;
  };

  /** Recode a C string to a C stream */
  inline bool string2file(const char *in, FILE *out)
  {
    return buffer2file(in,strlen(in),out);
  };

  /** Recode an STL string to a C stream */
  inline bool string2file(const std::string &in, FILE *out)
  {
    return buffer2file(in.data(),in.size(), out);
  };

  /*----------------------------------------------------*/
  /** \name Recoding: *2buffer() */
  //@{
  /** Recode a C buffer to a different C buffer */
  inline bool buffer2buffer(const char *in, size_t in_size, char **out, size_t *out_used, size_t *out_alloc)
  {
    if (standalone) return rc1.buffer2buffer(in,in_size, out,out_used,out_alloc);
    //-- temp bools are ugly, but librecode is returning weird...
    bool rv1 = rc1.buffer2buffer(in,in_size, &buf1,&buf1_used,&buf1_alloc);
    bool rv2 = rc2.buffer2buffer(buf1,buf1_used, out,out_used,out_alloc);
    return rv1 && rv2;
  };

  /** Recode a C string to a C buffer */
  inline bool string2buffer(const char *s, char **out, size_t *out_used, size_t *out_alloc)
  {
    return buffer2buffer(s,strlen(s), out,out_used,out_alloc);
  };

  /** Recode an STL string to a C buffer */
  inline bool string2buffer(const std::string &in, char **out, size_t *out_used, size_t *out_alloc)
  {
    return buffer2buffer(in.data(),in.size(), out,out_used,out_alloc);
  };
  //@}

  /*----------------------------------------------------*/
  /** \name Recoding: *2string() */
  //@{
  /** Recode a C buffer to an STL string (append) */
  inline bool buffer2string(const char *in, size_t in_size, std::string &out)
  {
    if (standalone) {
      //-- re-use internal buffer
      bool rv1 = rc1.buffer2buffer(in,in_size, &buf1,&buf1_used,&buf1_alloc);
      if (buf1) out.append(buf1,buf1_used);
      return rv1;
    }
    //-- temp bools are ugly, but librecode seems to be returning weird...
    bool rv1 = rc1.buffer2buffer(in,in_size, &buf1,&buf1_used,&buf1_alloc);
    bool rv2 = rc2.buffer2string(buf1,buf1_used, out);
    return rv1 && rv2;
  };

  /** Recode a C buffer to an STL string (append) */
  inline bool string2string(const char *s, std::string &out)
  {
    return buffer2string(s,strlen(s), out);
  };

  /** Recode an STL string to an STL string (append) */
  inline bool string2string(const std::string &in, std::string &out)
  {
    return buffer2string(in.data(),in.size(), out);
  };
  /* ... etc. */
  //@}

  /*----------------------------------------------------*/
  /** \name Recoding: *2mstream() */
  //@{
  /** Recode a C buffer to an STL string (append) */
  inline bool buffer2mstream(const char *in, size_t in_size, mootio::mostream *out)
  {
    if (standalone) {
      //-- re-use internal buffer1
      bool rv1 = rc1.buffer2buffer(in,in_size, &buf1,&buf1_used,&buf1_alloc);
      if (buf1) return 
		  out && (out->write(buf1,buf1_used) == buf1_used) && rv1;
    }
    //-- temp bools are ugly, but librecode seems to be returning weird...
    bool rv1 = rc1.buffer2buffer(in,in_size, &buf1,&buf1_used,&buf1_alloc);
    bool rv2 = rc2.buffer2buffer(buf1,buf1_used, &buf2,&buf2_used,&buf2_alloc);
    return
      out && (out->write(buf2,buf2_used) == buf2_used) && rv1 && rv2;
  };

  /** Recode a C buffer to an STL string (append) */
  inline bool string2mstream(const char *s, mootio::mostream *out)
  {
    return buffer2mstream(s,strlen(s), out);
  };

  /** Recode an STL string to an STL string (append) */
  inline bool string2mstream(const std::string &in, mootio::mostream *out)
  {
    return buffer2mstream(in.data(),in.size(), out);
  };
  /* ... etc. */
  //@}

}; //-- /class mootXMLRecoder



}; //-- /namespace moot

#endif //_MOOT_RECODE_H

