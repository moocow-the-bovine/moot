/*-*- Mode: C++ -*- */
extern "C" {
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
//#include "ppport.h"
};

#include <moot.h>
#include "MootPerl.h"

MODULE = Moot		PACKAGE = Moot

##=====================================================================
## Moot (Debug)
##=====================================================================

#ifdef MOOTDEBUG

int __refcnt(SV *sv)
CODE:
 if (sv && SvOK(sv)) {
   RETVAL = SvREFCNT(sv);
 } else {
   XSRETURN_UNDEF;
 }
OUTPUT:
 RETVAL

#endif


##=====================================================================
## Moot: submodules
INCLUDE: Constants.xs
INCLUDE: Lexfreqs.xs
## INCLUDE: Ngrams.xs
