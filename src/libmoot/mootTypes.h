/*-*- Mode: C++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

/*----------------------------------------------------------------------
 * Name: mootTypes.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + shared typedefs for libmoot
 *----------------------------------------------------------------------*/
/** \file mootTypes.h
 * \brief Common typedefs and constants.
 */

#ifndef _moot_TYPES_H
#define _moot_TYPES_H

#include <string>
#include <deque>
#include <map>
#include <set>

#include <mootToken.h>

#if defined(__GNUC__)
#  if __GNUC__ >= 3
#    if __GNUC_MINOR__ == 0
#      define moot_STL_NAMESPACE std
#    else
#      define moot_STL_NAMESPACE __gnu_cxx
#    endif /* __GNUC_MINOR__ == 0 */
#    include <ext/hash_map>
#    include <ext/hash_set>
#  else  /* __GNUC__ >= 3 */
#    define moot_STL_NAMESPACE std
#    include <hash_map>
#    include <hash_set>
#  endif /* __GNUC__ >= 3 */

/*-- hack for string hashing --*/
namespace moot_STL_NAMESPACE {
      template<> struct hash<std::string> {
        std::size_t operator()(const std::string &__s) const {
          return __stl_hash_string(__s.c_str());
        }
      };
};

#else  /* defined(__GNUC__) */
#  include <hash_map>
#  include <hash_set>
#  define moot_STL_NAMESPACE std
#endif /* defiend(__GNUC__) */

/** Maximum weight */
#ifndef MAXFLOAT
#include <values.h>
#endif

/* Namespace definitions: not yet ready */
#define moot_NAMESPACE moot
#define moot_BEGIN_NAMESPACE namespace moot {
#define moot_END_NAMESPACE };
//#define moot_NAMESPACE
//#define moot_BEGIN_NAMESPACE
//#define moot_END_NAMESPACE

moot_BEGIN_NAMESPACE

using namespace std;
using namespace moot_STL_NAMESPACE;

/*----------------------------------------------------------------------
 * Basic Types
 *----------------------------------------------------------------------*/

//#define MOOT_USE_DOUBLE

#ifdef MOOT_USE_DOUBLE
/** Type for probabilities */
typedef double ProbT;
#else
/** Type for probabilities */
typedef float ProbT;
#endif //-- MOOT_USE_DOUBLE

/** Count types (for raw frequencies) */
typedef ProbT CountT;


/**
 * \def mootProbEpsilon
 * Constant representing a minimal probability.
 * Used for default HMM constructor. 
 * The standard value should be appropriate for an IEEE 754-1985 float.
 */
#define mootProbEpsilon  1.19209290E-06F
/*
#ifdef FLT_EPSILON
//#define mootProbEpsilon FLT_EPSILON*10
# define mootProbEpsilon  FLT_EPSILON*10
#else
# define mootProbEpsilon  1.19209290E-06F
//#define mootProbEpsilon 1.19209290E-07F
#endif
*/

/** \def MOOT_PROB_ZERO
 * Probability lower-bound.
 * The standard value should be appropriate for an IEEE 754-1985 float.
 *
 * \def MOOT_PROB_ONE
 * Probability upper-bound
 * The standard value should be appropriate for an IEEE 754-1985 float.
 */
#define MOOT_PROB_NEG  -3E+38
#define MOOT_PROB_ZERO -1E+38
#define MOOT_PROB_ONE   0.0
/*
#ifdef FLT_MAX
#  define MOOT_PROB_NEG  -FLT_MAX
#  define MOOT_PROB_ZERO -1E+38
#  define MOOT_PROB_ONE   0.0
# else //-- !(DBL|FLT)_MAX
#  define MOOT_PROB_NEG  -3E+38
#  define MOOT_PROB_ZERO -1E+38
#  define MOOT_PROB_ONE   0.0
#endif //-- /(DBL|FLT)_MAX
*/

moot_END_NAMESPACE

#endif /* _moot_TYPES_H */
