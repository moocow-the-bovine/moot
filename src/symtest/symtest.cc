#include <iostream.h>
#include <FSMSymSpec.h>

using namespace std;

/*
 * GLOBALS
 */
FSMSymSpec *syms;
#define ATT_SYM_COMPAT true

/*
 * TESTS
 */
void test_type_inference(const char *feat) {
  FSMSymbolString s = feat;
  set<FSMSymbolString> *results;

  results = syms->type_inference(feat);
  cout << "type_inference(" << feat << ") ="
       << (results->empty() ? " --empty--\n" : "\n");
  for (set<FSMSymbolString>::iterator r = results->begin(); r != results->end(); r++) {
    cout << "\t" << *r << "\n";
  }
}

void test_subtypes_of(const char *type) {
  FSMSymbolString s = type;
  set<FSMSymbol> *results;

  results = syms->subtypes_of(s);
  cout << "subtypes_of(" << s << ") ="
       << (results->empty() ? " --empty--\n" : "\n");
  for (set<FSMSymbol>::iterator r = results->begin(); r != results->end(); r++) {
    cout << "\t" << syms->symbol_to_symbolname(*r)->c_str() << "\n";
  }
}



/*
 * MAIN
 */
int main (int argc, char **argv) {
  list<string> msglist;
  syms = new FSMSymSpec("dwdst.sym", &msglist, ATT_SYM_COMPAT);

  if (!msglist.empty()) {
    cerr << *argv << " Error loading symbols file 'dwdst.sym'\n";
    abort();
  }

  while (--argc > 0) {
    ++argv;

    test_type_inference(*argv);
    //test_subtypes_of(*argv);
  }
}
