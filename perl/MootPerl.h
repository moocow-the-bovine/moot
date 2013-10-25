/*-*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*-*/
extern "C" {
 #include "EXTERN.h"
 #include "perl.h"
 #include "XSUB.h"
};
//#include "ppport.h"

#include <moot.h>

/*======================================================================
 * Debugging
 */
/*#define MOOTDEBUG 1*/

#if defined(MOOTDEBUG) || defined(MOOT_PERL_DEBUG)
# define MOOT_PERL_DEBUG_EVAL(code_) code_
#else
# define MOOT_PERL_DEBUG_EVAL(code_)
#endif

/*======================================================================
 * typedefs
 */
//typedef moot::mootLexfreqs mootLexfreqs;
//typedef moot::mootNgrams   mootNgrams;
typedef moot::mootHMM::TagID   TagID;
typedef moot::mootHMM::TagID   TokID;
typedef moot::mootHMM::ClassID ClassID;
typedef moot::mootTokString    TokStr;
typedef moot::mootTagString    TagStr;

/*======================================================================
 * Constants
 */
extern const char *moot_version_string;

/*======================================================================
 * Conversions: copy
 */
HV*           token2hv(const mootToken *tok, U32 utf8=TRUE);        //-- copies: type,text,tag,analyses
mootToken    *hv2token(HV *hv, mootToken *tok=NULL, U32 utf8=TRUE); //-- copies: type,text,tag,analyses,data=hv

AV*           sentence2av(const mootSentence *s, U32 utf8=TRUE);
mootSentence *av2sentence(AV *av, mootSentence *s=NULL, U32 utf8=TRUE);

mootTagSet *av2tagset(AV *tsav, mootTagSet *tagset, U32 utf8=TRUE);

/*======================================================================
 * Conversions: in-place
 */
void sentence2tokdata(mootSentence *s, U32 utf8=TRUE);

/*======================================================================
 * mootio stream wrappers for I/O on perl scalars ::: TODO
 */
/*
class mootPerlBuffer : virtual public mootio::mcbuffer
{
public:
  SV *sv; //-- underlying sv
  
};
*/
