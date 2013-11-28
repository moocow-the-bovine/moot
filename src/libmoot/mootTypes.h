/*-*- Mode: C++ -*-*/

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2011 by Bryan Jurish <moocow@cpan.org>

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
 * Name: mootTypes.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + shared typedefs for libmoot
 *----------------------------------------------------------------------*/
/** \file mootTypes.h
 * \brief Common typedefs and constants.
 */

#ifndef _moot_TYPES_H
#define _moot_TYPES_H

#include <mootConfig.h>

#include <string>
#include <map>
#include <set>
#include <deque>
#include <mootSTLHacks.h>

#include <stdint.h>

/* Maximum weight */
#if !defined(MAXFLOAT)
# if defined(HAVE_VALUES_H)
#  include <values.h>
# elif defined(HAVE_FLOAT_H)
#  include <float.h>
# endif /* HAVE_VALUES_H */
# if !defined(MAXFLOAT)
#  define MAXFLOAT 1E38
# endif /* !defined(MAXFLOAT) [2nd] */
#endif /* !defined(MAXFLOAT) */

moot_BEGIN_NAMESPACE

using namespace std;

#ifdef moot_STL_NAMESPACE
 using namespace moot_STL_NAMESPACE;
#endif

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

/** Offset type (for byte offsets) */
typedef long unsigned int OffsetT;


#ifdef MOOT_32BIT_FORCE
 /** Fixed-width signed integer type for binary I/O (32-bit) */
 typedef int32_t  BinInt;
 /** Fixed-width signed integer type for binary I/O (32-bit) */
 typedef int32_t  BinLong;
 /** Fixed-width unsigned integer type for binary I/O (32-bit) */
 typedef uint32_t BinUInt;
 /** Fixed-width unsigned integer type for binary I/O (32-bit) */
 typedef uint32_t BinULong;
#else
 /** Fixed-width signed integer type for binary I/O (native) */
 typedef int               BinInt;
 /** Fixed-width integer type for binary I/O (native) */
 typedef long int          BinLong;
 /** Fixed-width unsigned integer type for binary I/O (native) */
 typedef unsigned int      BinUInt;
 /** Fixed-width unsigned integer type for binary I/O (native) */
 typedef unsigned long int BinULong;
#endif

/** alias (fixed-width) */
typedef BinInt   Int;

/** alias (fixed-width) */
typedef BinUInt  UInt;

/** alias (fixed-width) */
typedef BinUInt  Size;

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
 *
 * \def MOOT_PROB_NONE
 * Impossible probability, used as a default flag.
 *
 * \def MOOT_PROB_SAFE(p)
 * True iff probability p is considered "safe"
 */
#define MOOT_PROB_NEG  -3E+38
#define MOOT_PROB_ZERO -1E+38
#define MOOT_PROB_ONE   0.0
#define MOOT_PROB_NONE  1.0
#define MOOT_PROB_SAFE(p) (p > -1E+37)
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
