#/*-*- Mode: C++ -*- */

MODULE = Moot		PACKAGE = Moot::Lexfreqs

##=====================================================================
## Constructors etc.
##=====================================================================

##-- disable perl prototypes
PROTOTYPES: DISABLE

##--------------------------------------------------------------
## Constructor: new()
mootLexfreqs*
new(char *CLASS)
CODE:
 RETVAL=new mootLexfreqs();
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## clear
void
clear(mootLexfreqs* lf)
CODE:
 lf->clear();

##--------------------------------------------------------------
## Destructor: DESTROY()
void
DESTROY(mootLexfreqs* lf)
CODE:
 if (lf) delete lf;


##=====================================================================
## Accessors
##=====================================================================

##--------------------------------------------------------------
## compute specials
void
compute_specials(mootLexfreqs *lf)
CODE:
   lf->compute_specials();

##--------------------------------------------------------------
## n_pairs
size_t
n_pairs(mootLexfreqs *lf)
CODE:
 RETVAL = lf->n_pairs();
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## n_tokens
size_t
n_tokens(mootLexfreqs *lf)
CODE:
 RETVAL = lf->n_tokens;
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## add_count
void
add_count(mootLexfreqs *lf, char *word, char *tag, double count)
CODE:
 lf->add_count(word,tag,count);


##--------------------------------------------------------------
## lookup: f(tag)
CountT
f_tag(mootLexfreqs *lf, char *tag)
CODE:
 RETVAL = lf->taglookup(tag);
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## lookup: f(word)
CountT
f_word(mootLexfreqs *lf, char *word)
CODE:
 moot::mootLexfreqs::LexfreqTokTable::const_iterator lfi = lf->lftable.find(word);
 RETVAL = (lfi==lf->lftable.end() ? 0 : lfi->second.count);
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## lookup: f(word,tag)
CountT
f_word_tag(mootLexfreqs *lf, char *word, char *tag)
CODE:
 RETVAL = 0;
 moot::mootLexfreqs::LexfreqTokTable::const_iterator lfi = lf->lftable.find(word);
 if (lfi != lf->lftable.end()) {
   moot::mootLexfreqs::LexfreqSubtable::const_iterator lsi = lfi->second.freqs.find(tag);
   if (lsi != lfi->second.freqs.end()) RETVAL = lsi->second;
 }
OUTPUT:
 RETVAL


##--------------------------------------------------------------
## I/O: File

bool
loadFile(mootLexfreqs *lf, char *filename)
CODE:
 RETVAL = lf->load(filename);
OUTPUT:
 RETVAL

bool
saveFile(mootLexfreqs *lf, char *filename)
CODE:
 RETVAL = lf->save(filename);
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## I/O: FH

bool
loadFh(mootLexfreqs *lf, FILE *f, char *filename=NULL)
CODE:
 RETVAL = lf->load(f,filename);
OUTPUT:
 RETVAL

bool
saveFh(mootLexfreqs *lf, FILE *f, char *filename=NULL)
CODE:
 RETVAL = lf->save(f,filename);
OUTPUT:
 RETVAL
