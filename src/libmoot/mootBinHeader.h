/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2011 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootBinHeader.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : abstract binary headers
 *--------------------------------------------------------------------------*/

/** \file mootBinHeader.h
 *  \brief abstract binary header class
 */

#ifndef _MOOT_BINHEADER_H
#define _MOOT_BINHEADER_H

namespace mootBinIO {

  /*------------------------------------------------------------
   * public typedefs: Generic header information
   */
  /** \brief Header information structure, used for binary HMM model files */
  class HeaderInfo {
  public:
    /** Typedef for a version component */
    typedef BinUInt VersionT;
    
    /** Typedef for a "magic number" component */
    typedef BinUInt MagicT;
    
    /** Typedef for a generic "flags" component */
    typedef BinULong FlagsT;

  public:
    MagicT    magic;      /**< Magic number */
    VersionT  version;    /**< Major version */
    VersionT  revision;   /**< Minor version */
    VersionT  minver;     /**< Minimum compatible version */
    VersionT  minrev;     /**< Minimum compatible revision */
    FlagsT    flags;      /**< Some user flags (unused) */

  public:
    /** Default constructor */
    HeaderInfo(MagicT mag=0,
	       VersionT ver=0, VersionT rev=0,
	       VersionT mver=0, VersionT mrev=0,
	       FlagsT f=0)
      : magic(mag),
	version(ver),
	revision(rev),
	minver(mver),
	minrev(mrev),
	flags(f)
    {};

    /** Useful constructor: generate "magic" from IDstring by hashing */
    HeaderInfo(const string &IDstring,
	       VersionT ver=0, VersionT rev=0,
	       VersionT mver=0, VersionT mrev=0,
	       FlagsT f=0)
      : version(ver),
	revision(rev),
	minver(mver),
	minrev(mrev),
	flags(f)
    {
      magic = 0;
      for (string::const_iterator si = IDstring.begin(); si != IDstring.end(); si++) {
	magic = (magic<<5)-magic + static_cast<MagicT>(*si);
      }
    };
  };

}; //-- mootBinIO


#endif /* _MOOT_BINHEADER_H */
