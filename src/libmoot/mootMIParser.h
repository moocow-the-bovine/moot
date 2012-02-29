/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2009 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootMIParser.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger: MI "parser"
 *--------------------------------------------------------------------------*/

/**
\file mootMIParser.h
\brief pointwise-MI parser using mootHMM
*/

#ifndef _MOOT_MIPARSER_H
#define _MOOT_MIPARSER_H

#include <mootDynHMM.h>
#include <assert.h>

moot_BEGIN_NAMESPACE


/*======================================================================
 *Generic  Utilities
 */


/*======================================================================
 * mootMIParser : MI "parser" class
 */

/**
 * \brief HMM subclass for MI parsing
 * \details
 * Adds a new analysis ?
 */
class mootMIParser: public mootDynHMM {
public:
  /*---------------------------------------------------------------------*/
  /* Types */
  typedef mootTagString             TagStr;
  typedef size_t                    SentIndex;
  typedef std::pair< ProbT, SentIndex > ProbIndexPair;
  typedef std::vector< ProbIndexPair >  ProbIndexPairVector;
  typedef std::vector< mootToken* >     TokenPtrVector;

public:
  /*---------------------------------------------------------------------*/
  /* Data */
  mootTagString newtag_str;
  TagID         newtag_id;
  ProbT         newtag_f;
  bool          text_tags;     /**< use token text as 'tag' for n-gram computations? (default=false: use tag) */
  mootTagString output_tag;    /**< output analysis tag to use (default="$MIParser" */
  mootTagString output_prefix; /**< analysis prefix to use (default="[$MIParser] " */

public:
  /*---------------------------------------------------------------------*/
  /** \name Constructors, etc. */
  //@{
  /** Default constructor sets hash_ngrams=true */
  mootMIParser(void)
    : newtag_str("@NEW"),
      newtag_id(0),
      newtag_f(0.5),
      text_tags(false),
      output_tag("$MIParser"),
      output_prefix("[$MIParser] ")
  {};

  /** Destructor */
  virtual ~mootMIParser(void)
  {};

  /** Set generic user-level options. */
  virtual void set_options(const mootDynHMMOptions &opts)
  {
    newtag_str = opts.newtag_str;
    text_tags  = opts.text_tags;
  };
  //@}

  //------------------------------------------------------------
  // Utilities
  //@{
  //@}

  //---------------------------------------------------------------------
  ///\name Compilation & initialization
  //@{
  /** load a binary or text mode */
  virtual bool load_model(const string &modelname,
			  const mootTagString &start_tag_str="__$",
			  const char *myname="mootDynLexHMM::load_model()",
			  bool  do_estimate_nglambdas=true,
			  bool  do_estimate_wlambdas=true,
			  bool  do_estimate_clambdas=true,
			  bool  do_build_suffix_trie=true,
			  bool  do_compute_logprobs=true);

  /** compile a text model */
  virtual bool compile(mootLexfreqs &lexfreqs,
		       mootNgrams &ngrams,
		       mootClassfreqs &classfreqs,
		       const mootTagString &start_tag_str="__$");
  //@}

  //------------------------------------------------------------
  /** \name Tagging: Hooks (API) */
  //@{
  //@}

  /*---------------------------------------------------------------------*/
  ///\name User-level Niceties
  //@{
  /** Write some debugging header information to an output stream
   *  Default implementation does nothing. */
  virtual void tw_put_info(moot::TokenWriter *tw)
  {
    //mootDynHMM::tw_put_info(tw);
    tw->printf_raw("  +DynHMM class      : %s\n", "mootMIParser");
    tw->printf_raw("   New Tag           : %s\n", newtag_str.c_str());
    tw->printf_raw("   N-gram source     : %s\n", text_tags ? "text" : "tags");
    tw->printf_raw("   Output Tag        : %s\n", output_tag.c_str());
    tw->printf_raw("   Output Prefix     : \"%s\"\n", output_prefix.c_str());
  };
  //@}

  //------------------------------------------------------------
  // Tagging: Top-level
  /** \name Top-level Tagging Interface (overrides) */
  //@{

  /**
   * Top-level tagging interface: mootSentence
   * Just wraps mootHMM::tag_sentence() in calls to tag_hook_pre() and tag_hook_post().
   */
  virtual void tag_sentence(mootSentence &sentence);
  //@}
};


moot_END_NAMESPACE

#endif /* _MOOT_DYNHMM_H */
