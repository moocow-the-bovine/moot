/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2004-2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootIO.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : low-level I/O abstractions
 *--------------------------------------------------------------------------*/

#include <stdlib.h>     // for free ()
#include <mootIO.h>
#include <mootConfig.h>

/*-- gnulib stuff --*/
extern "C" {
  /*#include "vasprintf.h"*/ //-- chokes on CentOS 5.4 ~ RHEL
}

namespace mootio {

bool mostream::vprintf(const char *fmt, va_list &ap) {
  char *obuf = NULL;
  size_t len = 0;
  len = vasprintf(&obuf, fmt, ap);
  bool rc = len >= 0 && write(obuf, len);
  if (obuf) free(obuf);
  return rc;
}

}; /* namespace mootio */
