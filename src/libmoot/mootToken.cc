/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootToken.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token information
 *--------------------------------------------------------------------------*/

#include "mootToken.h"

namespace moot {
  using namespace std;

  const char *mootTokenTypeNames[NTokTypes] =
    {
      "TokTypeUnknown",
      "TokTypeVanilla",
      "TokTypeLibXML",
      "TokTypeXMLRaw",
      "TokTypeComment",
      "TokTypeEOS",
      "TokTypeEOF",
      "TokTypeUser"
    };

  const char *mootTokenFlavorNames[NTokFlavors] =
    {
      "@ALPHA",
      "@CARD",
      "@CARDPUNCT",
      "@CARDSUFFIX",
      "@CARDSEPS",
      "@UNKNOWN"
    };


}; // moot_END_NAMESPACE
