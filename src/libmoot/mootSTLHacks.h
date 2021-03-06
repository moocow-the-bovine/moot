/*-*- Mode: C++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2009 by Bryan Jurish <moocow@cpan.org>

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

/*----------------------------------------------------------------------
 * Name: mootSTLHacks.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + STL hacks for libmoot
 *----------------------------------------------------------------------*/

/**
\file mootSTLHacks.h
\brief various STL and namespace-related hacks
*/

#ifndef _MOOT_STL_HACKS_H
#define _MOOT_STL_HACKS_H

#if defined(__GNUC__)
#  if __GNUC__ >= 3                                         /* gcc >= v3.0 */
#    if __GNUC__ == 3 && __GNUC_MINOR__ == 0                /* gcc == v3.0 */
#      define moot_STL_NAMESPACE std
#    else  /* !( __GNUC__ == 3 && __GNUC_MINOR__ == 0 ) */  /* gcc >  v3.0 */
#      define moot_STL_NAMESPACE __gnu_cxx
#    endif /*  ( __GNUC__ == 3 && __GNUC_MINOR__ == 0 ) */
#    include <ext/hash_map>
#    include <ext/hash_set>
#  else    /* !( __GNUC__ >= 3 ) */                         /* gcc <  v3.0 */
#    define moot_STL_NAMESPACE std
#    include <hash_map>
#    include <hash_set>
#  endif   /*  ( __GNUC__ >= 3 ) */

#  ifndef MOOT_HAVE_HASH_STRING
/*-- hack for string hashing --*/
namespace moot_STL_NAMESPACE {
      template<> struct hash<std::string> {
        std::size_t operator()(const std::string &__s) const {
          return __stl_hash_string(__s.c_str());
        }
      };
};
#  endif // MOOT_HAVE_HASH_STRING

#else  /* !( defined(__GNUC__) ) */
#  include <hash_map>
#  include <hash_set>
#  define moot_STL_NAMESPACE std
#endif /*  ( defined(__GNUC__) ) */

/* Namespace definitions */
#define moot_NAMESPACE moot
#define moot_BEGIN_NAMESPACE namespace moot {
#define moot_END_NAMESPACE };

/* gcc6 workaround for "error: reference to 'hash' is ambiguous" using hash<TYPE> */
#define moot_hash moot_STL_NAMESPACE::hash

#endif // _MOOT_STL_HACKS_H
