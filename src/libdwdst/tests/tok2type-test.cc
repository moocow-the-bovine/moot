#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <string>
#include <vector>

using namespace std;

/*------------------------------------------------------------
 * globals
 */
char *progname;

timeval t1,t2;

/*------------------------------------------------------------
 * token2type
 */
/** Enum typedef for token2id */
typedef enum {
  TokTypeAlpha,
  TokTypeCard,
  TokTypeCardPunct,
  TokTypeCardSuffix,
  TokTypeCardSeps,
  TokTypeEnd
} TokenType;

const char *type2name[TokTypeEnd] =
  {
    "TokTypeAlpha",
    "TokTypeCard",
    "TokTypeCardPunct",
    "TokTypeCardSuffix",
    "TokTypeCardSeps"
  };


const char *DIGITS = "0123456789";

inline TokenType token2type1(const string &token) {
  //if (token.empty()) return TokTypeAlpha;
  //size_t len = token.size();
  size_t pos = token.find_first_of(DIGITS, 0);
  if (pos == token.npos) return TokTypeAlpha;
  else if (pos == 0) {
    pos = token.find_first_not_of(DIGITS, pos);
    if (pos == token.npos) return TokTypeCard;
    else if (pos == token.size()-1) return TokTypeCardPunct;
    while (ispunct(token[pos]) && pos != token.npos) {
      pos = token.find_first_not_of(DIGITS, pos+1);
    }
    if (pos == token.npos) return TokTypeCardSeps;
    while (isalpha(token[pos]) && pos != token.npos) {
      pos = token.find_first_not_of(DIGITS, pos+1);
    }
    if (pos == token.npos) return TokTypeCardSuffix;
  }
  //fprintf(stderr, "find_first_of(DIGITS, 0, \"%s\")==%u\n", token.c_str(), pos);
  return TokTypeAlpha;
}

inline TokenType token2type(const string &token) {
  string::const_iterator ti = token.begin();

  if (ti==token.end() || !isdigit(*ti))
    return TokTypeAlpha;

  //-- ^[:digit:]
  for (ti++; ti != token.end() && isdigit(*ti); ti++) {;}  //-- find first non-digit
  //-- ^([:digit:]+)

  if (ti == token.end())  //-- ^([:digit:]+)$
    return TokTypeCard;

  else if (ispunct(*ti)) {
    //-- ^([:digit:]+)([:punct:])
    for (ti++; ti != token.end() && ispunct(*ti); ti++) {;}
    //-- ^([:digit:]+)([:punct:]+)

    if (ti == token.end())      //-- ^([:digit:]+)([:punct:]+)$
      return TokTypeCardPunct;

    else if (isdigit(*ti)) {
      //-- ^([:digit:]+)([:punct:]+)([:digit:])
      for (ti++; ti != token.end() && (isdigit(*ti) || ispunct(*ti)); ti++) {;}
      //-- ^([:digit:]+)([:punct:]+)(([:digit:]|[:punct:]+))
      if (ti == token.end())
	//-- ^([:digit:]+)([:punct:]+)(([:digit:]|[:punct:]+))$
	return TokTypeCardSeps;
    }
  }

  //-- ^([:digit:]+)([[:digit:][:punct]]*)([^[:digit:][:punct:]])
  for (ti++; ti != token.end() && !isdigit(*ti); ti++) {;}
  //-- ^([:digit:]+)([[:digit:][:punct]]*)([^[:digit:][:punct:]])([^[:digit]]*)

  if (ti == token.end())
    //-- ^([:digit:]+)([[:digit:][:punct]]*)([^[:digit:][:punct:]])([^[:digit]]*)$
    return TokTypeCardSuffix;

  return TokTypeAlpha;
}


/*----------------------------------------------------------------------
 * Benchmarking
 *----------------------------------------------------------------------*/
void benchinfo(char *name, unsigned iters, timeval started, timeval stopped)
{
  double elapsed = (stopped.tv_sec
		    - started.tv_sec
		    + (double)(stopped.tv_usec - started.tv_usec)
		    / 1000000.0);
  printf("%s: Benchmarked %u iterations of '%s':\n", progname, iters, name);
  printf("%s:   Elapsed : %g sec\n", progname, elapsed);
  printf("%s:     Rate  : %g iters/sec\n", progname, (double)iters / elapsed);
}


/*------------------------------------------------------------
 * MAIN
 */
#define BENCH_ITERS 65536

int main (int argc, char **argv) {
  progname = *argv;
  int i;
  vector<string> sv;
  TokenType typ;

  //-- prepare
  sv.resize(argc-1);
  for (i = 1; i < argc; i++) {
    sv[i-1] = argv[i];
  }

  //-- test
  for (vector<string>::const_iterator svi = sv.begin(); svi != sv.end(); svi++) {
    typ     = token2type(*svi);
    fprintf(stderr, "\t%s => %s\n", svi->c_str(), type2name[typ]);
  }


  //-- benchmark
  gettimeofday(&t1,NULL);
  unsigned u;
  for (u = 0; u < BENCH_ITERS; u++) {
    for (vector<string>::const_iterator svi = sv.begin(); svi != sv.end(); svi++) {
      typ = token2type(*svi);
    }
  }
  gettimeofday(&t2,NULL);
  benchinfo("token2type", BENCH_ITERS * sv.size(), t1, t2);

  return 0;
}
