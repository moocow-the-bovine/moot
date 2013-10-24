/* -*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*- */

/*
   libmoot : moot part-of-speech tagging library
   Copyright (C) 2013 by Bryan Jurish <moocow@cpan.org>

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

#include "wasteTypes.h"
#include "wasteScanner.h"
namespace moot {

/*==========================================================================
 * wasteScanner
 */

//----------------------------------------------------------------------
wasteScanner::wasteScanner(const std::string &myname,
			   size_t line,
			   size_t column,
			   mootio::ByteOffset byte,
			   size_t init_bufsize)
  : GenericLexer(myname,line,column,byte),
    yybuffer(NULL),
    yycursor(NULL),
    yylimit(NULL),
    yytoken(NULL),
    yymarker(NULL),
    buffer_size(init_bufsize)
{
  yybuffer = new char[buffer_size];
  yycursor = yylimit = yytoken = yymarker = yybuffer;
}

//----------------------------------------------------------------------
wasteScanner::~wasteScanner()
{
  if (yybuffer)
    delete[] yybuffer;
}

//----------------------------------------------------------------------
bool wasteScanner::fill( size_t n )
{
  if ( !mglin || mglin->eof() )
  {
    if ( (yylimit - yycursor) <= 0 ) {
      return false;
    }
  }

  ptrdiff_t restSize = yylimit - yytoken;
  if ( restSize + n >= buffer_size )
  {
    // extend buffer
    buffer_size *= 2;
    char* newBuffer = new char[buffer_size];
    for ( ptrdiff_t i = 0; i < restSize; ++i )
    {
      // memcpy
      *( newBuffer + i ) = *( yytoken + i );
    }
    yymarker = newBuffer + ( yymarker - yytoken );
    yycursor = newBuffer + ( yycursor - yytoken );
    yytoken = newBuffer;
    yylimit = newBuffer + restSize;

    delete[] yybuffer;
    yybuffer = newBuffer;
  }
  else
  {
    // move remained data to head.
    for ( ptrdiff_t i = 0; i < restSize; ++i )
    {
      //memmove( yybuffer, yytoken, (restSize)*sizeof(char) );
      *( yybuffer + i ) = * ( yytoken + i );
    }
    yymarker = yybuffer + ( yymarker - yytoken );
    yycursor = yybuffer + ( yycursor - yytoken );
    yytoken = yybuffer;
    yylimit = yybuffer + restSize;
  }

  // fill to buffer
  size_t read_size = buffer_size - restSize;
  yylimit += mglin->read ( yylimit, read_size );

  return true;
}

//----------------------------------------------------------------------
void wasteScanner::reset(void)
{
  GenericLexer::reset();
  yycursor = yylimit = yytoken = yymarker = yybuffer;
}

//----------------------------------------------------------------------
void wasteScanner::from_mstream (mootio::mistream *in)
{
    if (in) {
      if (mglin && in != mglin && mgl_in_created) {
	delete mglin;
	mgl_in_created = false;
      }
      mglin = in;
    }
    reset();
}

/*==========================================================================
 * wasteTokenScanner
 */

//----------------------------------------------------------------------
wasteTokenScanner::wasteTokenScanner(int fmt, const std::string &name)
  : TokenReader(fmt,name),
    scanner(name)
{
  tr_token = &wts_token;
  tr_sentence = &wts_sentence;
}

//----------------------------------------------------------------------
wasteTokenScanner::~wasteTokenScanner(void)
{}

//----------------------------------------------------------------------
void wasteTokenScanner::from_mstream(mootio::mistream *mistreamp)
{
  scanner.from_mstream(mistreamp);
}

//----------------------------------------------------------------------
void wasteTokenScanner::close(void)
{
  wts_token.clear();
  wts_sentence.clear();
  TokenReader::close();
}

//----------------------------------------------------------------------
mootTokenType wasteTokenScanner::get_token(void)
{
  int scantyp = scanner.yylex();
  wts_token.clear();

  //-- token type
  switch ( scantyp ) {
  case wScanTypeEOF:		return wts_token.toktype(TokTypeEOF);
  case wScanTypeNewline:	wts_token.text("\\n"); break;
  case wScanTypeWB:		wts_token.toktype(TokTypeWB); break;
  case wScanTypeSB:		wts_token.toktype(TokTypeSB); break;
  case wScanTypeComment:	wts_token.toktype(TokTypeComment); break;
  default:			break;
  }

  //-- token text
  if (wts_token.text().empty())
    wts_token.text(scanner.yytext());

  //-- token location
  wts_token.location( scanner.theByte-scanner.yyleng(), scanner.yyleng() );

  //-- +Tagged: set tag text
  if ( tr_format&tiofTagged )
    wts_token.besttag( wasteScannerTypeNames[scantyp] );

  //-- +Analyzed: set analysis text
  if ( tr_format&tiofAnalyzed )
    wts_token.insert( wasteScannerTypeNames[scantyp], "" );

  //-- return
  return wts_token.toktype();
}

//----------------------------------------------------------------------
mootTokenType wasteTokenScanner::get_sentence(void)
{
  wts_sentence.clear();
  mootTokenType toktyp;
  for (toktyp=get_token(); toktyp != TokTypeEOF; toktyp=get_token()) {
    wts_sentence.push_back( wts_token );
  }
  return toktyp;
}


}; //-- /namespace moot;
