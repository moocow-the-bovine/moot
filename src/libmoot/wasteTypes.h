/* -*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2013-2017 by Bryan Jurish <moocow@cpan.org> and Kay-Michael Würzner

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

#include <string>
#include <stdexcept>

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
  wScanTypePercent,	/**< escaped "%" sign (for literal "%%") */
  wScanTypeOther,	/**< any other character */
  NwScanTypes		/**< eof */
};
typedef wasteScannerTypeE wasteScannerType;

/** Symbolic names for wasteScannerTypeE, useful for debugging */
extern const char *wasteScannerTypeNames[NwScanTypes];


/** return value enum for wasteLexer::yylex() */
enum wasteLexerTypeE {
  wLexerTypeDot,        /**< "." */
  wLexerTypeComma,      /**< "," */
  wLexerTypeHyph,       /**< hyphen, en-, or em-dash */
  wLexerTypeApostrophe, /**< single quotes and apostrophe */
  wLexerTypeQuote,      /**< quotation characters (quotes and guillemets) */
  wLexerTypeMonetary,   /**< currency symbols */
  wLexerTypePercent,    /**< paragraph, percent and permille character */
  wLexerTypePlus,       /**< "+" */
  wLexerTypeLBR,        /**< left brackets and left brace */
  wLexerTypeRBR,        /**< right brackets and right brace */
  wLexerTypeSlash,      /**< slash and backslash */
  wLexerTypeColon,      /**< ":" */
  wLexerTypeSemicolon,  /**< ";" */
  wLexerTypeEOS,        /**< sentence terminating punctuation characters */
  wLexerTypePunct,      /**< punctuation and "special" characters */

  wLexerTypeSpace,      /**< whitespace without embedded newline */
  wLexerTypeNewline,    /**< newline */

  wLexerTypeNumber,     /**< number string */
  wLexerTypeRomanCaps,  /**< roman numeral string in caps*/
  wLexerTypeRomanLower, /**< roman numeral string in lower case */

  wLexerTypeAlphaLower, /**< alphabetic string, any script, lower case */
  wLexerTypeAlphaUpper, /**< alphabetic string, any script, first character in upper case */
  wLexerTypeAlphaCaps,  /**< alphabetic string, any script, all characters in upper case */
  wLexerTypeAlphaTrunc, /**< alphabetic string, any script, terminated by hyphen */

  wLexerTypeOther,      /**< any other character */

  NwLexerTypes          /**< number of lexer types */
};
typedef wasteLexerTypeE wasteLexerType;

/** Useful for debugging old dwdsScanner types */
extern const char *wasteLexerTypeNames[NwLexerTypes];

//--------------------------------------------------------------------------
///\name waste tag attribute access
//@{

/**< waste tag hidden attribute positions, relative to end-of-tag */
enum wasteTagAttrPosE {
  wtap_w = 1, //-- 'w': beginning-of-word
  wtap_S = 4, //-- 'S': end-of-sentence
  wtap_s = 7  //-- 's': beginning-of-sentence
};
typedef wasteTagAttrPosE wasteTagAttrPos;

/**
 * Get a boolean WASTE tag attribute by position relative to end-of-string
 */
inline bool waste_tag_attr_get(const std::string &tagstr, size_t rpos, bool mydefault=false)
{
  return tagstr.size() > rpos ? (tagstr[tagstr.size()-rpos]!='0') : mydefault;
};

/**
 * Set a boolean WASTE tag attribute position relative to end-of-string
 */
inline void waste_tag_attr_set(std::string &tagstr, size_t rpos, bool val)
{
  if (tagstr.size() <= rpos) throw std::overflow_error("waste_tag_attr_set(): tag string too short");
  tagstr[tagstr.size()-rpos] = (val ? '1' : '0');
};

//@}

//--------------------------------------------------------------------------
///\name lexer re2c wrappers, from wasteLexerTypes.cc
//@{

/** Get waste case-type for tok_text */
wasteLexerTypeE waste_casetype(const std::string &tok_text);

/** Get waste kexer-type for tok_text */
wasteLexerTypeE waste_lexertype(const std::string &tok_text);
//@}

}; /*--/namespace moot --*/

#endif /* _WASTE_TYPES_H */

