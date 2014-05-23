/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2012-2014 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootModelSpec.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : model specification
 *--------------------------------------------------------------------------*/

/**
\file mootModelSpec.h
\brief utilities for model specification
*/

#ifndef _moot_MODELSPEC_H
#define _moot_MODELSPEC_H

#include <string>

namespace moot {

  //============================================================================
  /** Utility class for mootHMM::load_model() and friends
   *  \detail See mootfiles(5) for details
   */
  class mootModelSpec {
  public:
    std::string binfile;	///< (bin): filename of binary model; overrides text model options
    std::string lexfile;	///< (txt): filename of lexical frequency file
    std::string ngfile;		///< (txt): filename of n-gram frequency file
    std::string lcfile;		///< (txt): filename of lexical-class frequency file
    std::string flafile;	///< (txt): filename of flavor heuristic file

  public:
    //----------------------------------------------------------
    // Constructors etc.

    /** Default constructor given an optional model name calls parse() */
    mootModelSpec(const std::string &modelname="", bool try_bin=true)
      : binfile(""), lexfile(""), ngfile(""), lcfile(""), flafile("")
    { parse(modelname,try_bin); };

    /** Clear the specification */
    void clear(void);

    /** Returns true iff model-spec is empty */
    bool empty(void) const;

    /** Returns true iff this is a binary model */
    inline bool isbinary(void) const
    { return !binfile.empty(); };

    /** Parse a single modelname; if \a try_bin is true, existence
     *  of a binary file \a modelname clobbers any text model in \a modelname.*
     *  \returns true iff at least one component model file exists
     */
    bool parse(const std::string &modelname, bool try_bin=true);
  };

  //============================================================================


}; /* namespace moot */

#endif /* _moot_MODELSPEC_H */
