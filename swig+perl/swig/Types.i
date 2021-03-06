/* -*- Mode: C++ -*- */
/*
 * File: Types.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: types
 */

%{
using namespace moot;
%}

//-- mootTypes.h
//typedef double ProbT;  //-- only if moot defines MOOT_USE_DOUBLE (not by default)
typedef float  ProbT;
typedef ProbT  CountT;

extern const ProbT mootProbEpsilon;
extern const ProbT MOOT_PROB_NEG;
extern const ProbT MOOT_PROB_ZERO;
extern const ProbT MOOT_PROB_ONE;
extern const ProbT MOOT_PROB_NONE;
