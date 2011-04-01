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

/*======================================================================
 * Constants
 */
extern const char *moot_version_string;
