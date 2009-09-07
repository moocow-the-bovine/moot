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
#define mootWrapVersion "0.01"

/*-- wrappers --*/
%include "Types.i"
%include "Templates.i"
%include "Token.i"
%include "Enum.i"
%include "AssocVector.i"
%include "HMM.i"
