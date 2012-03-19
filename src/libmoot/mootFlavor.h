/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2012 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
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
 * Author: Bryan Jurish <moocow@cpan.org>
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
// types
typedef string mootFlavorStr;
typedef UInt   mootFlavorID;

//==============================================================================
// mootTaster

class mootTaster;  //-- forward decl

/** Default built-in taster */
extern const mootTaster builtinTaster;

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
  // Embedded Types & Constants

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
    mootFlavorStr lab;   ///< symbolic label
    mootFlavorID  id;    ///< numeric id (zero by default)
    string   	  re_s;  ///< POSIX.2 regex to match ("extended" regex string; see regex(7) manpage)
    regex_t	  *re_t;  ///< regex to match (compiled)

  public:
    //--------------------------------------------------
    // mootTaster::Rule: constructors etc.

    /** String-based constructor */
    Rule(const mootFlavorStr &label="", const std::string &regex="")
      : lab(label), id(0), re_s(regex), re_t(NULL)
    { compile(); };

    /** Copy constructor */
    Rule(const Rule &r)
      : lab(r.lab), id(r.id), re_s(r.re_s), re_t(NULL)
    { compile(); };

    /** destructor */
    ~Rule(void)
    { clear(); };

    /** de-compile the regex, if any */
    void clear();

    /** (re-)compile rule */
    void compile();

    /** equality predicate compares lab,re_s */
    inline bool operator==(const Rule &r2) const
    { return lab==r2.lab && /*id==r2.id &&*/ re_s==r2.re_s; };

    /** assignment operator */
    inline Rule& operator=(const Rule &r2)
    { lab=r2.lab; id=r2.id; re_s=r2.re_s; compile(); return *this; };

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

  Rules rules;			///< matching heuristics in order of decreasing priority
  mootFlavorStr nolabel;	///< label to return if no rule matches (default: empty)
  mootFlavorID  noid;		///< id to return if no rule matches (default: empty)
  set<mootFlavorStr> labels;	///< set of all flavor labels

public:
  //--------------------------------------------------------------------
  // Constructors etc.

  /** Default constructor */
  mootTaster(const mootFlavorStr &default_label="", mootFlavorID default_id=0)
    : nolabel(default_label), noid(default_id)
  { set_default_rules(); };

  /** Destructor */
  ~mootTaster()
  {};

  /** clear stored rules */
  void clear();

public:
  //--------------------------------------------------------------------
  // methods: info

  /** get current number of rules */
  inline size_t size() const
  { return rules.size(); };

  /** get current number of rules */
  inline bool empty() const
  { return rules.empty(); };

  /** equality predicate tests rules, nolabel, noid */
  inline bool operator==(const mootTaster &t2) const
  { return rules==t2.rules && nolabel==t2.nolabel && noid==t2.noid; };

  /** returns true iff this taster is equivalent to the default set of built-in rules */
  inline bool is_builtin(void) const
  { return operator==(builtinTaster); };

  /** assignment operator */
  inline mootTaster& operator=(const mootTaster &t2)
  { rules=t2.rules; nolabel=t2.nolabel; noid=t2.noid; labels=t2.labels; return *this; };

public:
  //--------------------------------------------------------------------
  // methods: rule-set

  /** append a single rule */
  inline void append_rule(const Rule &r)
  {
    rules.push_back(r);
    labels.insert(r.lab);
  };

  /** append a single rule specification */
  inline void append_rule(const mootFlavorStr &label, const std::string &regex)
  { append_rule(Rule(label,regex)); };

  /** set the default label \a nolabel in global object and all rules with target label \a nolabel  */
  void set_default_label(const mootFlavorStr &label, bool update_rules=true);

public:
  //--------------------------------------------------------------------
  // methods: info

  /** check whether this taster defines at least one rule for label \a l */
  inline bool has_label(const mootFlavorStr &l) const
  { return labels.find(l) != labels.end(); };

public:
  //--------------------------------------------------------------------
  // methods: matching

  //--------------------------------------
  /** get index of first rule matching \a s, or \a rules.end() if no rule matches */
  Rules::const_iterator find(const char *s) const;

  /** get index of first rule matching \a s, or \a rules.end() if no rule matches; std::string version */
  inline Rules::const_iterator find(const std::string &s) const
  { return find(s.c_str()); };

  //--------------------------------------
  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  inline const mootFlavorStr& flavor(const char *s) const
  {
    Rules::const_iterator ri = find(s);
    return (ri == rules.end()) ? nolabel : ri->lab;
  };

  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  inline const mootFlavorStr& flavor(const string &s) const
  { return flavor(s.c_str()); };

  //--------------------------------------
  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  inline mootFlavorID flavor_id(const char *s) const
  {
    Rules::const_iterator ri = find(s);
    return (ri == rules.end()) ? noid : ri->id;
  };

  /** get label of first rule matching \a s, or \a this->nolabel if no rule matches */
  inline mootFlavorID flavor_id(const string &s) const
  { return flavor_id(s.c_str()); };

public:
  //--------------------------------------------------------------------
  // methods: I/O: load


  /** load (append) rules from a moot input stream (mistream).
   *  File format is a list of rules in order of decreasing precedence,
   *  one rule per line.  Each rule-line is a TAB-separated list of the form:
   *    LABEL "\t" REGEX
   *  where LABEL is the label of a rule and REGEX is a POSIX.2 extended regular
   *  expression, or a line:
   *    "DEFAULT" "\t" LABEL
   *  which cases the default label to be set to LABEL.
   *
   *  If specified the literal prefix \a prefix is removed from each line before parsing.
   */
  bool load(mootio::mistream *mis, const std::string &prefix="");

  /** load (append) rules from a named file */
  bool load(const char *filename, const std::string &prefix="");

  /** load (append) rules from a named file */
  bool load(const std::string &filename, const std::string &prefix="")
  { return load(filename.c_str(), prefix); };

  /** set default TnT-style rules (called by default constructor) */
  void set_default_rules(void);

  //--------------------------------------------------------------------
  // methods: I/O: save

  /** save rules to a moot output stream (mostream).
   *  If specified the literal prefix \a prefix is prepended to each output line.
   */
  bool save(mootio::mostream *mos, const std::string &prefix="") const;

  /** save rules to a named file (clobbers old file) */
  bool save(const char *filename, const std::string &prefix="") const;

  /** save rules to a named file, std::string version */
  bool save(const std::string &filename, const std::string &prefix="") const
  { return save(filename.c_str(), prefix); };

  /** save rules to an open C FILE* */
  bool save(FILE *f, const std::string &prefix="") const
  { mootio::mcstream mcs(f); return save(&mcs,prefix); };
    
}; //-- /mootTaster


}; /* namespace moot */

#endif /* _moot_FLAVOR_H */
