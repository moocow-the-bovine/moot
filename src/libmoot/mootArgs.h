/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2009 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootArgs.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : variable-argument macros
 *--------------------------------------------------------------------------*/

/** \file mootArgs.h
 *  \brief utilities for functions taking variable number of arguments
 */

#ifndef _MOOT_ARGS_H
#define _MOOT_ARGS_H

#include <mootConfig.h>
#include <stdarg.h>

/** \brief Copy a va_list \a src to \a dst */
#ifdef __va_copy
# define moot_va_copy(dst,src) __va_copy((dst),(src))
#else
# define moot_va_copy(dst,src) dst = (src)
#endif



#endif //_MOOT_ARGS_H
