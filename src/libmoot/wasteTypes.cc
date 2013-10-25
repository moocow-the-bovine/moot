/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
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

#include <wasteTypes.h>

namespace moot {

//----------------------------------------------------------------------
const char *wasteScannerTypeNames[NwScanTypes] =
  {
    "EOF",
    "WB",
    "SB",
    "Latin",
    "Greek",
    "Alpha",
    "Newline",
    "Space",
    "Number",
    "Roman",
    "Hyphen",
    "Punct",
    "Link",
    "XML",
    "Comment",
    "Other"
  };

//----------------------------------------------------------------------
const char *wasteLexerTypeNames[NwLexerTypes] =
  {
    "Dot",
    "Comma",
    "Hyph",
    "Apostrophe",
    "Quote",
    "Monetary",
    "Percent",
    "Plus",
    "LBR",
    "RBR",
    "Slash",
    "Colon",
    "Semicolon",
    "EOS",
    "Punct",
    "Space",
    "Newline",
    "Number",
    "Roman",
    "AlphaLower",
    "AlphaUpper",
    "AlphaCaps",
    "AlphaTrunc",
    "Other",
  };

}; //--/namespace moot
