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
HV*           token2hv(const mootToken *tok);        //-- copies: type,text,tag,analyses
mootToken    *hv2token(HV *hv, mootToken *tok=NULL); //-- copies: type,text,tag,analyses,data=hv

AV*           sentence2av(const mootSentence *s);
mootSentence *av2sentence(AV *av, mootSentence *s=NULL);

/*======================================================================
 * Conversions: in-place
 */
void sentence2tokdata(mootSentence *s);
