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
#include <set>
#include <mootTypes.h>
#include <mootIO.h>
#include <mootCIO.h>

namespace moot {
  using namespace std;

//==============================================================================
// mootTaster
/**
 * \brief High-level heuristic token classifier
 * \detail
 * \note regular expressions may be sensitive to the current locale settings,
 *       in particular LC_CTYPE.  For best results, ensure that your locale is set
 *       sensibly whenever you use a user-defined taster, e.g. by calling setlocale(LC_ALL,"").
 */
class mootTaster {
public:
  //--------------------------------------------------------------------
  // Embedded Types

  //------------------------------------------------------------
  // mootTaster::Rule
  /** \brief type for a single regex-based token classification heuristic
   * \detail
   * \note regexes may be locale-sensitive; ensure that your locale is set sensibly
   *       if using locale-sensitive regexes; e.g. by calling setlocale(LC_ALL,"")
   */
  class Rule {
  public:
    //--------------------------------------------------
    // mootTaster::Rule: data members
    string   lab;   ///< symbolic label
    string   re_s;  ///< POSIX.2 regex to match ("extended" regex string; see regex(7) manpage)
    regex_t *re_t;  ///< regex to match (compiled)

  public:
    //--------------------------------------------------
    // mootTaster::Rule: constructors etc.

    /** String-based constructor */
    Rule(const std::string &label="", const std::string& regex="")
      : lab(label), re_s(regex), re_t(NULL)
    { compile(); };

    /** Copy constructor */
    Rule(const Rule &r)
      : lab(r.lab), re_s(r.re_s), re_t(NULL)
    { compile(); };

    /** destructor */
    ~Rule(void)
    { clear(); };

    /** de-compile the regex, if any */
    void clear();

    /** (re-)compile rule */
    void compile();

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
  inline void clear()
  { rules.clear(); };

public:
  //--------------------------------------------------------------------
  // methods: rule-set

  /** get current number of rules */
  inline size_t size() const
  { return rules.size(); };

  /** append a single rule */
  inline void append_rule(const Rule &r)
  { rules.push_back(r); };

  /** append a single rule specification */
  inline void append_rule(const std::string &label, const std::string &regex)
  { rules.push_back(Rule(label,regex)); };

  /** set the default label \a nolabel in global object and all rules with target label \a nolabel  */
  void set_default_label(const std::string &label, bool update_rules=true);

public:
  //--------------------------------------------------------------------
  // methods: info

  /** check whether this taster defines at least one rule for label \a l */
  bool has_label(const std::string &l) const;

  /** set of all label names defined by this taster */
  set<string> labels(void) const;

public:
  //--------------------------------------------------------------------
  // methods: matching

  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  const string& flavor(const char *s) const;

  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  inline const string& flavor(const string &s) const
  { return flavor(s.c_str()); };

public:
  //--------------------------------------------------------------------
  // methods: I/O

  /** load (append) rules from a moot input stream (mistream).
   *  File format is a list of rules in order of decreasing precedence,
   *  one rule per line.  Each rule-line is a TAB-separated list of the form:
   *    LABEL "\t" REGEX
   *  where LABEL is the label of a rule and REGEX is a POSIX.2 extended regular
   *  expression, or a line:
   *    "DEFAULT" "\t" LABEL
   *  which cases the default label to be set to LABEL.
   */
  void load(mootio::mistream* mis);

  /** load (append) rules from a named file */
  void load(const char *filename);

  /** set default TnT-style rules */
  void set_default_rules(void);

    
}; //-- /mootTaster



}; /* namespace moot */

#endif /* _moot_FLAVOR_H */
