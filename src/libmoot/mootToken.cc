/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2014 by Bryan Jurish <moocow@cpan.org>

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
 * File: mootToken.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : token information
 *--------------------------------------------------------------------------*/

#include "mootToken.h"
#include "mootUtils.h"

namespace moot {
  using namespace std;

/*--------------------------------------------------------------------------
 * globals
 */
const char* mootTokenTypeNames[NTokTypes] =
  {
    "Unknown",
    "Vanilla",
    "LibXML",
    "XMLRaw",
    "Comment",
    "EOS",
    "EOF",
    "WB",
    "SB",
    "User"
  };

/*--------------------------------------------------------------------------
 * mootToken::Analysis
 */
void mootToken::Analysis::dump(const char *label, FILE *f) const
{
  if (f==NULL) f=stderr;
  fputs(" + ",f);
  if (label) {
    fputs(label,f);
    fputc(' ',f);
  }
  fprintf(f, "ANALYSIS %p <%f>: tag=", this, prob);
  fwrite(tag.data(), 1, tag.size(), f);
  fprintf(f, " ; details=");
  fwrite(details.data(), 1, details.size(), f);
  fputc('\n', f);
  fflush(f);
}
  
/*--------------------------------------------------------------------------
 * mootToken
 */
void mootToken::dump(const char *label, FILE *f) const
{
  if (f==NULL) f=stderr;
  if (label) {
    fputs(label,f);
    fputc(' ',f);
  }
  fprintf(f, "TOKEN:%s %p [%lu %lu]: ", mootTokenTypeNames[tok_type], this, tok_location.offset, tok_location.length);
  fwrite(tok_text.data(), 1, tok_text.size(), f);
  fputc('\n',f);
  for (mootToken::Analyses::const_iterator ai=tok_analyses.begin(); ai != tok_analyses.end(); ++ai) {
    ai->dump(NULL,f);
  }
  fflush(f);
}

/*--------------------------------------------------------------------------
 * mootSentence
 */
mootToken &sentence_printf_append(mootSentence &s, mootTokenType typ, const char *fmt, ...)
{
  va_list ap;
  s.push_back(mootToken(typ));
  va_start(ap,fmt);
  std_vsprintf(s.back().tok_text, fmt, ap);
  va_end(ap);
  return s.back();
};

}; // moot_END_NAMESPACE
