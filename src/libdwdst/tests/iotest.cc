#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <string>


using namespace std;

/*------------------------------------------------------------
 * Embedding class: binIO
 */

namespace binIO {
  using namespace std;

  /*------------------------------------------------------------
   * Generic items
   */
  template<class T> class Item {
  public:
    /** Load a single item */
    inline bool load(FILE *file, T &x) const
    {
      return fread(&x, sizeof(T), 1, file)==1;
    };

    /** Save a single item */
    inline bool save(FILE *file, const T &x) const
    {
      return fwrite(&x, sizeof(T), 1, file)==1;
    };

    /** Load C-array of items.
     *  'n' should hold the currently allocated length of 'x'.
     *  If the saved length is > n, 'x' will be re-allocated.
     *  The new size of the array will be stored in 'n' at completion.
     */
    inline bool load_n(FILE *file, T *&x, size_t &n) const {
      //-- get saved size
      Item<size_t> size_item;
      size_t saved_size;
      if (!size_item.load(file, saved_size)) return false;

      //-- re-allocate if necessary
      if (saved_size > n) {
	if (x) free(x);
	x = (T *)malloc(saved_size);
	if (!x) {
	  n = 0;
	  return false;
	}
      }

      //-- read in items
      n = fread(x, sizeof(T), saved_size, file);
      return n==saved_size;
    };

    /** Save a C-array of items.
     *  'n' should hold the number of items in 'x', it will be stored too.
     */
    inline bool save_n(FILE *file, const T *x, size_t n) const {
      //-- get saved size
      Item<size_t> size_item;
      if (!size_item.save(file, n)) return false;

      //-- save items
      size_t nwritten = fwrite(x, sizeof(T), n, file);
      return nwritten==n;
    };
  };

  /*------------------------------------------------------------
   * C-strings
   */
  template<> class Item<char *> {
  public:
    Item<char> charItem;

  public:
    /** Load a C-string, if (x==NULL), the string will be allocated with malloc(). */
    inline bool load(FILE *file, char *&x) const
    {
      size_t len=0;
      return charItem.load_n(file,x,len);
    };
  
    inline bool save(FILE *file, const char *x) const
    {
      if (x) {
	size_t len = strlen(x)+1;
	return charItem.save_n(file,x,len);
      } else {
	return charItem.save_n(file,"",1);
      }
    };
  };

  /*------------------------------------------------------------
   * C++ strings
   */
  template<> class Item<string> {
  public:
    Item<char *> cstr_item;
  public:
    inline bool load(FILE *file, string &x) const
    {
      char *buf=NULL;
      bool rc = cstr_item.load(file, buf);
      if (rc) x = buf;
      if (buf) free(buf);
      return rc;
    };

    inline bool save(FILE *file, const string &x) const
    {
      return cstr_item.save(file, x.c_str());
    };
  };

  /*------------------------------------------------------------
   * STL: vectors
   */
  template<class ValT> class Item<vector<ValT> > {
  public:
    Item<ValT> val_item;
  public:
    inline bool load(FILE *file, vector<ValT> &x) const
    {
      //-- get saved size
      Item<size_t> size_item;
      size_t len;
      if (!size_item.load(file, len)) return false;

      //-- resize
      x.clear();
      x.reserve(len);

      //-- read in items
      while (len-- > 0) {
	x.push_back(ValT());
	if (!val_item.load(file,x.back())) return false;
      }
      return len==0;
    };

    inline bool save(FILE *file, const vector<ValT> &x) const
    {
      //-- get saved size
      Item<size_t> size_item;
      if (!size_item.save(file, n)) return false;

      //-- save items
      for (vector<ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	val_item.save(file,*xi);
      }
      return true;
    };
  };


  /*------------------------------------------------------------
   * SomeType
   */
  /*
  template<> class Item<SomeType> {
  public:
    inline bool load(FILE *file, T &x) const {};
    inline bool save(FILE *file, const T &x) const {};
    inline bool load_n(FILE *file, T *&x, size_t &n) const {};
    inline bool save_n(FILE *file, const T *x, size_t n) const {};
  };
  */
  

}; //-- namespace binIO



/*--------------------------------------------------------------
 * MAIN
 */
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

  /*--------------------------------------------------
   * save: int
   */
  fprintf(stderr, "%s: testing save<int> [0..3] ...", progname);
  binIO::Item<int> bin_int;
  int i;
  for (i = 0; i < 3; i++) {
    if (!bin_int.save(file, i)) {
      fprintf(stderr, "%s: save<int> failed for i=%d", progname, i);
      exit(3);
    }
  }
  fprintf(stderr, " ok.\n");

  /*--------------------------------------------------
   * save: char *
   */
  fprintf(stderr, "%s: testing save<char *> [foo,bar,baz] ...", progname);
  binIO::Item<char *> bin_cstr;
  const char * cstrs[] = {"foo","bar","baz"};
  for (i = 0; i < 3; i++) {
    if (!bin_cstr.save(file, cstrs[i])) {
      fprintf(stderr, "%s: save<char *> failed for s=%s", progname, cstrs[i]);
      exit(3);
    }
  }
  fprintf(stderr, " ok.\n");


  /*--------------------------------------------------
   * save: string
   */
  fprintf(stderr, "%s: testing save<string> [foo,bar,baz] ...", progname);
  binIO::Item<string> bin_string;
  const string strs[3] = {string("foo"), string("bar"), string("baz")};
  for (i = 0; i < 3; i++) {
    if (!bin_string.save(file, strs[i])) {
      fprintf(stderr, "%s: save<string> failed for s=%s", progname, strs[i].c_str());
      exit(3);
    }
  }
  fprintf(stderr, " ok.\n");


  //-- cleanup
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

  /*--------------------------------------------------
   * load: int
   */
  fprintf(stderr, "%s: testing load<int> [0..3] ... ", progname);
  int j;
  for (i = 0; i < 3; i++) {
    if (!bin_int.load(file, j)) {
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
    if (!bin_cstr.load(file, buf)) {
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
    if (!bin_string.load(file, s)) {
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
  fclose(file);


  return 0;
}
