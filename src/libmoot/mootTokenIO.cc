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
  while ((lxtok = lexer.yylex()) != TF_EOF) {
    switch (lxtok) {

    case TF_COMMENT:
    case TF_TOKEN:
      sentence.push_back(lexer.mtoken);
      break;

    case TF_EOS:
      if (!sentence.empty()) return sentence;
      break;

    default:
      //-- whoa
      fprintf(stderr, "mootTokenIO::get_sentence(): unknown token type '%d' ignored!\n", lxtok);
      break;
    }
  }
  return sentence;
}


/*------------------------------------------------------------
 * TokenWriter
 *------------------------------------------------------------*/
string TokenWriter::token_string(const mootToken &token)
{
  string s = token.text();
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
}

void TokenWriter::token_put(FILE *out, const mootToken &token)
{
  /*
  fputs(token_string(token).c_str(), out);
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
