#include <stdio.h>
#include <stdlib.h>
#include "moot_param_lexer.h"

// -- globals
moot_param_lexer plex;


// -- churn
void plex_churn() {
    int tok;
    plex.theColumn = 0;
    plex.theLine = 0;
    while ((tok = plex.yylex())) {
	switch (tok) {
	case moot_param_lexer::PF_REGEX:
	    fprintf(stderr, "regex: '%s'\n", plex.yytext);
	    break;
	case moot_param_lexer::PF_COUNT:
	    fprintf(stderr, "count: '%s' = %f\n", plex.yytext, atof((const char *)plex.yytext));
	    break;
	case moot_param_lexer::PF_NEWLINE:
	    fprintf(stderr, "newline\n");
	    break;
	default:
	    fprintf(stderr, "UNKNOWN token: type=%d, text='%s'\n", tok, plex.yytext);
	}
    }
}



int main (int argc, char **argv) {
    int i;
    if (argc > 1) {
	for (i = 1; i < argc; i++) {
	    fprintf(stderr, ">>> Parsing string '%s'\n", argv[i]);
	    plex.select_string(argv[i]);
	    plex_churn();
	}
    } else {
	fprintf(stderr, ">>> Parsing stream (stdin)\n");
	plex.select_streams(stdin,stdout);
	plex_churn();
    }
    return 0;
}
