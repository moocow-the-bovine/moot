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
 //fprintf(stderr, "%s::new() --> %p=%i\n", CLASS,RETVAL,RETVAL);
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
 //fprintf(stderr, "%s::new() --> %p=%i\n", CLASS,RETVAL,RETVAL);
OUTPUT:
 RETVAL

##-------------------------------------------------------------
void
close(wasteLexer *wl)
CODE:
 wl->close();  
 if (wl->tr_data) {
   SvREFCNT_dec( (SV*)wl->tr_data );
   wl->tr_data = NULL;
 }

##-------------------------------------------------------------
##int
##_scanner_refcnt(wasteLexer *wl)
##CODE:
## if (wl->tr_data) {
##   RETVAL = SvREFCNT((SV*)SvRV((SV*)wl->tr_data));
## } else {
##   RETVAL = -1;
## }
##OUTPUT:
## RETVAL

##-------------------------------------------------------------
SV*
_get_scanner(wasteLexer *wl)
CODE:
 if (!wl->tr_data || !wl->scanner) { XSRETURN_UNDEF; }
 RETVAL = newSVsv((SV*)wl->tr_data);
OUTPUT:
 RETVAL

##-------------------------------------------------------------
void
_set_scanner(wasteLexer *wl, SV *scanner_sv)
PREINIT:
  TokenReader *tr;
CODE:
  if( sv_isobject(scanner_sv) && (SvTYPE(SvRV(scanner_sv)) == SVt_PVMG) )
    tr = (TokenReader*)SvIV((SV*)SvRV( scanner_sv ));
  else {
    warn("Moot::Waste::Lexer::_set_scanner() -- scanner_sv is not a blessed SV reference");
    XSRETURN_UNDEF;
  }
  wl->from_reader(tr);
  wl->tr_data = newSVsv(scanner_sv);

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
## NO standalone objects!!!
#wasteLexicon*
#new(char *CLASS)
#CODE:
#  RETVAL=new wasteLexicon();
#OUTPUT:
#  RETVAL

##--------------------------------------------------------------
## NO standalone objects!!!
#void
#DESTROY(wasteLexicon* lx)
#CODE:
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


##=====================================================================
## Moot::Waste::Decoder
## + uses TokenWriter::tw_data to hold SV* of underlying writer
##=====================================================================

MODULE = Moot		PACKAGE = Moot::Waste::Decoder

##--------------------------------------------------------------
wasteDecoder*
new(char *CLASS, TokenIOFormatMask fmt=tiofUnknown)
CODE:
 RETVAL=new wasteDecoder(fmt, CLASS);
 //fprintf(stderr, "%s::new() --> %p=%i\n", CLASS,RETVAL,RETVAL);
OUTPUT:
 RETVAL

##-------------------------------------------------------------
void
close(wasteDecoder *wd)
CODE:
 wd->close();  
 if (wd->tw_data) {
   SvREFCNT_dec( (SV*)wd->tw_data );
   wd->tw_data = NULL;
 }

##-------------------------------------------------------------
SV*
_get_sink(wasteDecoder *wd)
CODE:
 if (!wd->tw_data || !wd->wd_sink) { XSRETURN_UNDEF; }
 RETVAL = newSVsv((SV*)wd->tw_data);
OUTPUT:
 RETVAL

##-------------------------------------------------------------
void
_set_sink(wasteDecoder *wd, SV *sink_sv)
PREINIT:
  TokenWriter *tw;
CODE:
  if( sv_isobject(sink_sv) && (SvTYPE(SvRV(sink_sv)) == SVt_PVMG) )
    tw = (TokenWriter*)SvIV((SV*)SvRV( sink_sv ));
  else {
    warn("Moot::Waste::Decoder::_set_sink() -- sink_sv is not a blessed SV reference");
    XSRETURN_UNDEF;
  }
  wd->to_writer(tw);
  wd->tw_data = newSVsv(sink_sv);
