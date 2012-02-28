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
 * File: mootFlavor.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token information : "flavors"
 *--------------------------------------------------------------------------*/

/**
\file mootFlavor.h
\brief classes and utilities for regex-based token "flavor" heuristics
*/

#ifndef _moot_FLAVOR_H
#define _moot_FLAVOR_H

#include <regex.h>
#include <list>
#include <vector>
#include <mootTypes.h>
#include <mootIO.h>
#include <mootCIO.h>

namespace moot {
  using namespace std;

//==============================================================================
// mootTaster
/**
 * \brief High-level heuristic token classifier
 */
class mootTaster {
public:
  //--------------------------------------------------------------------
  // Embedded Types

  //------------------------------------------------------------
  // mootTaster::Rule
  /** \brief type for a single regex-based token classification heuristic */
  class Rule {
  public:
    //--------------------------------------------------
    // mootTaster::Rule: data members
    regex_t *re_t;  ///< regex to match (compiled)
    string   re_s;  ///< regex to match (string)
    string   lab;   ///< symbolic label

  public:
    //--------------------------------------------------
    // mootTaster::Rule: constructors etc.

    /** String-based constructor */
    Rule(const std::string& regex="", const std::string &label="")
      : re_t(NULL), re_s(regex), lab(label)
    { compile(); };

    /** Copy constructor */
    Rule(const Rule &r)
      : re_t(NULL), re_s(r.re_s), lab(r.lab)
    { compile(); };

    /** destructor */
    ~Rule(void)
    { clear(); };

    /** de-compile the regex, if any */
    inline void clear()
    {
      if (re_t) {
	regfree(re_t);
	delete re_t;
      }
      re_t = NULL;
    };

    /** (re-)compile rule */
    inline void compile()
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
    };

  public:
    //--------------------------------------------------
    // mootTaster::Rule: methods

    /** returns true iff string \a s matches the compiled regex */
    inline bool match(const char *s) const
    {
      if (!re_t) return false;
      return regexec(re_t, s, 0, NULL, 0)==0;
    };

    /** returns true iff string \a s matches the compiled regex */
    inline bool match(const std::string &s) const
    { return match(s.c_str()); };
  };
  //--/mootTaster::Rule

public:
  //--------------------------------------------------------------------
  // Data Members
  typedef vector<Rule> Rules;

  Rules rules;		///< matching heuristics in order of decreasing priority
  string nolabel;	///< label to return if no rule matches (default: empty)

public:
  //--------------------------------------------------------------------
  // Constructors etc.

  /** Default constructor */
  mootTaster(const string &default_label="")
    : nolabel(default_label)
  {};

  /** Destructor */
  ~mootTaster()
  {};

  /** clear stored rules */
  void clear()
  { rules.clear(); };

public:
  //--------------------------------------------------------------------
  // Methods: rule-set

  /** get current number of rules */
  inline size_t size() const
  { return rules.size(); };

  /** append a single rule */
  inline void append_rule(const Rule &r)
  { rules.push_back(r); };

  /** append a single rule specification */
  inline void append_rule(const std::string &regex, const std::string &label)
  { rules.push_back(Rule(regex,label)); };

public:
  //--------------------------------------------------------------------
  // methods: information

  /** check whether this taster defines at least one rule for label \a l */
  bool has_label(const std::string &l) const
  {
    for (Rules::const_iterator ri=rules.begin(); ri!=rules.end(); ++ri) {
      if (ri->lab==l) return true;
    }
    return false;
  };

  /** list of all label names defined by this taster */
  list<string> labels(void) const
  {
    list<string> labs;
    for (Rules::const_iterator ri=rules.begin(); ri!=rules.end(); ++ri) {
      labs.push_back(ri->lab);
    }
    return labs;
  };

public:
  //--------------------------------------------------------------------
  // methods: matching

  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  const string& match(const char *s) const
  {
    for (Rules::const_iterator ri=rules.begin(); ri!=rules.end(); ++ri) {
      if (ri->match(s)) return ri->lab;
    }
    return nolabel;
  };

  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  const string& match(const string &s) const
  { return match(s.c_str()); };

public:
  //--------------------------------------------------------------------
  // methods: I/O

  /** load (append) rules from a moot input stream (mistream) */
  void load(mootio::mistream* mis)
  {
    string line, lab, re;
    size_t e_lab, b_re, e_re;
    while (mis->valid() && !mis->eof()) {
      mis->getline(line);
      e_lab = line.find('\t');
      b_re  = e_lab==line.npos ? line.npos : line.find_first_not_of("\t\r\n",e_lab);
      e_re  = b_re==line.npos  ? line.npos : line.find_first_of("\t\r\n",b_re);

      if (e_lab==line.npos || b_re>=e_re)
	continue; //-- skip lines without a tab or with no regex

      lab.clear();
      re.clear();
      if (e_lab != line.npos) lab.assign(line, 0, e_lab);
      if (b_re  != line.npos) re.assign(line, b_re, e_re-b_re);

      rules.push_back(Rule(re,lab));
    }
  };

  /** load (append) rules from a named file */
  void load(const char *filename)
  {
    mifstream mis(filename);
    load(&mis);
  };

  /** set default TnT-style rules */
  void set_default(void)
  {
    clear();
    append_rule("^([0-9]+)$","@CARD");
    append_rule("^([0-9]+)([\\.\\,\\-])$","@CARDPUNCT");
    append_rule("^([0-9])([\\.\\,\\-0-9]+)$","@CARDSEPS");
    append_rule("^([0-9])([\\.\\,\\-0-9]*)([^\\.\\,\\-0-9])(.{0,3})$", "@CARDSUFFIX");
  };

    
}; //-- /mootTaster



}; /* namespace moot */

#endif /* _moot_FLAVOR_H */
