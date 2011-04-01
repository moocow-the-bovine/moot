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
add_count(mootLexfreqs *lf, char *text, char *tag, double count)
CODE:
 lf->add_count(text,tag,count);


##--------------------------------------------------------------
## I/O

bool
load(mootLexfreqs *lf, char *filename)
CODE:
 RETVAL = lf->load(filename);
OUTPUT:
 RETVAL

bool
save(mootLexfreqs *lf, char *filename)
CODE:
 RETVAL = lf->save(filename);
OUTPUT:
 RETVAL
