/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootGenericLexer.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : generic lexer routines
 *--------------------------------------------------------------------------*/

#include <mootGenericLexer.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <mootIO.h>
#include <mootCIO.h>
#include <mootBufferIO.h>

#include <string>

namespace moot {
  using namespace std;

  /*====================================================================
   * Generic Lexer: Constructors etc
   *====================================================================*/

  /*----------------------------------------------------------------------
   * Generic Lexer: Constructors etc: Destructor
   */
  GenericLexer::~GenericLexer(void)
  {
    void **current = (void**)mgl_yy_current_buffer_p();
    if (current && *current) {
      mgl_yy_delete_buffer(*current);
      *current = NULL;
    }
    if (mgl_in_created && mglin) delete mglin;
    if (mgl_out_created && mglout) delete mglout;
  };

  /*----------------------------------------------------------------------
   * Generic Lexer: Constructors etc: reset()
   */
  void GenericLexer::reset(void)
  {
    mgl_begin(0);
  };

  /*----------------------------------------------------------------------
   * Generic Lexer: Constructors etc: clear()
   */
  void GenericLexer::clear(bool clear_input, bool clear_output)
  {
    if (clear_input) {
      if (mglin && mgl_in_created) delete mglin;
      mgl_in_created = false;
      mglin = NULL;
    }
    if (clear_output) {
      if (mglout && mgl_out_created) delete mglout;
      mgl_out_created = false;
      mglout = NULL;
    }
  };

  /*====================================================================
   * Generic Lexer: Required wrapper methods
   *====================================================================*/

  void *GenericLexer::mgl_yy_current_buffer_p(void)
  { return NULL; }
  //{ return &((void *)yy_current_buffer); }

  void GenericLexer::mgl_begin(int stateno)
  { return; };
  //{ BEGIN(stateno); };



  /*====================================================================
   * Generic Lexer: Input Selection
   *====================================================================*/

  /*----------------------------------------------------------------------
   * Generic Lexer: Input Selection: mootio
   */
  void GenericLexer::from_mstream(mootio::mistream *in)
  {
    if (in) {
      if (mglin && in != mglin && mgl_in_created) {
	delete mglin;
	mgl_in_created = false;
      }
      mglin = in;
    }
    //-- grey magic (see flex(1))
    void **bufp = (void **)mgl_yy_current_buffer_p();
    if (bufp && *bufp) {
      if (*bufp) mgl_yy_delete_buffer(*bufp);
      mgl_yy_switch_to_buffer(mgl_yy_create_buffer(MGL_DEFAULT_BUFFER_SIZE));
    }
    reset();
  };

  /*----------------------------------------------------------------------
   * Generic Lexer: Input Selection: named file
   */
  void GenericLexer::from_filename(const std::string &filename)
  {
    clear(true,false);
    mglin = new mifstream(filename);
    mgl_in_created = true;
    from_mstream(mglin);
  };


  /*----------------------------------------------------------------------
   * Generic Lexer: Input Selection: C streams
   */
  void GenericLexer::from_file(FILE *in)
  {
    clear(true,false);
    mglin = new mootio::micstream(in);
    mgl_in_created = true;
    from_mstream(mglin);
  };

  /*----------------------------------------------------------------------
   * Generic Lexer: Input Selection: C buffer
   */
  void GenericLexer::from_buffer(const char *buf, size_t len)
  {
    clear(true,false);
    mglin = new mootio::micbuffer(buf,len);
    mgl_in_created = true;
    from_mstream(mglin);
  };


  /*----------------------------------------------------------------------
   * Generic Lexer: Output Selection: mootio
   */
  void GenericLexer::to_mstream(mootio::mostream *out)
  {
    if (out) {
      if (mglout && out != mglout && mgl_out_created) {
	delete mglout;
	mgl_out_created = false;
      }
      mglout = out;
    }
  };

  /*----------------------------------------------------------------------
   * Generic Lexer: Output Selection: named file
   */
  void GenericLexer::to_filename(const std::string &filename)
  {
    clear(false,true);
    mglout = new mofstream(filename);
    mgl_out_created = true;
    to_mstream(mglout);
  };

  /*----------------------------------------------------------------------
   * Generic Lexer: Output Selection: C streams
   */
  void GenericLexer::to_file(FILE *out)
  {
    clear(false,true);
    mglout = new mootio::mocstream(out);
    mgl_out_created = true;
    to_mstream(mglout);
  };

  /*------------------------------------------------------------
   * Diagnostics : yycarp()
   */
  void GenericLexer::yycarp(const char *fmt, ...)
  {
    if (!lexname.empty()) fprintf(stderr, "%s: ", lexname.c_str());
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "in \"%s\" at line %ld, column %ld.\n",
	    (mglin ? mglin->name.c_str() : "(unknown)"),
	    theLine, theColumn);
  };

}; //-- /namespace moot

