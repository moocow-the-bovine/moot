//-*- Mode: C++ -*-

%module "moot"
%include "std_string.i"
%include "std_vector.i"

/*-- debug ? --*/
//#define MOOTDEBUG 1

/*-- headers --*/
%{
#include <moot.h>

using namespace moot;
using namespace mootio;
%}

/*-- config --*/
extern const char * const PACKAGE_VERSION;
%ignore VERSION;
%rename(VERSION) mootWrapVersion;
#define mootWrapVersion "0.02"

/*-- wrappers --*/
%include "Types.i"
%include "STL.i"
%include "Token.i"
%include "Enum.i"
%include "AssocVector.i"
%include "HMM.i"
%include "DynHMM.i"
%include "Ngrams.i"
%include "Lexfreqs.i"

/*--- additions --*/
%perlcode %{
sub library_version { return $moot::PACKAGE_VERSION; }
%}
