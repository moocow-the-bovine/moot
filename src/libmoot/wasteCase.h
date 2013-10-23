/*-*- Mode: C++; coding: utf-8; c-basic-offset: 2; -*-*/
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
 * File: wasteToLower.h
 * Author: Bryan Jurish <moocow@cpan.org>, Kay-Michael Würzner
 * Description:
 *   + moot PoS tagger : WASTE HMM tokenizer: re2c utf8 case-folding
 *--------------------------------------------------------------------------*/

/**
\file wasteCase.h
\brief case-folding operation(s) for UTF-8 strings
*/

#ifndef _WASTE_CASE_H
#define _WASTE_CASE_H

#include <string>

namespace moot {

/** Return a lower-cased version of \a s ; in- and outputs are UTF-8 encoded byte strings */
std::string utf8ToLower(const std::string &s);

};

#endif /* _WASTE_CASE_H */
