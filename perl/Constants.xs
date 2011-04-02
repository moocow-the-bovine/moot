#/*-*- Mode: C -*- */

MODULE = Moot		PACKAGE = Moot   PREFIX = moot_

##=====================================================================
## Constants
##=====================================================================

##-- disable perl prototypes
PROTOTYPES: DISABLE

##--------------------------------------------------------------
## mootConfig.h
const char *
library_version()
 CODE:
   RETVAL=moot_version_string;
 OUTPUT:
   RETVAL

##--------------------------------------------------------------
## mootToken.h: mootTokenTypeE

mootTokenType
TokTypeUnknown()
CODE:
 RETVAL=moot::TokTypeUnknown;
OUTPUT:
 RETVAL

mootTokenType
TokTypeVanilla()
CODE:
 RETVAL=moot::TokTypeVanilla;
OUTPUT:
 RETVAL

mootTokenType
TokTypeLibXML()
CODE:
 RETVAL=moot::TokTypeLibXML;
OUTPUT:
 RETVAL

mootTokenType
TokTypeXMLRaw()
CODE:
 RETVAL=moot::TokTypeXMLRaw;
OUTPUT:
 RETVAL

mootTokenType
TokTypeComment()
CODE:
 RETVAL=moot::TokTypeComment;
OUTPUT:
 RETVAL

mootTokenType
TokTypeEOS()
CODE:
 RETVAL=moot::TokTypeEOS;
OUTPUT:
 RETVAL

mootTokenType
TokTypeEOF()
CODE:
 RETVAL=moot::TokTypeEOF;
OUTPUT:
 RETVAL

mootTokenType
TokTypeUser()
CODE:
 RETVAL=moot::TokTypeUser;
OUTPUT:
 RETVAL
