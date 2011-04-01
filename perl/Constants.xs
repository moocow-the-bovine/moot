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
