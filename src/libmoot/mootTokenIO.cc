/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2010 by Bryan Jurish <jurish@uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootTokenIO.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : token I/O
 *--------------------------------------------------------------------------*/

#include <mootConfig.h>

#include <mootTokenIO.h>
#include <mootUtils.h>
#include <assert.h>

#ifdef MOOT_LIBXML_ENABLED
# include <mootTokenXmlDoc.h>
#endif /* MOOT_LIBXML_ENABLED */

#ifdef MOOT_EXPAT_ENABLED
# include <mootTokenExpatIO.h>
#endif /* MOOT_EXPAT_ENABLED */

/*-- gnulib stuff --*/
/*#include "vasprintf.h"*/ //-- chokes on CentOS 5.4 (~RHEL5)


/*moot_BEGIN_NAMESPACE*/
namespace moot {
  using namespace std;

/*==========================================================================
 * TokenIO
 *==========================================================================*/

/*------------------------------------------------------------
 * TokenIO: parse_format_string()
 */
int TokenIO::parse_format_string(const std::string &fmtString)
{
  int flags = 0;
  int flag = 0;
  bool neg = false;
  list<string> sflags = moot_strtok(fmtString,", ");

  for (list<string>::iterator sfi = sflags.begin();
       sfi != sflags.end();
       sfi++)
    {
      string &s = *sfi;
      if (s[0] == '!') {
	neg = true;
	s.erase(0,1);
      }
      else neg = false;

      //-- canonicalize to lower case
      for (string::iterator si = s.begin(); si != s.end(); si++) {
	*si = tolower(*si);
      }

      //-- get flag bitmask
      if (s=="none") flag = tiofNone;
      else if (s=="null") flag = tiofNull;
      else if (s=="unknown") flag = tiofUnknown;
      else if (s=="native") flag = tiofNative;
      else if (s=="xml") flag = tiofXML;
      //else if (s=="expat") flag = tiofExpat;
      else if (s=="conserve") flag = tiofConserve;
      else if (s=="pretty") flag = tiofPretty;
      else if (s=="text") flag = tiofText;
      else if (s=="analyzed") flag = tiofAnalyzed;
      else if (s=="tagged") flag = tiofTagged;
      else if (s=="user") flag = tiofUser;
      else if (s=="pruned") flag = tiofPruned;
      else if (s=="location" || s=="loc") flag =tiofLocation;
      //-- aliases
      else if (s=="rare" || s=="r") flag = tiofRare;
      else if (s=="mediumrare" || s=="mr") flag = tiofMediumRare;
      else if (s=="medium" || s=="m") flag = tiofMedium;
      else if (s=="welldone" || s=="wd") flag = tiofWellDone;
      else {
	flag = 0;
	fprintf(stderr,
		"TokenIO::parse_format_string(): unknown format flag \"%s\"\n",
		s.c_str());
	continue;
      }

      if (neg) flags &= ~flag;
      else flags |= flag;
    }
  return flags;
}

/*------------------------------------------------------------
 * TokenIO: guess_filename_format()
 */
int TokenIO::guess_filename_format(const char *filename)
{
  if (!filename) return tiofNone;

  int flags = 0;
  list<string> exts = moot_strtok(filename,".");

  for (list<string>::reverse_iterator exti = exts.rbegin();
       exti != exts.rend();
       exti++)
    {
      if      (*exti == "t"   ) flags |= tiofRare;
      else if (*exti == "r"   ) flags |= tiofRare;
      else if (*exti == "rt"  ) flags |= tiofRare;

      else if (*exti == "mr"  ) flags |= tiofMediumRare;
      else if (*exti == "mrt" ) flags |= tiofMediumRare;

      else if (*exti == "tt"  ) flags |= tiofMedium;
      else if (*exti == "ttt" ) flags |= tiofMedium;
      else if (*exti == "m"   ) flags |= tiofMedium;
      else if (*exti == "mt"  ) flags |= tiofMedium;

      else if (*exti == "wd"  ) flags |= tiofWellDone;
      else if (*exti == "wdt" ) flags |= tiofWellDone;

      else if (*exti == "xml" ) flags |= tiofXML;
      else if (*exti == "moot") flags |= tiofNative;

      else {
	//-- unknown extension - break off search
	break;
      }
    }
  return flags;
}

/*------------------------------------------------------------
 * TokenIO: is_empty_format()
 */
bool TokenIO::is_empty_format(int fmt)
{
  return
    //( (fmt&~(tiofNative|tiofXML|tiofExpat|tiofPretty)) == 0 )
    ( (fmt&(tiofText|tiofAnalyzed|tiofTagged)) == 0 )
    ;
}

/*------------------------------------------------------------
 * TokenIO: sanitize_format()
 */
int TokenIO::sanitize_format(int fmt, int fmt_implied, int fmt_default)
{
  int san = fmt|fmt_implied;

  //-- handle empty formats
  if (is_empty_format(san)) san |= fmt_default;

  //-- select only one basic format
  if      (fmt & tiofXML)    san &= ~tiofNative;
  else if (fmt & tiofNative) san &= ~tiofXML;
  else if (!(fmt&tiofUser))  san |= tiofNative;

  //-- and return
  return san;
}

/*------------------------------------------------------------
 * TokenIO: parse_format_request()
 */
int TokenIO::parse_format_request(const char *request,
				  const char *filename,
				  int fmt_implied,
				  int fmt_default)
{
  int fmt = 0;

  if (request && *request)
    fmt |= parse_format_string(request ? request : "");
  else
    fmt |= guess_filename_format(filename);

  return sanitize_format(fmt,fmt_implied,fmt_default);
}

/*------------------------------------------------------------
 * TokenIO: format_canonical_string()
 */
std::string TokenIO::format_canonical_string(int fmt)
{
  string s;
  if (fmt & tiofUnknown) s.append("Unknown,");
  if (fmt & tiofNull) s.append("Null,");
  if (fmt & tiofNative) s.append("Native,");
  if (fmt & tiofXML) s.append("XML,");
  //if (fmt & tiofExpat) s.append("Expat,");
  if (fmt & tiofConserve) s.append("Conserve,");
  if (fmt & tiofPretty) s.append("Pretty,");
  if (fmt & tiofText) s.append("Text,");
  if (fmt & tiofAnalyzed) s.append("Analyzed,");
  if (fmt & tiofTagged) s.append("Tagged,");
  if (fmt & tiofLocation) s.append("Location,");
  if (fmt & tiofPruned) s.append("Pruned,");
  if (fmt & tiofUser) s.append("User,");

  if (s.empty()) s = "None";
  else s.erase(s.size()-1);

  return s;
}


/*------------------------------------------------------------
 * TokenIO: new_reader()
 */
class TokenReader *TokenIO::new_reader(int fmt)
{
  //-- format dispatch
  if      (fmt & tiofNative) { return new TokenReaderNative(fmt); }

  //else if (fmt & (tiofExpat|tiofXML)) {
else if (fmt & tiofXML) {
#ifdef MOOT_EXPAT_ENABLED
    return new TokenReaderExpat(fmt);
#elif MOOT_LIBXML_ENABLED
    return new TokenXmlDoc(fmt);
#else
    throw domain_error("XML support disabled");
#endif
  }

  //-- ... more here ...

  //-- default to native format
  return new TokenReaderNative(fmt);
}

/*------------------------------------------------------------
 * TokenIO: new_writer()
 */
class TokenWriter *TokenIO::new_writer(int fmt)
{
  //-- format dispatch

  //-- native output
  if (fmt&tiofNative)
    {
      return new TokenWriterNative(fmt);
    }
  
  //-- XML output
  //else if (fmt & (tiofXML|tiofExpat)) {
  else if (fmt & tiofXML) {
#ifdef MOOT_EXPAT_ENABLED
    return new TokenWriterExpat(fmt);
#elif MOOT_LIBXML_ENABLED
    return new TokenXmlDoc(fmt);
#else
    throw domain_error("XML support disabled");
#endif
  }

  //-- ... more here ...

  //-- default to native format
  return new TokenWriterNative(fmt);
}

/*==========================================================================
 * TokenReader
 *==========================================================================*/


/*------------------------------------------------------------
 * Reader : Methods : Input : get_sentence
 */
mootTokenType TokenReader::get_sentence(void)
{
  if (!tr_sentence) return TokTypeEOF;

  int lxtyp = TokTypeUnknown;
  mootToken *tr_token_old = tr_token;
  tr_sentence->clear();

  while (lxtyp != TokTypeEOS && lxtyp != TokTypeEOF) {
    //-- allocate new destination token
    tr_sentence->push_back(mootToken());
    tr_token = &(tr_sentence->back());
    lxtyp = get_token();
  }

  tr_token = tr_token_old;
  return static_cast<mootTokenType>(lxtyp);
}

/*------------------------------------------------------------
 * TokenReader : Diagnostics : carp
 */
void TokenReader::carp(const char *fmt, ...)
{
  fprintf(stderr, "%s: ", tr_name.c_str());
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " in \"%s\" at byte %zd: line %zd, column %zd\n",
	  ((tr_istream
	    && tr_istream->valid()
	    && !tr_istream->name.empty()) ? tr_istream->name.c_str() : "(unknown)"),
	  static_cast<ssize_t>(byte_number()),
	  line_number(),
	  column_number());
}


/*==========================================================================
 * TokenReaderNative
 *==========================================================================*/

/*------------------------------------------------------------
 * Reader : Native : Input Selection
 */
//(in header)

/*----------------------------------------------
 * Reader : Native : Input Selection : from_mstream()
 */
void TokenReaderNative::from_mstream(mootio::mistream *mis)
{
  TokenReader::from_mstream(mis);
  lexer.from_mstream(mis);
}


/*------------------------------------------------------------
 * Reader : Native : Methods : get_token()
 */
mootTokenType TokenReaderNative::get_token(void)
{
  tr_token = lexer.mtoken = &(lexer.mtoken_default); //-- grab to lexer-internal token
  return static_cast<mootTokenType>(lexer.yylex());
};

/*------------------------------------------------------------
 * Reader : Native : Methods : get_sentence()
 */
mootTokenType TokenReaderNative::get_sentence(void)
{
  if (!tr_sentence) tr_sentence = &trn_sentence;
  int lxtyp = TokTypeUnknown;
  tr_sentence->clear();
  while (lxtyp != TokTypeEOS && lxtyp != TokTypeEOF) {
    //-- allocate new destination token
    tr_sentence->push_back(mootToken());
    lexer.mtoken = &(tr_sentence->back());
    lxtyp = lexer.yylex();
  }
  tr_sentence->pop_back();
  return static_cast<mootTokenType>(lxtyp);
};


/*==========================================================================
 * TokenWriter
 *==========================================================================*/

/*==========================================================================
 * TokenWriter
 */

/*------------------------------------------------------------
 * TokenWriter : Output : put_sentence()
 */
//(in class def)

/*------------------------------------------------------------
 * TokenWriter : Comments : printf_comment()
 */
void TokenWriter::printf_comment(const char *fmt, ...)
{
  char *buf =NULL;
  int len =0;
  va_list ap;
  va_start(ap, fmt);
  len = vasprintf(&buf, fmt, ap);
  va_end(ap);
  if (len>=0 && buf) {
    put_comment_buffer(buf,static_cast<size_t>(len));
    free(buf);
  }
}

/*------------------------------------------------------------
 * TokenWriter : Raw Data : printf_raw()
 */
void TokenWriter::printf_raw(const char *fmt, ...)
{
  char *buf =NULL;
  int len=0;
  va_list ap;
  va_start(ap, fmt);
  len = vasprintf(&buf, fmt, ap);
  va_end(ap);
  if (len>=0 && buf) {
    put_raw_buffer(buf,len);
    free(buf);
  }
}

/*------------------------------------------------------------
 * TokenWriter : Diagnostics : carp()
 */
void TokenWriter::carp(const char *fmt, ...)
{
  fprintf(stderr, "%s: ", tw_name.c_str());
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}



/*==========================================================================
 * TokenWriterNative
 */

/*------------------------------------------------------------
 * TokenWriterNative : Output : Utilities : _put_token()
 */
void TokenWriterNative::_put_token(const mootToken &token, mootio::mostream *os)
{
  if (!os || !os->valid() || tw_format & tiofNull) return;

  switch (token.toktype()) {

  case TokTypeComment:
    put_comment_buffer(token.text().data(), token.text().size());
    return;

  case TokTypeVanilla:
  case TokTypeLibXML:
    if (tw_is_comment_block) os->puts("%%");
    if (tw_format & tiofText) {
      os->puts(token.text());
    }

    if (tw_format & tiofTagged) {
      //-- best tag is always standalone first 'analysis'
      os->putbyte('\t');
      os->puts(token.besttag());
    }

    if (tw_format & tiofLocation) {
      //-- location might be first non-tag 'analysis'
      const mootToken::Location loc = token.location();
      os->putbyte('\t');
      os->printf("%lu %lu", loc.offset, loc.length);
    }


    if (tw_format & tiofAnalyzed) {
      for (mootToken::Analyses::const_iterator ai = token.analyses().begin();
	   ai != token.analyses().end();
	   ai++)
	{
	  if ((tw_format & tiofPruned) && ai->tag != token.besttag()) continue;
	  os->putbyte('\t');
	  if (tw_format & tiofPretty) {
	    os->puts(moot_normalize_ws((ai->details.empty() ? ai->tag : ai->details),
				       true, true));
	  } else {
	    os->puts(ai->details.empty() ? ai->tag : ai->details);
	  }
	  /*
	  if (ai->prob != 0) {
	    os->printf(" <%g>", static_cast<double>(ai->prob));
	  }
	  */
	}
    }
    os->putbyte('\n');
    break;

  case TokTypeEOS:
    if (tw_is_comment_block) os->puts("%%\n");
    else os->putbyte('\n');
    break;

  default:
    //-- ignore
    break;
  }
}

/*------------------------------------------------------------
 * TokenWriterNative : Output : Utilities : put_sentence()
 */
void TokenWriterNative::_put_sentence(const mootSentence &sentence, mootio::mostream *os)
{
  if (!os || !os->valid() || tw_format & tiofNull) return;
  for (mootSentence::const_iterator si = sentence.begin();
       si != sentence.end();
       si++)
    {
      _put_token(*si, os);
    }
  if (!sentence.empty() || sentence.back().toktype() != TokTypeEOS) {
    os->putbyte('\n');
  }
}


/*------------------------------------------------------------
 * TokenWriterNative : Output : Utilities : Comment
 */
void TokenWriterNative::_put_comment(const char *buf, size_t len, mootio::mostream *os)
{
  if (!os || !os->valid() || tw_format & tiofNull) return;

  if (len == 0) {
    os->putbyte('\n');
    return;
  }

  size_t i, j;
  for (i=0; i < len; i=j+1) {
    for (j=i; j < len && buf[j] != '\n'; j++)
      ;
    os->puts("%%");
    os->write(buf+i, j-i);
    os->putbyte('\n');
  }
}

/*------------------------------------------------------------
 * Writer : Native : Output : Utilities : Raw
 */
void TokenWriterNative::_put_raw_buffer(const char *buf, size_t len, mootio::mostream *os)
{
  if (!os || !os->valid() || tw_format & tiofNull)
    return;
  else if (tw_is_comment_block)
    _put_comment(buf, len, os);
  else
    os->write(buf,len);
}


}; /*moot_END_NAMESPACE*/
