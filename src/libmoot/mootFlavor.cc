/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2012 by Bryan Jurish <moocow.bovine@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootFlavor.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token information : "flavors"
 *--------------------------------------------------------------------------*/

#include "mootFlavor.h"

namespace moot {
  using namespace std;

//==============================================================================
// mootTaster::Rule

//----------------------------------------------------------------------
void mootTaster::Rule::clear(void)
{
  if (re_t) {
    regfree(re_t);
    delete re_t;
  }
  re_t = NULL;
}

//----------------------------------------------------------------------
void mootTaster::Rule::compile()
{
  if (re_t) { regfree(re_t); }
  else re_t = new regex_t();
  int rc = regcomp(re_t, re_s.c_str(), (REG_EXTENDED|REG_NOSUB));
  if (rc != 0) {
    char errbuf[512];
    regerror(rc, re_t, errbuf, 512);
    fprintf(stderr, "mootTaster::Rule::compile() failed for rule '%s', regex /%s/: %s\n", lab.c_str(), re_s.c_str(), errbuf);
    abort();
  }
}

//==============================================================================
// mootTaster: rule-set

//----------------------------------------------------------------------
void mootTaster::set_default_label(const std::string &label, bool update_rules)
{
  if (update_rules) {
    for (Rules::iterator ri=rules.begin(); ri!=rules.end(); ++ri) {
      if (ri->lab == nolabel) ri->lab = label;
    }
  }
  nolabel = label;
}


//==============================================================================
// mootTaster: info

//----------------------------------------------------------------------
bool mootTaster::has_label(const std::string &l) const
{
  for (Rules::const_iterator ri=rules.begin(); ri!=rules.end(); ++ri) {
    if (ri->lab==l) return true;
  }
  return false;
}

//----------------------------------------------------------------------
set<string> mootTaster::labels(void) const
{
  set<string> labs;
  for (Rules::const_iterator ri=rules.begin(); ri!=rules.end(); ++ri) {
    labs.insert(ri->lab);
  }
  return labs;
}

//==============================================================================
// mootTaster: matching

//----------------------------------------------------------------------
const string& mootTaster::flavor(const char *s) const
{
  for (Rules::const_iterator ri=rules.begin(); ri!=rules.end(); ++ri) {
    if (ri->match(s)) return ri->lab;
  }
  return nolabel;
};



//==============================================================================
// mootTaster: I/O

//----------------------------------------------------------------------
void mootTaster::load(mootio::mistream* mis)
{
  string line, lab, re;
  size_t e_lab, b_re, e_re;
  while (mis->valid() && !mis->eof()) {
    mis->getline(line);
    if (line.find("%%")==0) continue; //-- ignore comments
    e_lab = line.find('\t');
    b_re  = e_lab==line.npos ? line.npos : line.find_first_not_of("\t\r\n",e_lab);
    e_re  = b_re==line.npos  ? line.npos : line.find_first_of("\t\r\n",b_re);
    
    if (e_lab==line.npos || b_re>=e_re)
      continue; //-- skip lines without a tab or with no regex

    lab.clear();
    re.clear();
    if (e_lab != line.npos) lab.assign(line, 0, e_lab);
    if (b_re  != line.npos) re.assign(line, b_re, e_re-b_re);

    //-- check for "DEFAULT" line
    if (lab == string("DEFAULT")) {
      set_default_label(re);
      continue;
    }

    //-- normal rule
    rules.push_back(Rule(lab,re));
  }
};

//----------------------------------------------------------------------
void mootTaster::load(const char *filename)
{
  mootio::mifstream mis(filename);
  load(&mis);
};

//----------------------------------------------------------------------
void mootTaster::set_default_rules(void)
{
  clear();
  append_rule("",		"^[^0-9]");
  append_rule("@CARD",		"^([0-9]+)$");
  append_rule("@CARDPUNCT",	"^([0-9]+)([,\\.\\-])$");
  append_rule("@CARDSEPS",	"^([0-9])([0-9,\\.\\-]+)$");
  append_rule("@CARDSUFFIX",	"^([0-9])([0-9,\\.\\-]*)([^0-9,\\.\\-])(.{0,3})$");
};



//----------------------------------------------------------------------
}; /* namespace moot */
