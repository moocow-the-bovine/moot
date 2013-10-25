#/*-*- Mode: C++ -*- */

MODULE = Moot		PACKAGE = Moot::Waste::Scanner

##=====================================================================
## Constructors etc.
##=====================================================================

##-- disable perl prototypes
PROTOTYPES: DISABLE

##--------------------------------------------------------------
## Constructor: new()
wasteTokenScanner*
new(char *CLASS)
CODE:
  RETVAL=new wasteTokenScanner( (tiofText|tiofTagged|tiofLocation), CLASS );
OUTPUT:
  RETVAL

##--------------------------------------------------------------
## Destructor: DESTROY()
void
DESTROY(wasteTokenScanner* wts)
CODE:
 if (wts) delete wts;

##--------------------------------------------------------------
## reset scanner to initial state
void
reset(wasteTokenScanner* wts)
CODE:
 wts->scanner.reset();

##=====================================================================
## Input Selection
##=====================================================================

##--------------------------------------------------------------
## close currently opened stream, if any
void
close(wasteTokenScanner* wts)
CODE:
 wts->close();

##--------------------------------------------------------------
## returns true iff stream is opened
int
opened(wasteTokenScanner* wts)
CODE:
 RETVAL = wts->opened() ? 1 : 0;
OUTPUT:
 RETVAL

##--------------------------------------------------------------
void
from_fh(wasteTokenScanner* wts, FILE *fp)
CODE:
 wts->from_file(fp);

##--------------------------------------------------------------
void
from_file(wasteTokenScanner* wts, const char *filename)
CODE:
 wts->from_filename(filename);

##=====================================================================
## Token-Level Access
##=====================================================================

##--------------------------------------------------------------
HV*
get_token(wasteTokenScanner* wts)
PREINIT:
  mootTokenType toktyp;
CODE:
  toktyp = wts->get_token();
  if (toktyp==TokTypeEOF) {
    XSRETURN_UNDEF;
  }
  else RETVAL = token2hv( wts->token() );
OUTPUT:
  RETVAL


##=====================================================================
## Diagnostics
##=====================================================================

##--------------------------------------------------------------
const char *
name(wasteTokenScanner* wts, ...)
CODE:
 if (items > 1) {
    const char *myname = SvPV_nolen( ST(1) );
   wts->tr_name = myname;
 }
 RETVAL = wts->tr_name.c_str();
OUTPUT:
 RETVAL

##--------------------------------------------------------------
size_t
line_number(wasteTokenScanner* wts, ...)
CODE:
 if (items > 1) {
   size_t n = (size_t)SvUV( ST(1) );
   wts->line_number(n);
 }
 RETVAL = wts->line_number();
OUTPUT:
 RETVAL

##--------------------------------------------------------------
size_t
column_number(wasteTokenScanner* wts, ...)
CODE:
 if (items > 1) {
   size_t n = (size_t)SvUV( ST(1) );
   wts->column_number(n);
 }
 RETVAL = wts->column_number();
OUTPUT:
 RETVAL

##--------------------------------------------------------------
size_t
byte_number(wasteTokenScanner* wts, ...)
CODE:
 if (items > 1) {
   size_t n = (size_t)SvUV( ST(1) );
   wts->byte_number(n);
 }
RETVAL = (size_t)wts->byte_number();
OUTPUT:
 RETVAL

