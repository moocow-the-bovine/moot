/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2005 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootEval.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : evaluator : guts
 *--------------------------------------------------------------------------*/


#include <mootEval.h>
#include <mootToken.h>
#include <string>


moot_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * compareTokens(tok1,tok2)
 */
int mootEval::compareTokens(const mootToken &tok1, const mootToken &tok2)
{
  status = MEF_None;

  if (tok1.text() != tok2.text()) status |= MEF_TokMismatch;

  if (tok1.besttag() != tok2.besttag()) status |= MEF_BestMismatch;

  if (tok1.analyses().empty()) {
    //status |= (MEF_EmptyClass1|MEF_ImpClass1|MEF_XImpClass1);
    status |= MEF_EmptyClass1;
  } else {
    if (!tok1.has_analysis_for_tag(tok1.besttag()))
	status |= MEF_ImpClass1;

    if (!tok1.has_analysis_for_tag(tok2.besttag()))
	status |= MEF_XImpClass1;
  }

  if (tok2.analyses().empty()) {
    //status |= (MEF_EmptyClass1|MEF_ImpClass2|MEF_XImpClass2);
    status |= MEF_EmptyClass2;
  } else {
    if (!tok2.has_analysis_for_tag(tok2.besttag()))
	status |= MEF_ImpClass2;

    if (!tok2.has_analysis_for_tag(tok1.besttag()))
	status |= MEF_XImpClass2;
  }

  return status;
}

/*--------------------------------------------------------------------------
 * compareSentences(sent1,sent2)
 */
//list<mootEval::evalFlag> compareSentences(const mootSentence &sent1, const mootSentence &sent2);


/*--------------------------------------------------------------------------
 * status_string()
 */
string mootEval::status_string(void)
{
  string str;
  str.reserve(10);
  str.push_back(status&MEF_TokMismatch  ? 't' : '-');
  str.push_back(status&MEF_BestMismatch ? 'b' : '-');
  str.push_back(':');
  str.push_back(status&MEF_EmptyClass1  ? 'e' : '-');
  str.push_back(status&MEF_ImpClass1    ? 'i' : '-');
  str.push_back(status&MEF_XImpClass1   ? 'x' : '-');
  str.push_back(':');
  str.push_back(status&MEF_EmptyClass2  ? 'e' : '-');
  str.push_back(status&MEF_ImpClass2    ? 'i' : '-');
  str.push_back(status&MEF_XImpClass2   ? 'x' : '-');
  return str;
}


moot_END_NAMESPACE
