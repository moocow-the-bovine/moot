#include "ziotest.h"


/*--------------------------------------------------------------
 * MAIN
 */

//--ok
/*
#define ICLASS icBinStream
#define OCLASS ocBinStream
*/

//--ok
/*
#define ICLASS iccBinStream
#define OCLASS occBinStream
*/

//-- ok
/*
#define ICLASS izBinStream
#define OCLASS ozBinStream
*/

//-- ok (sweet!)
#define ICLASS izBinStream
#define OCLASS ocBinStream

using namespace dwdstBinIO;

int main (int argc, char **argv) {
  char *progname = *argv++;
  if (argc < 2) {
    fprintf(stderr, "Usage: %s FILENAME\n", progname);
    exit(1);
  }
  char *filename = *argv++;
  FILE *file = NULL;

  /*------------------------------------------------------------
   * Save
   */
  fprintf(stderr, "%s: testing save...\n", progname);

  //-- prepare
  file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "%s: save: open failed for file '%s': %s\n",
	    progname, filename, strerror(errno));
    exit(2);
  }

  //-- prepare hack class
  OCLASS oc;
  oc.dopen(fileno(file));

  /*--------------------------------------------------
   * save: int
   */
  fprintf(stderr, "%s: testing save<int> [0..3] ...", progname);
  Item<int> bin_int;
  int i;
  for (i = 0; i < 3; i++) {
    if (!bin_int.save(oc, i)) {
      fprintf(stderr, "%s: save<int> failed for i=%d", progname, i);
      exit(3);
    }
  }
  fprintf(stderr, " ok.\n");

  /*--------------------------------------------------
   * save: char *
   */
  fprintf(stderr, "%s: testing save<char *> [foo,bar,baz] ...", progname);
  Item<char *> bin_cstr;
  const char * cstrs[] = {"foo","bar","baz"};
  for (i = 0; i < 3; i++) {
    if (!bin_cstr.save(oc, cstrs[i])) {
      fprintf(stderr, "%s: save<char *> failed for s=%s", progname, cstrs[i]);
      exit(3);
    }
  }
  fprintf(stderr, " ok.\n");


  /*--------------------------------------------------
   * save: string
   */
  fprintf(stderr, "%s: testing save<string> [foo,bar,baz] ...", progname);
  Item<string> bin_string;
  const string strs[3] = {string("foo"), string("bar"), string("baz")};
  for (i = 0; i < 3; i++) {
    if (!bin_string.save(oc, strs[i])) {
      fprintf(stderr, "%s: save<string> failed for s=%s", progname, strs[i].c_str());
      exit(3);
    }
  }
  fprintf(stderr, " ok.\n");


  //-- cleanup
  oc.close();
  fclose(file);


  /*------------------------------------------------------------
   * Load
   */
  //-- prepare
  fprintf(stderr, "\n%s: testing load...\n", progname);
  file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "%s: load: open failed for file '%s': %s\n",
	    progname, filename, strerror(errno));
    exit(2);
  }

  //-- hack
  ICLASS ic;
  ic.dopen(fileno(file));

  /*--------------------------------------------------
   * load: int
   */
  fprintf(stderr, "%s: testing load<int> [0..3] ... ", progname);
  int j;
  for (i = 0; i < 3; i++) {
    if (!bin_int.load(ic, j)) {
      fprintf(stderr, "%s: load<int> failed for i=%d", progname, i);
      exit(4);
    }
    if (j != i) {
      fprintf(stderr, "%s: load<int> for i=%d returned j=%d != i\n", progname, i, j);
      exit(5);
    }
  }
  fprintf(stderr, " ok.\n");


  /*--------------------------------------------------
   * load: char *
   */
  fprintf(stderr, "%s: testing load<char *> [foo,bar,baz] ... ", progname);
  for (i = 0; i < 3; i++) {
    char *buf = NULL;
    if (!bin_cstr.load(ic, buf)) {
      fprintf(stderr, "%s: load<char *> failed for src=%s", progname, cstrs[i]);
      exit(4);
    }
    if (strcmp(cstrs[i],buf) != 0) {
      fprintf(stderr, "%s: load<char *> for src='%s' returned buf='%s' != src\n",
	      progname, cstrs[i], buf);
      exit(5);
    }
    if (buf) free(buf);
  }
  fprintf(stderr, " ok.\n");


  /*--------------------------------------------------
   * load: string
   */
  fprintf(stderr, "%s: testing load<string> [foo,bar,baz] ... ", progname);
  for (i = 0; i < 3; i++) {
    string s;
    if (!bin_string.load(ic, s)) {
      fprintf(stderr, "%s: load<string> failed for src=%s", progname, strs[i].c_str());
      exit(4);
    }
    if (s != strs[i]) {
      fprintf(stderr, "%s: load<string> for src='%s' returned buf='%s' != src\n",
	      progname, strs[i].c_str(), s.c_str());
      exit(5);
    }
  }
  fprintf(stderr, " ok.\n");


  //-- cleanup
  ic.close();
  fclose(file);


  return 0;
}
