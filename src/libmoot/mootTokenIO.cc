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

/*------------------------------------------------------------
 * TokenReader
 *------------------------------------------------------------*/

/* void select_stream(FILE *in=stdin, char *srcname=NULL); */
void TokenReader::select_stream(FILE *in, char *source_name)
{
  //-- prepare lexer
  lexer.select_streams(in,stderr);
  lexer.srcname   = source_name;
  lexer.theLine   = 1;
  lexer.theColumn = 0;
}

/* void select_string(const char *instr, char *srcname=NULL); */
void TokenReader::select_string(const char *instr, char *source_name)
{
    lexer.select_string(instr,stderr);
    lexer.srcname = source_name;
    lexer.theLine = 1;
    lexer.theColumn = 0;
}


/* mootSentence &get_sentence(void); */
mootSentence &TokenReader::get_sentence(void)
{
  int lxtok;
  sentence.clear();
  while ((lxtok = lexer.yylex()) != mootTokenLexer::TLEOF) {
    if (lxtok == mootTokenLexer::TLEOS) {
      if (!sentence.empty()) return sentence;
      continue;
    }
    sentence.push_back(lexer.mtoken);
  }
  return sentence;
}


/*------------------------------------------------------------
 * TokenWriter
 *------------------------------------------------------------*/
string TokenWriter::token_string(const mootToken &token)
{
  string s = token.text();
  if (want_tags_only || token.analyses().empty()) {
    if (want_best_only || token.analyses().empty()) {
      //-- best-only, tags-only: exactly 1 tag/tok
      s.push_back('\t');
      s.append(token.besttag());
    } //-- /+tags,+best
    else {
      //-- tags-only, all possibilities
      set<mootTagString> tagset;
      token.tokExport(NULL,&tagset);
      for (set<mootTagString>::const_iterator tsi = tagset.begin();
	   tsi != tagset.end();
	   tsi++)
	{
	  s.push_back('\t');
	  if (*tsi == token.besttag()) s.push_back('/');
	  s.append(*tsi);
	}
    } //-- /+tags,-best
  } //-- /+tags
  else if (want_best_only) {
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
	if (ai->tag == token.besttag()) s.push_back('/');
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
}

void TokenWriter::token_put(FILE *out, const mootToken &token)
{
  /*
  fputs(token_string(token).c_str(), out);
  fputc('\n', out);
  */
  fputs(token.text().c_str(), out);
  if (want_tags_only || token.analyses().empty()) {
    if (want_best_only || token.analyses().empty()) {
      //-- best-only, tags-only: exactly 1 tag/tok
      fputc('\t',out);
      fputs(token.besttag().c_str(),out);
    } //-- /+tags,+best
    else {
      //-- tags-only, all possibilities
      set<mootTagString> tagset;
      token.tokExport(NULL,&tagset);
      for (set<mootTagString>::const_iterator tsi = tagset.begin();
	   tsi != tagset.end();
	   tsi++)
	{
	  fputc('\t',out);
	  if (*tsi == token.besttag()) fputc('/',out);
	  fputs(tsi->c_str(),out);
	}
    } //-- /+tags,-best
  } //-- /+tags
  else if (want_best_only) {
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
	if (ai->tag == token.besttag()) fputc('/',out);
	fputs((ai->details.empty() ? ai->tag.c_str() : ai->details.c_str()), out);
	if (ai->cost != 0.0) {
	  fprintf(out,"<%g>",ai->cost);
	}
      }
    } //-- -tags,-best
  fputc('\n',out);
}



void TokenWriter::sentence_put(FILE *out, const mootSentence &sentence)
{
  for (mootSentence::const_iterator si = sentence.begin();
       si != sentence.end();
       si++)
    {
      token_put(out, *si);
    }
  fputc('\n', out);
}


}; /*moot_END_NAMESPACE*/
