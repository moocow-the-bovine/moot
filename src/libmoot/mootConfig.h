/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2007 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
 * File: mootConfig.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : configuration (hack)
 *--------------------------------------------------------------------------*/

/**
 * \file mootConfig.h
 * \brief safely include autoheader preprocessor defines
 *
 * \file mootNoConfig.h
 * \brief Undefine any autoheader preprocessor defines
 *
 * \file mootAutoConfig.h
 * \brief autoheader-generated preprocessor defines
 */

/* 
 * Define a sentinel preprocessor symbol _MOOT_CONFIG_H, just
 * in case someone wants to check whether we've already
 * (#include)d this file ....
 */
#ifndef _MOOT_CONFIG_H
#define _MOOT_CONFIG_H
#endif /* _MOOT_CONFIG_H */

/* 
 * Putting autoheader files within the above #ifndef/#endif idiom
 * is potentially a BAD IDEA, since we might need to (re-)define
 * moot's autoheader-generated preprocessor symbols (e.g. after
 * (#include)ing in some config.h from another autoheader package
 */
#include <mootNoConfig.h>
#include <mootAutoConfig.h>
