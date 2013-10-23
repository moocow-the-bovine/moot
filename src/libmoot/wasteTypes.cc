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

/** Useful for debugging waste-scanner types */
const char *wasteScannerTypeNames[wst_N_TYPES] =
  {
    "TOKEN_EOF",
    "TOKEN_UNKNOWN",
    "TOKEN_WORD",
    "TOKEN_HYPH",

    "TOKEN_SPACE",
    "TOKEN_NL",

    "TOKEN_GREEK",

    "TOKEN_NUM",
    "TOKEN_DOT",
    "TOKEN_EOS",
    "TOKEN_COMMA",
    "TOKEN_QUOTE",
    "TOKEN_SC",
    "TOKEN_ROMAN_NUM",

    "STOP_LOWER",
    "STOP_UPPER",
    "STOP_CAPS",

    "LATIN_LOWER",
    "LATIN_UPPER",
    "LATIN_CAPS",

    "TOKEN_MONEY",
    "TOKEN_SB",

    "LATIN_LOWER_TRUNC",
    "LATIN_LOWER_TRUNC1",
    "LATIN_LOWER_TRUNC2",
    "TOKEN_TRUNC",

    "TOKEN_PERCENT",

    "TOKEN_XML",

    "ABBREV",
    "TOKEN_APOS",
    "TOKEN_PLUS",
    "TOKEN_LBR",
    "TOKEN_RBR",
    "TOKEN_SLASH",
    "TOKEN_COLON",
    "TOKEN_SEMICOLON",

    "TOKEN_WB",
    "TOKEN_REST"
  };

}; //--/namespace moot
