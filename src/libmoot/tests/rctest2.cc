#include <stdio.h>

#include <stdbool.h>
#include <recodext.h>

#include <string>

using namespace std;

int main (int argc, char **argv) {
  //char *progname = *argv;
  RECODE_OUTER outer;
  RECODE_REQUEST req1;
  RECODE_REQUEST req2;

  char *buf1 = NULL;
  char *buf2 = NULL;
  size_t
    used1 =0,
    alloc1 =0,
    used2 =0,
    alloc2 =0;

  outer = recode_new_outer(1);

  req1  = recode_new_request(outer);
  req2  = recode_new_request(outer);

  /*-- ok */
  req1->diacritics_only = false;
  req2->diacritics_only = true;


  const char *req1s = "latin1..HTML-i18n";
  const char *req2s = "HTML-i18n..UTF-8";

  recode_scan_request(req1,req1s);
  recode_scan_request(req2,req2s);

  /*-- NOT ok!
  req1->diacritics_only = false;
  req2->diacritics_only = true;
  */

  fprintf(stderr, "rc1:[reqstr=\"%s\", dia_only=%d]\n", req1s, req1->diacritics_only);
  fprintf(stderr, "rc2:[reqstr=\"%s\", dia_only=%d]\n", req2s, req2->diacritics_only);


  for (argc--,argv++; argc > 0; argc--, argv++) {
    //-- by hand
    printf(" -----\n");
    bool rv1 = recode_string_to_buffer(req1,*argv, &buf1,&used1,&alloc1);
    printf("rc1.s2b(\"%s\") = \"%s\" [returned=%d]\n", *argv, buf1, rv1);

    bool rv2 = recode_buffer_to_buffer(req2,buf1,used1, &buf2,&used2,&alloc2);
    printf("rc2.b2b(\"%s\") = \"%s\" [returned=%d]\n", buf1, buf2, rv2);
  }
  
  return 0;
}
