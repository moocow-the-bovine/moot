#/*-*- Mode: C++; c-basic-offset: 2; -*- */

##=====================================================================
## Moot::Waste::Scanner
##=====================================================================

MODULE = Moot		PACKAGE = Moot::Waste::Scanner

##--------------------------------------------------------------
wasteTokenScanner*
new(char *CLASS, TokenIOFormatMask fmt=tiofMedium|tiofLocation)
CODE:
  RETVAL=new wasteTokenScanner(fmt, CLASS);
OUTPUT:
  RETVAL

##--------------------------------------------------------------
void
reset(wasteTokenScanner* wts)
CODE:
 wts->scanner.reset();


##=====================================================================
## Moot::Waste::Lexer
## + uses TokenReader::tr_data to hold SV* of underlying reader
##=====================================================================

MODULE = Moot		PACKAGE = Moot::Waste::Lexer

##--------------------------------------------------------------
wasteLexer*
new(char *CLASS, TokenIOFormatMask fmt=tiofUnknown)
CODE:
  RETVAL=new wasteLexer(fmt, CLASS);
OUTPUT:
  RETVAL

##-------------------------------------------------------------
void
close(wasteLexer *wl)
CODE:
 wl->close();  
 if (wl->tr_data) {
   SvREFCNT_dec( (SV*)wl->tr_data );
 }

##-------------------------------------------------------------
void
_from_reader(wasteLexer *wl, SV *reader)
PREINIT:
  TokenReader *tr;
CODE:
  if( sv_isobject(reader) && (SvTYPE(SvRV(reader)) == SVt_PVMG) )
    tr = (TokenReader*)SvIV((SV*)SvRV( reader ));
  else {
    warn("Moot::Waste::Lexer::from_reader() -- reader is not a blessed SV reference");
    XSRETURN_UNDEF;
  }
  wl->from_reader(tr);
  wl->tr_data = reader;

##--------------------------------------------------------------
bool
dehyphenate(wasteLexer* wl, ...)
CODE:
 if (items > 1) {
   bool on = (bool)SvTRUE( ST(1) );
   wl->dehyph_mode(on);
 }
 RETVAL = wl->wl_dehyph_mode;
OUTPUT:
 RETVAL

##--------------------------------------------------------------
wasteLexicon*
stopwords(wasteLexer* wl)
PREINIT:
 const char *CLASS="Moot::Waste::Lexicon";
CODE:
 RETVAL = &wl->wl_stopwords;
OUTPUT:
 RETVAL

##--------------------------------------------------------------
wasteLexicon*
abbrevs(wasteLexer* wl)
PREINIT:
 const char *CLASS="Moot::Waste::Lexicon";
CODE:
 RETVAL = &wl->wl_abbrevs;
OUTPUT:
 RETVAL

##--------------------------------------------------------------
wasteLexicon*
conjunctions(wasteLexer* wl)
PREINIT:
 const char *CLASS="Moot::Waste::Lexicon";
CODE:
 RETVAL = &wl->wl_conjunctions;
OUTPUT:
 RETVAL


##=====================================================================
## Moot::Waste::Lexicon
## - NO standalone objects allowed: always accessed via wasteLexer (so we can skip ref-counting)
##=====================================================================

MODULE = Moot		PACKAGE = Moot::Waste::Lexicon

##--------------------------------------------------------------
#wasteLexicon*
#new(char *CLASS)
#CODE:
#  RETVAL=new wasteLexicon();
#  fprintf(stderr, "Waste::Lexicon::new(lx=%p)\n", RETVAL);
#OUTPUT:
#  RETVAL

##--------------------------------------------------------------
#void
#DESTROY(wasteLexicon* lx)
#CODE:
# fprintf(stderr, "Waste::Lexicon::DESTROY(lx=%p)\n", lx);
# //if (lx) delete lx;

##--------------------------------------------------------------
void
clear(wasteLexicon* lx)
CODE:
 lx->clear();

##--------------------------------------------------------------
size_t
size(wasteLexicon* lx)
CODE:
 RETVAL = lx->lex.size();
OUTPUT:
 RETVAL

##--------------------------------------------------------------
void
insert(wasteLexicon* lx, const char *str)
CODE:
 lx->insert(str);

##--------------------------------------------------------------
bool
lookup(wasteLexicon* lx, const char *str)
CODE:
 RETVAL = lx->lookup(str);
OUTPUT:
 RETVAL

##--------------------------------------------------------------
bool
_load_reader(wasteLexicon* lx, TokenReader *reader)
CODE:
 RETVAL = lx->load(reader);
OUTPUT:
 RETVAL
 
##--------------------------------------------------------------
bool
_load_file(wasteLexicon* lx, const char *filename)
CODE:
 RETVAL = lx->load(filename);
OUTPUT:
 RETVAL

##--------------------------------------------------------------
AV*
to_array(wasteLexicon* lx, bool utf8=TRUE)
CODE:
 RETVAL = newAV();
 for (wasteLexicon::Lexicon::const_iterator lxi=lx->lex.begin(); lxi!=lx->lex.end(); ++lxi) {
   SV *sv = stdstring2sv(*lxi, utf8);
   av_push(RETVAL, sv);
 }
OUTPUT:
 RETVAL
