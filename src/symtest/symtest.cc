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

/*
 * type_inference("Case") => "ADJ", "NOUN", ...
 *   + from feature-supertypes ("feat") to category-types
 *     ("cat") for which "feat" is appropriate
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

/*
 * subtypes_of("Case") => "none", "nom", "acc", ...
 *   + from supertypes to subtypes
 *   + recursive ???
 */
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
 * supertypes_of("dat") => "Case"
 *   + from subtypes to supertypes
 */
void test_supertypes_of(const char *type) {
  FSMSymbolString s = type;
  vector<FSMSymbolString> *results;

  results = syms->supertypes_of(s);
  cout << "supertypes_of(" << s << ") ="
       << (results->empty() ? " --empty--\n" : "\n");
  for (vector<FSMSymbolString>::iterator r = results->begin(); r != results->end(); r++) {
    cout << "\t" << r->c_str() << "\n";
  }
}


/*
 * features_of_category("Noun") => "Gender", "Number", "Case" 
 *   + from categories to their feature-supertypes
 *   + BREAKS if 'cat' is not a category!
 */
void test_features_of_cat(const char *cat) {
  FSMSymbolString s = cat;
  const vector<FSMSymbolString> *results = syms->features_of_category(s);

  cout << "features_of_category(" << s << ") ="
       << (results->empty() ? " --empty--\n" : "\n");
  for (vector<FSMSymbolString>::const_iterator r = results->begin(); r != results->end(); r++) {
    cout << "\t" << r->c_str() << "\n";
  }
}


/*
 * signs() => "A".."Z", "NOUN", "VERB", ...   // 1063 lines
 *   + NOT i.e. "Case"
 */
void test_signs() {
  //FSMSymbolString s = cat;
  const set<FSMSymbolString> *results = syms->signs();

  cout << "signs() = "
       << (results->empty() ? " --empty--\n" : "\n");
  for (set<FSMSymbolString>::const_iterator r = results->begin(); r != results->end(); r++) {
    cout << "\t" << r->c_str() << "\n";
  }
}


/*
 * symbols() => "A".."Z", "NOUN", ... // 1085 lines
 *   + probably every single darned symbol
 */
void test_symbols() {
  //FSMSymbolString s = cat;
  const set<FSMSymbolString> *results = syms->symbols();

  cout << "symbols() = "
       << (results->empty() ? " --empty--\n" : "\n");
  for (set<FSMSymbolString>::const_iterator r = results->begin(); r != results->end(); r++) {
    cout << "\t" << r->c_str() << "\n";
  }
}


/*
 * categories1() => ABBREV ... NOUN ...  VZUINFIX  // 69 lines
 *  + gets all categories (HACK!)
 */
void test_categories1() {
  //FSMSymbolString s = cat;
  const set<FSMSymbolString> *symbols = syms->symbols();
  set <FSMSymbolString> cats = *symbols;
  set<FSMSymbolString>::iterator r;

  // -- set up cats
  for (r = cats.begin(); r != cats.end(); r++) {
    if (!syms->is_category(*r)) {
      cats.erase(r);
    }
  }

  cout << "categories1() = "
       << (cats.empty() ? " --empty--\n" : "\n");
  for (r = cats.begin(); r != cats.end(); r++) {
    cout << "\t" << r->c_str() << "\n";
  }
}


/*
 * categories2() => ABBREV ... NOUN ... VZUINFIX  // 64 lines
 *   + all categories with any features (?!)
 */
void test_categories2() {
  //FSMSymbolString s = cat;
  const map<FSMSymbolString,vector<FSMSymbolString> > *cfeats = syms->category_feats();
  map<FSMSymbolString,vector<FSMSymbolString> >::const_iterator r;

  cout << "categories2() = "
       << (cfeats->empty() ? " --empty--\n" : "\n");
  for (r = cfeats->begin(); r != cfeats->end(); r++) {
    cout << "\t" << r->first.c_str() << "\n";
  }
}


/*
 * test_name2sym() => 
 */
void test_name2sym(const char *name) {
  FSMSymbolString s = name;
  FSMSymbol num = syms->symbolname_to_symbol(s);
  cout
    << "symbolname_to_symbol(" << s << ") = "
    << num << "\n";
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

  // ??
  //syms->save_att_label_file("dwdst.lab");


  while (--argc > 0) {
    ++argv;

    //test_type_inference(*argv);
    //test_subtypes_of(*argv);
    //test_supertypes_of(*argv);
    test_features_of_cat(*argv);
    //test_name2sym(*argv);
  }
  //test_signs();
  //test_symbols();
  //test_categories1();
  //test_categories2();
}
