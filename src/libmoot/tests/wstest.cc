#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dotest.h"

#include <string>

using namespace std;

bool dobench = false;

size_t bench_iters = 1048576/2;

string normalize_stl_2(const std::string &s, bool strip_left=true, bool strip_right=true)
{
  string st;
  st.reserve(s.size());
  string::const_iterator si;
  string::const_iterator sj;
  for (si = s.begin(); si != s.end(); si = sj) {
    if (isspace(*si)) {
      for (sj = si+1; sj != s.end() && isspace(*sj); sj++) ;
      //printf("--> ws: si='%c' ; sj='%c'\n", *si, *sj);
      if ( (si != s.begin() || !strip_left) && (sj != s.end() || !strip_right) )
	st.push_back(' ');
    } else {
      for (sj = si, sj++; sj != s.end() && !isspace(*sj); sj++) ;
      //printf("--> non-ws: si='%c' ; sj='%c'\n", *si, *sj);
      st.append(si, sj);
    }
  }
  return st;
};

string normalize_stl_2_bad(const std::string &s, bool strip_left=true, bool strip_right=true)
{
  string st;
  string::const_iterator si;
  string::const_iterator sj;
  for (si = s.begin(); si != s.end(); si = sj) {
    if (isspace(*si)) {
      for (sj = si+1; sj != s.end() && isspace(*sj); sj++) ;
      //printf("--> ws: si='%c' ; sj='%c'\n", *si, *sj);
      if ( (si != s.begin() || !strip_left) && (sj != s.end() || !strip_right) )
	st.push_back(' ');
    } else {
      for (sj = si, sj++; sj != s.end() && !isspace(*sj); sj++) ;
      //printf("--> non-ws: si='%c' ; sj='%c'\n", *si, *sj);
      st.append(si, sj);
    }
  }
  return st;
};

string normalize_stl(const char *s, bool strip_left=true, bool strip_right=true)
{
  size_t beg,end;
  string st = s;
  string str;
  for (beg  = st.find_first_not_of(" \n\r\t", 0) ,  end  = st.find_first_of(" \n\r\t", beg);
       beg  < st.size();
       beg  = end+1                              ,  end  = st.find_first_of(" \n\r\t", end+1))
    {
      str.append(st,beg,end);
      if (end==st.npos) break;
    }
  return str;
};



string normalize_cstr_2(const char *s, bool strip_left=true, bool strip_right=true)
{
  size_t i, j, len = strlen(s);
  string st;
  st.reserve(len);
  for (i = 0; i < len; i = j) {
    if (isspace(s[i])) {
      for (j = i+1; j < len && isspace(s[j]); j++) ;
      //printf("--> ws: s[i=%u]='%c' ; s[j=%u]='%c'\n", i, s[i], j, s[j]);
      if ( (i != 0 || !strip_left) && (j != len || !strip_right) )
	st.push_back(' ');
    } else {
      for (j = i+1; j < len && !isspace(s[j]); j++) ;
      //printf("--> non-ws: s[i=%u]='%c' ; s[j=%u]='%c'\n", i, s[i], j, s[j]);
      st.append(s+i, j-i);
    }
  }
  return st;
};


string normalize_cstr_2a(const char *s, bool strip_left=true, bool strip_right=true)
{
  size_t i, j, len = strlen(s);
  string st;
  for (i = 0; i < len; i = j) {
    switch (s[i]) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      for (j = i+1; j < len && isspace(s[j]); j++) ;
      //printf("--> ws: s[i=%u]='%c' ; s[j=%u]='%c'\n", i, s[i], j, s[j]);
      if ( (i != 0 || !strip_left) && (j != len || !strip_right) )
	st.push_back(' ');
      break;
    default:
      for (j = i+1; j < len && !isspace(s[j]); j++) ;
      //printf("--> non-ws: s[i=%u]='%c' ; s[j=%u]='%c'\n", i, s[i], j, s[j]);
      st.append(s+i, j-i);
      break;
    }
  }
  return st;
};

string normalize_cstr(const char *s, bool strip_left=true, bool strip_right=true)
{
  size_t i, len = strlen(s);
  string st;
  for (i = 0; i < len; ) {
    if (isspace(s[i])) {
      if (i != 0 || !strip_left) st.push_back(' ');
      for (i++; i < len && isspace(s[i]); i++) ;
      if (i==len && strip_right) st.erase(st.size()-1,1);
    }
    else {
      st.push_back(s[i]);
      i++;
    }
  }
  return st;
};


int main (int argc, char **argv) {
  //const char *progname = *argv;
  bool strip_left = true;
  bool strip_right = false;
  char lastc = ' ';

  for (argc--, argv++; argc > 0; argc--, argv++) {
    const char *s = *argv;
    string st_stl;
    string st_cstr;

    strip_left = isspace(lastc);
    strip_right = argc == 1;

    if (dobench) {
      fprintf(stderr, " -----\n");
      DOTEST_CODE(bench_iters, st_stl=normalize_stl(s));
      DOTEST_CODE(bench_iters, st_cstr=normalize_stl_2(s));

      DOTEST_CODE(bench_iters, st_cstr=normalize_cstr(s));
      DOTEST_CODE(bench_iters, st_cstr=normalize_cstr_2(s));
    }
    else {
      printf(" -----\n");
      st_stl = normalize_stl(s,strip_left,strip_right);
      printf("normalize_stl    (\"%s\",%d,%d) = \"%s\"\n",
	     s, strip_left, strip_right, st_stl.c_str());

      st_stl = normalize_stl_2(s,strip_left,strip_right);
      printf("normalize_stl_2  (\"%s\",%d,%d) = \"%s\"\n",
	     s, strip_left, strip_right, st_stl.c_str());

      st_cstr = normalize_cstr(s,strip_left,strip_right);
      printf("normalize_cstr   (\"%s\",%d,%d) = \"%s\"\n",
	     s, strip_left, strip_right, st_cstr.c_str());

      st_cstr = normalize_cstr_2(s,strip_left,strip_right);
      printf("normalize_cstr_2 (\"%s\",%d,%d) = \"%s\"\n",
	     s, strip_left, strip_right, st_cstr.c_str());
    }

    lastc=s[strlen(s)-1];
  }

  printf("isspace('\\0')=%d\n", isspace('\0'));
  return 0;
}
