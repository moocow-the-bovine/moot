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
 * File: mootTokenIO.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token I/O
 *--------------------------------------------------------------------------*/

/*#include "mootTypes.h"*/
#include "mootTokenIO.h"

/*moot_BEGIN_NAMESPACE*/
namespace moot {

/*==========================================================================
 * TokenReader
 *==========================================================================*/

/*------------------------------------------------------------
 * TokenReader
 */
void TokenReader::carp(const char *fmt, ...)
{
  fprintf(stderr, "moot::TokenReader: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

/*------------------------------------------------------------
 * TokenReaderCooked
 */
/* mootSentence &get_sentence(void); */
mootTokFlavor TokenReaderCooked::get_sentence(void)
{
  int lxtok;

  msentence.clear();
  while ((lxtok = lexer.yylex()) != TF_EOF) {
    switch (lxtok) {

    case TF_COMMENT:
    case TF_TOKEN:
      msentence.push_back(lexer.mtoken);
      break;

    case TF_EOS:
      if (!msentence.empty()) return TF_EOS;
      break;

    default:
      //-- whoa
      fprintf(stderr,
	      "mootTokenIO::get_sentence(): unknown token type '%d' ignored!\n",
	      lxtok);
      break;
    }
  }
  return TF_EOF;
}

void TokenReaderCooked::carp(const char *fmt, ...)
{
  fprintf(stderr, "moot::TokenReaderCooked: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " in %s %s at line %d, column %d, near `%s'\n",
	  (lexer.use_string ? "string" : "file"),
	  (lexer.srcname.empty() ? "(null)" : lexer.srcname.c_str()),
	  lexer.theLine,
	  lexer.theColumn,
	  lexer.yytext);
}

/*------------------------------------------------------------
 * TokenReaderCookedFile
 */
//(empty)

/*------------------------------------------------------------
 * TokenReaderCookedString
 */
//(empty)



/*==========================================================================
 * TokenWriter
 *==========================================================================*/

/*------------------------------------------------------------
 * TokenWriterCooked
 */
//(empty)

/*------------------------------------------------------------
 * TokenWriterCookedString
 */
/* std::string &token2string(const mootToken &token, std::string &s); */
string &TokenWriterCookedString::token2string(const mootToken &token, string &s)
{
  s = token.text();
  if (token.flavor() == TF_COMMENT) return s;

  //-- best tag is always standalone first 'analysis'
  s.push_back('\t');
  s.append(token.besttag());

  if (want_best_only) {
    //-- best-only, full analyses
    for (mootToken::AnalysisSet::const_iterator ai = token.lower_bound(token.besttag());
	 ai != token.analyses().end() && ai->tag == token.besttag();
	 ai++ )
      {
	s.push_back('\t');
	s.append(ai->details.empty() ? ai->tag : ai->details);
	if (ai->cost != 0.0) {
	  s.push_back('<');
	  sprintf(costbuf, "%g", ai->cost);
	  s.append(costbuf);
	  s.push_back('>');
	}
      }
  } //-- /-tags,+best
  else {
    //-- all possibilities, full analyses
    for (mootToken::AnalysisSet::const_iterator ai = token.analyses().begin();
	 ai != token.analyses().end();
	 ai++ )
      {
	s.push_back('\t');
	s.append(ai->details.empty() ? ai->tag : ai->details);
	if (ai->cost != 0.0) {
	  s.push_back('<');
	  sprintf(costbuf, "%g", ai->cost);
	  s.append(costbuf);
	  s.push_back('>');
	}
      }
  } //-- -tags,-best
  return s;
};

/*void sentence2string(const mootSentence &sentence, std::string &s);*/
string &TokenWriterCookedString::sentence2string(const mootSentence &sentence, string &s)
{
  for (mootSentence::const_iterator si = sentence.begin();
       si != sentence.end();
       si++)
    {
      token2string(*si, s);
      s.push_back('\n');
    }
  return s;
};

/*------------------------------------------------------------
 * TokenWriterCookedFile
 */
void TokenWriterCookedFile::put_token(const mootToken &token)
{
  /*
  string s;
  fputs(token2string(token,s).c_str(), out);
  fputc('\n', out);
  */
  fputs(token.text().c_str(), out);
  if (token.flavor() == TF_COMMENT) {
    fputc('\n', out);
    return;
  }

  //-- best tag is always standalone first 'analysis'
  fputc('\t',out);
  fputs(token.besttag().c_str(),out);

  if (want_best_only) {
    //-- best-only, full analyses
    for (mootToken::AnalysisSet::const_iterator ai = token.lower_bound(token.besttag());
	 ai != token.analyses().end() && ai->tag == token.besttag();
	 ai++ )
      {
	fputc('\t',out);
	fputs((ai->details.empty() ? ai->tag.c_str() : ai->details.c_str()), out);
	if (ai->cost != 0.0) {
	  fprintf(out,"<%g>",ai->cost);
	}
      }
  } //-- /-tags,+best
  else {
    //-- all possibilities, full analyses
    for (mootToken::AnalysisSet::const_iterator ai = token.analyses().begin();
	 ai != token.analyses().end();
	 ai++ )
      {
	fputc('\t',out);
	fputs((ai->details.empty() ? ai->tag.c_str() : ai->details.c_str()), out);
	if (ai->cost != 0.0) {
	  fprintf(out,"<%g>",ai->cost);
	}
      }
    } //-- -tags,-best
  fputc('\n',out);
}

void TokenWriterCookedFile::put_sentence(const mootSentence &sentence)
{
  for (mootSentence::const_iterator si = sentence.begin();
       si != sentence.end();
       si++)
    {
      put_token(*si);
    }
  fputc('\n', out);
}


}; /*moot_END_NAMESPACE*/
