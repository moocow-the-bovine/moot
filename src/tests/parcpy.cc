#include <stdio.h>
#include "mootTrainer.h"
#include "mootParamCompiler.h"


// -- define this to use 'parcomp' directly (useful for debugging)
#define PARCPY_LOAD_FROM_COMPILER 1


/*--------------------------------------------------------------------------
 * globals
 *--------------------------------------------------------------------------*/
mootTrainer moott;

#ifdef PARCPY_LOAD_FROM_COMPILER
mootParamCompiler parcomp;
#endif


/*--------------------------------------------------------------------------
 * functions : loading
 *--------------------------------------------------------------------------*/
#ifdef PARCPY_LOAD_FROM_COMPILER
void parcpy_load_from_compiler(void) {
    fprintf(stderr, ">> parcpy: Loading using mootParamCompiler::parse_from_file()... \n");
    if (!parcomp.parse_from_file(stdin,"(stdin)")) {
	fprintf(stderr, ">> parcpy: Load FAILED.\n");
	exit(1);
    }
}
#endif // PARCPY_LOAD_FROM_COMPILER

void parcpy_load_from_trainer(void) {
    fprintf(stderr, ">> parcpy: Loading using mootTrainer::load_param_file()...\n");
    if (!moott.load_param_file(stdin,"(stdin)")) {
	fprintf(stderr, ">> parcpy: Load FAILED.\n");
	exit(1);
    }
}

/*--------------------------------------------------------------------------
 * functions : initialization
 *--------------------------------------------------------------------------*/
void parcpy_init_globals(void) {
    // -- initialization
#ifdef PARCPY_LOAD_FROM_COMPILER
    parcomp.objname = "(parcpy)mootParamCompiler";
    parcomp.ngtable = &moott.ngtable;
    parcomp.alltags = &moott.alltags;
    //parcomp.yydebug = 1;
#endif
}




int main (int argc, char **argv) {
    parcpy_init_globals();
    fprintf(stderr, ">> parcpy: initialized.\n");

# ifdef PARCPY_LOAD_FROM_COMPILER
    parcpy_load_from_compiler();
# else
    parcpy_load_from_trainer();
# endif // PARCPY_LOAD_FROM_COMPILER
    fprintf(stderr, ">> parcpy: Loaded %d N-Grams\n", moott.ngtable.size());

    fprintf(stderr, ">> parcpy: Saving using mootTrainer::save_param_file()...\n");
    if (!moott.save_param_file(stdout)) {
	fprintf(stderr, ">> parcpy: Save FAILED.\n");
	exit(2);
    }
    fprintf(stderr, ">> parcpy: Saved.\n");

    return 0;
}
