#include <stdio.h>
#include <string>

#include "mootRecode.h"

using namespace std;
using namespace moot;

#define FULL_WAHOOZLE

#define SRC_ENCODING "UTF-8"

//#define MED_ENCODING "HTML_4.0"
#define MED_ENCODING ""

//#define DST_ENCODING "latin1"
#define DST_ENCODING "XML-standalone"

int main (int argc, char **argv) {
  //char *progname = *argv;
#ifdef FULL_WAHOOZLE
  mootXMLRecoder recoder;
  string recoded;
#else
  mootRecoder rc1, rc2;
  string recoded1;
  string recoded2;
#endif

#ifdef FULL_WAHOOZLE
  recoder.scan_request(SRC_ENCODING,DST_ENCODING);
  fprintf(stderr, "recoder: standalone=%d ; rc1.diacritics_only=%d ; rc2.diacritics_only=%c\n",
	  recoder.standalone,
	  (recoder.rc1.rc_request ? (recoder.rc1.rc_request->diacritics_only ? 'y' : 'n') : '-'),
	  (recoder.rc2.rc_request ? (recoder.rc2.rc_request->diacritics_only ? 'y' : 'n') : '-')
	  );

#elif 0
  const char *req1 = SRC_ENCODING ".." MED_ENCODING;
  const char *req2 = MED_ENCODING ".." DST_ENCODING;
  rc1.scan_request(req1, false);
  rc2.scan_request(req2, true);
  fprintf(stderr, "rc1:[reqstr=\"%s\", dia_only=%d]\n", req1, rc1.rc_request->diacritics_only);
  fprintf(stderr, "rc2:[reqstr=\"%s\", dia_only=%d]\n", req2, rc2.rc_request->diacritics_only);
#else
  rc1.scan_request(SRC_ENCODING, MED_ENCODING, false);
  rc2.scan_request(MED_ENCODING, DST_ENCODING, true);
  fprintf(stderr, "rc1:[reqstr=\"%s\", dia_only=%d]\n",
	  rc1.rc_reqstr.c_str(), rc1.rc_request->diacritics_only);
  fprintf(stderr, "rc2:[reqstr=\"%s\", dia_only=%d]\n",
	  rc2.rc_reqstr.c_str(), rc2.rc_request->diacritics_only);
#endif


  for (argc--,argv++; argc > 0; argc--, argv++) {
#  ifdef FULL_WAHOOZLE
    //-- full wahoozle
    recoded.clear();
    bool rv = recoder.string2string(*argv,recoded);
    printf("recoder.s2s(\"%s\") = \"%s\" [returned=%d]\n", *argv, recoded.c_str(), rv);
#elif 0
    //-- hybrid: step-by-step
    printf(" -----\n");
    recoded1.clear();
    bool rv1 = recoder.rc1.string2string(*argv,recoded1);
    printf("recoder.rc1.s2s(\"%s\") = \"%s\" [returned=%d]\n",
	   *argv, recoded1.c_str(), rv1);

    recoded2.clear();
    bool rv2 = recoder.rc2.string2string(recoded1,recoded2);
    printf("recoder.rc2.s2s(\"%s\") = \"%s\" [returned=%d]\n",
	   recoded1.c_str(), recoded2.c_str(), rv2);
#else
    //-- by hand
    printf(" -----\n");
    recoded1.clear();
    bool rv1 = rc1.string2string(*argv,recoded1);
    printf("recoder.rc1.s2s(\"%s\") = \"%s\" [returned=%d]\n",
	   *argv, recoded1.c_str(), rv1);

    recoded2.clear();
    bool rv2 = rc2.string2string(recoded1,recoded2);
    printf("recoder.rc2.s2s(\"%s\") = \"%s\" [returned=%d]\n",
	   recoded1.c_str(), recoded2.c_str(), rv2);
#endif // FULL_WAHOOZLE
  }
  
  return 0;
}
