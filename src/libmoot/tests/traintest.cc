#include <stdio.h>

#include <deque>
#include <set>
#include <string>

using namespace std;

typedef string        Tag;
typedef set<Tag>      TagSet;
typedef deque<Tag>    Ngram;


/*-------------------------------------------------------------
 * output
 */
string stringify(const TagSet &ts) {
  string s("{");
  for (TagSet::const_iterator tsi = ts.begin(); tsi != ts.end(); tsi++) {
    if (tsi != ts.begin()) s.push_back(',');
    s.append(*tsi);
  }
  s.push_back('}');
  return s;
}


string stringify(const Ngram &ng) {
  string s("<");
  for (Ngram::const_iterator ngi = ng.begin(); ngi != ng.end(); ngi++) {
    if (ngi != ng.begin()) s.append(", ");
    s.append(*ngi);
  }
  s.push_back('>');
  return s;
}


/*-------------------------------------------------------------
 * Guts
 */
class NgramSet {
public:
  typedef deque<TagSet> ngsType;
public:
  ngsType ngs;
public:
  //-- construct / destroy
  NgramSet(size_t size=0) { ngs.resize(size); };
  ~NgramSet(void) {};

  //-- clear , size, resize
  void clear(void) { ngs.clear(); };
  const size_t size(void) { return ngs.size(); };
  void resize(size_t size) { ngs.resize(size); };

  //-- push, pop, step
  void push_front(const TagSet &ts) { ngs.push_front(ts); };
  void push_back(const TagSet &ts) { ngs.push_back(ts); };
  void pop_front(void) { ngs.pop_front(); };
  void pop_back(void) { ngs.pop_back(); };
  void step(const TagSet &ts)
  {
    ngs.pop_front();
    ngs.push_back(ts);
  };

  //-- iteration
  struct ngIteratorItem {
    size_t pos;
    TagSet::iterator cur;
    TagSet::iterator begin;
    TagSet::iterator end;
  };

  typedef deque<ngIteratorItem>         ngIterator;
  typedef ngIterator::iterator          ngIterator2;
  typedef ngIterator::reverse_iterator  ngIterator2r;

  ngIterator iter_begin(size_t len=0)
  {
    ngIterator  it(len <= ngs.size() ? len : ngs.size());
    ngIterator2 iti;
    ngsType::iterator ngsi;
    size_t pos;
    for (iti    = it.begin() ,  ngsi  = ngs.begin() , pos = 0;
	 iti   != it.end()  &&  ngsi != ngs.end();
	 iti++               ,  ngsi++              , pos++)
      {
	iti->pos   = pos;
	iti->begin = iti->cur = ngsi->begin();
	iti->end   = ngsi->end();
      }
    return it;
  };

  bool iter_valid(ngIterator &it)
  {
    //--DEBUG
    /*
    bool rc = !it.empty() && it.front().cur != it.front().end;
    Ngram ng;
    iter2ngram(it,ng);
    printf("(DEBUG): iter_valid(%s) => %d\n", stringify(ng).c_str(), rc ? 1 : 0);
    */
    //--/DEBUG    
    return !it.empty() && it.front().cur != it.front().end;
  };

  ngIterator &iter_next(ngIterator &it)
  {
    /*
    Ngram ng;
    iter2ngram(it,ng);
    printf("(DEBUG/incr): iter_next(%s) : called\n", stringify(ng).c_str());
    */

    if (!iter_valid(it)) return it;

    //printf("(DEBUG/incr): iter_next(%s) : running\n", stringify(ng).c_str());

    ngIterator2r                 itr;
    for (itr = it.rbegin(); itr != it.rend(); itr++) {
      Tag lasttag = *(itr->cur);

      //-- easy increment
      itr->cur++;

      //-- check overflow
      if (itr->cur == itr->end) {
	  if (itr->pos != 0) {
	    //printf("(DEBUG/incr): end()+reset after '%s'\n", lasttag.c_str());
	    itr->cur = itr->begin;
	  } /*
	      else {
	    printf("(DEBUG/incr): end()+NO-reset after '%s'\n", lasttag.c_str());
	  }
	    */
	  continue;
	}

	//-- done
	break;
      }

    /*
    iter2ngram(it,ng);
    printf("(DEBUG/incr): iter_next(%s) : returning\n", stringify(ng).c_str());
    */
    return it;
  };

  Ngram &iter2ngram(ngIterator &it, Ngram &ng)
  {
    ng.resize(it.size());
    Ngram::iterator ngi;
    ngIterator2 iti;
    for (iti    = it.begin() ,  ngi  = ng.begin();
	 iti   != it.end()  &&  ngi != ng.end();
	 iti++               ,  ngi++)
      {
	*ngi = *(iti->cur);
      }
    return ng;
  };
};


/*-------------------------------------------------------------
 * More stringification
 */
string stringify(const NgramSet &ngs) {
  string s("<");
  for (NgramSet::ngsType::const_iterator ngsi = ngs.ngs.begin(); ngsi != ngs.ngs.end(); ngsi++) {
    if (ngsi != ngs.ngs.begin()) s.append(", ");
    s.append(stringify(*ngsi));
  }
  s.push_back('>');
  return s;
}



/*-------------------------------------------------------------
 * MAIN
 */
char *progname = NULL;
int main (int argc, char **argv) {
  progname = *argv;

  TagSet ts1;
  TagSet ts2;
  TagSet ts3;
  NgramSet ngs;

  ts1.insert("tag1.1");
  ts2.insert("tag2.1");
  ts3.insert("tag3.1");
  ts3.insert("tag3.2");

  ngs.push_back(ts1);
  ngs.push_back(ts2);
  ngs.push_back(ts3);

  //-- basic check
  printf("%s: ts1=%s\n", progname, stringify(ts1).c_str());
  printf("%s: ts2=%s\n", progname, stringify(ts2).c_str());
  printf("%s: ts3=%s\n", progname, stringify(ts3).c_str());
  printf("%s: ngs=%s\n", progname, stringify(ngs).c_str());

  //-- try to iterate
  size_t len;
  NgramSet::ngIterator ngi;
  Ngram ng;
  for (len = 1; len <= ngs.size(); len++) {
    printf("%s: iteration [len=%u]...\n", progname, len);
    for (ngi = ngs.iter_begin(len); ngs.iter_valid(ngi); ngs.iter_next(ngi)) {
      ngs.iter2ngram(ngi,ng);
      printf("\t%s\n", stringify(ng).c_str());
    }
  }

  return 0;
}
