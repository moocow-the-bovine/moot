/* -*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*- */
/*
   libmoot : moot part-of-speech tagging library
   Copyright (C) 2013 by Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner

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

/**
 * \file wasteLexicon.h
 *
 * \brief simple hash_set<>-based lexicon class for moot::wasteLexer
 *
 * \author moocow
 * 
 * \date 2013
 * 
 */

#ifndef _WASTE_LEXICON_H
#define _WASTE_LEXICON_H

#include <mootSTLHacks.h>
#include <mootTokenIO.h>

moot_BEGIN_NAMESPACE

/*============================================================================
 * wasteLexicon
 */
/** \brief simple hash_set<>-based lexicon class  */
class wasteLexicon
{
 public:
  typedef moot_STL_NAMESPACE::hash_set<std::string>  Lexicon;  /**< typedef for underlying lexicon data */
  Lexicon lex;  /**< underlying lexicon data */

  //------------------------------------------------------------
  /// \name Constructors etc.
  //@{
  /** Default constructor */
  wasteLexicon()
  {};

  /** desctructor */
  ~wasteLexicon()
  {};    

  /** clear all entries */
  inline void clear()
  {
    lex.clear();
  };
  //@}

  //------------------------------------------------------------
  /// \name Access and Manipulation
  //@{
  /** \returns true iff \a word is present in the lexicon */
  inline bool lookup(const std::string &word) const
  {
    return (lex.find(word) != lex.end());
  };

  /** insert \a word into the lexicon */
  inline void insert(const std::string &word)
  {
    lex.insert(word);
  };
  //@}

  //------------------------------------------------------------
  /// \name I/O
  //@{
  /** load lexicon from a moot::TokenReader: adds only mootToken::text() of moot::TokTypeVanilla tokens
   *  \returns true on success, false otherwise
   */
  bool load(moot::TokenReader *reader);

  /** load lexicon from a \a mootio::mistream; wraps load(moot::TokenReader*) using a temporary TokenReaderNative
   *  \returns true on success, false otherwise
   */
  bool load(mootio::mistream *mis);

  /** load lexicon from a named file; wraps load(mootoi::mistream*) method 
   *  \returns true on success, false otherwise
   */
  bool load(const char *filename);
  //@}
};

moot_END_NAMESPACE

#endif /* _WASTE_LEXICON_H */

