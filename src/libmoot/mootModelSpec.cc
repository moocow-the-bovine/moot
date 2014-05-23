/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2014 Jurish <moocow@cpan.org>

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
 * File: mootModelSpec.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : model specification
 *--------------------------------------------------------------------------*/

#include <mootModelSpec.h>

#include <mootConfig.h>
#include <mootUtils.h>

namespace moot {
  using namespace std;
  using namespace mootio;

//==============================================================================
// mootModelSpec

//----------------------------------------------------------------------
void mootModelSpec::clear(void)
{
  binfile.clear();
  lexfile.clear();
  ngfile.clear();
  lcfile.clear();
  flafile.clear();
};

//----------------------------------------------------------------------
bool mootModelSpec::parse(const string &modelname, bool try_bin)
{
  clear();

  if (try_bin && moot_file_exists(modelname)) {
    //-- binary model
    binfile = modelname;
    return !binfile.empty();
  }

  //-- text model
  if (modelname.find(',') != modelname.npos) {
    //-- comma-separated list LEX,123,CLX,FLA
    vector<string> ml = moot_strsplit(modelname,",");
    size_t    ml_size = ml.size();
    if (ml_size > 0) lexfile = ml[0];
    if (ml_size > 1) ngfile  = ml[1];
    if (ml_size > 2) lcfile  = ml[2];
    if (ml_size > 3) flafile = ml[3];
  } else {
    //-- no commas: expand modelname as basename
    lexfile = modelname + ".lex";
    //if (!moot_file_exists(lexfile)) lexfile.clear();  //-- not here: (used for training!)

    ngfile = modelname + ".123";
    //if (!moot_file_exists(ngfile)) ngfile.clear(); //-- not here: (used for training!)

    lcfile = modelname + ".clx";
    //if (!moot_file_exists(lcfile)) lcfile.clear(); //-- not here: (used for training!)

    flafile = modelname + ".fla";
    //if (!moot_file_exists(flafile)) flafile.clear(); //-- not here: (used for training!)
  }

  return !(lexfile.empty() && ngfile.empty() && lcfile.empty() && flafile.empty());
}

}; //-- namespace moot

