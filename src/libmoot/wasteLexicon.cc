/* -*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*- */
/*
   libmoot : moot part-of-speech tagging library
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

#include "wasteLexicon.h"

moot_BEGIN_NAMESPACE

/*============================================================================
 * wasteLexicon
 */

//----------------------------------------------------------------------
bool wasteLexicon::load(moot::TokenReader *reader)
{
  mootTokenType toktyp;
  while ( (toktyp=reader->get_token()) != TokTypeEOF ) {
    if (toktyp==TokTypeVanilla || toktyp==TokTypeLibXML)
      insert( reader->token()->text() );
  }
  return true;
}

//----------------------------------------------------------------------
bool wasteLexicon::load(mootio::mistream *mis)
{
  TokenReaderNative tr(tiofRare, "wasteLexicon::load::TokenReaderNative");
  tr.from_mstream(mis);
  bool rc = load(&tr);
  return rc;
}

//----------------------------------------------------------------------
bool wasteLexicon::load(const char *filename)
{
  mootio::mifstream mis(filename,"r");
  bool rc = load(&mis);
  return rc;
}

moot_END_NAMESPACE
