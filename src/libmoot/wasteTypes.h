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

/*--------------------------------------------------------------------------
 * File: wasteTypes.h
 * Author: Bryan Jurish <moocow@cpan.org>, Kay-Michael Würzner
 * Description:
 *   + moot PoS tagger : WASTE HMM tokenizer: common definitions
 *--------------------------------------------------------------------------*/

/**
\file wasteTypes.h
\brief Common definitions for WASTE HMM-based tokenizer
*/
#ifndef _WASTE_TYPES_H
#define _WASTE_TYPES_H

namespace moot {

/** return value enum for wasteScanner::yylex() */
enum wasteScannerTypeE {
  wScanTypeEOF,		/**< end-of-stream */
  wScanTypeWB,		/**< $WB$: word-break hint */
  wScanTypeSB,		/**< $SB$: sentence-break hint */
  wScanTypeLatin,	/**< latin string */
  wScanTypeGreek,	/**< greek string */
  wScanTypeAlpha,	/**< alphabetic string, any script */
  wScanTypeNewline,	/**< newline */
  wScanTypeSpace,	/**< whitespace without embedded newline */
  wScanTypeNumber,	/**< number string */
  wScanTypeRoman,	/**< roman numeral string (subset of latin) */
  wScanTypeHyphen,	/**< hyphen, en-, or em-dash */
  wScanTypePunct,	/**< punctuation and "special" characters */
  wScanTypeLink,	/**< URI or other link */
  wScanTypeXML,		/**< raw XML */
  wScanTypeComment,	/**< raw comment */
  wScanTypeOther,	/**< any other character */
  NwScanTypes		/**< eof */
};

/** Symbolic names for wasteScannerTypeE, useful for debugging */
extern const char *wasteScannerTypeNames[NwScanTypes];


/** old dwdsScanner type constants */
enum wasteLexerTypeE {
  wst_TOKEN_EOF,
  wst_TOKEN_UNKNOWN,
  wst_TOKEN_WORD,
  wst_TOKEN_HYPH,

  wst_TOKEN_SPACE,
  wst_TOKEN_NL,

  wst_TOKEN_GREEK,

  wst_TOKEN_NUM,
  wst_TOKEN_DOT,
  wst_TOKEN_EOS,
  wst_TOKEN_COMMA,
  wst_TOKEN_QUOTE,
  wst_TOKEN_SC,
  wst_TOKEN_ROMAN_NUM,

  wst_STOP_LOWER,
  wst_STOP_UPPER,
  wst_STOP_CAPS,

  wst_LATIN_LOWER,
  wst_LATIN_UPPER,
  wst_LATIN_CAPS,

  wst_TOKEN_MONEY,
  wst_TOKEN_SB,

  wst_LATIN_LOWER_TRUNC,
  wst_LATIN_LOWER_TRUNC1,
  wst_LATIN_LOWER_TRUNC2,
  wst_TOKEN_TRUNC,

  wst_TOKEN_PERCENT,

  wst_TOKEN_XML,

  wst_ABBREV,
  wst_TOKEN_APOS,
  wst_TOKEN_PLUS,
  wst_TOKEN_LBR,
  wst_TOKEN_RBR,
  wst_TOKEN_SLASH,
  wst_TOKEN_COLON,
  wst_TOKEN_SEMICOLON,

  wst_TOKEN_WB,
  wst_TOKEN_REST,

  wst_N_TYPES
};

/** Useful for debugging old dwdsScanner types */
extern const char *wasteLexerTypeNames[wst_N_TYPES];


}; /*--/namespace moot --*/

#endif /* _WASTE_TYPES_H */

