/* just include the config.h for libmoot */
#include <mootConfig.h>

/* PACKAGE does not jive well with multiple-program pacakges
   using 'gengetopt'.  'autoheader' doesn't like to undefine
   it, so we include this extra little header file to do that
*/
/*
#ifdef PACKAGE
# undef PACKAGE
#endif
*/

//#endif // _moot_CONFIG_H

