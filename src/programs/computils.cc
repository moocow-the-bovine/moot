/*
   moot-utils : moocow's part-of-speech tagger
   Copyright (C) 2002-2012 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: computils.cc
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot HMM PoS tagger/disambiguator : compilation utilities
 *--------------------------------------------------------------------------*/

#include "computils.h"

namespace moot {
  const char *PROGNAME = "moot::computils";
  int     vlevel;

  //--------------------------------------------------------------------------
  void put_hmm_header_begin(TokenWriter *writer)
  {
    time_t now_time = time(NULL);
    tm *now_tm = localtime(&now_time);
    writer->put_comment_block_begin();
    writer->printf_raw("\n %s output file generated on %s", PROGNAME, asctime(now_tm));
  }

  //--------------------------------------------------------------------------
  void put_hmm_header_config(TokenWriter *writer, mootHMM& hmm)
  {
    writer->printf_raw(" HMM Configuration:\n");
    writer->printf_raw("   Unknown Token     : %s\n", hmm.tokids.id2name(0).c_str());
    writer->printf_raw("   Unknown Tag       : %s\n", hmm.tagids.id2name(0).c_str());
    writer->printf_raw("   Border Tag        : %s\n", hmm.tagids.id2name(hmm.start_tagid).c_str());
#ifdef MOOT_USE_TRIGRAMS
    writer->printf_raw("   N-Gram lambdas    : lambda1=%g, lambda2=%g, lambda3=%g\n", exp(hmm.nglambda1), exp(hmm.nglambda2), exp(hmm.nglambda3));
#else
    writer->printf_raw("   N-Gram lambdas    : lambda1=%g, lambda2=%g\n", exp(hmm.nglambda1), exp(hmm.nglambda2));
#endif
    writer->printf_raw("   Hash n-grams?     : %s\n", (hmm.hash_ngrams ? "yes" : "no"));
    writer->printf_raw("   Relax?            : %s\n", (hmm.relax ? "yes" : "no"));
    writer->printf_raw("   Lex. Threshhold   : %g\n", hmm.unknown_lex_threshhold);
    writer->printf_raw("   Lexical lambdas   : lambdaw0=%g, lambdaw1=%g\n", exp(hmm.wlambda0), exp(hmm.wlambda1));
    writer->printf_raw("   Use classes?      : %s\n", hmm.use_lex_classes ? "yes" : "no");
    writer->printf_raw("   Use flavors?      : %s\n", hmm.use_flavors ? "yes" : "no");
    writer->printf_raw("   Num/Flavors       : %u\n", hmm.taster.size());
    writer->printf_raw("   Class Threshhold  : %g\n", hmm.unknown_class_threshhold);
    writer->printf_raw("   Class lambdas     : lambdac0=%g, lambdac1=%g\n", exp(hmm.clambda0), exp(hmm.clambda1));
    writer->printf_raw("   Beam Width        : %g\n", exp(hmm.beamwd));
#ifdef MOOT_ENABLE_SUFFIX_TRIE
    writer->printf_raw("   Suffix theta      : %g\n", hmm.suftrie.theta);
    writer->printf_raw("   Suffix trie size  : %u\n", hmm.suftrie.size());
#else
    writer->printf_raw("   Suffix theta      : (DISABLED)\n");
    writer->printf_raw("   Suffix trie size  : (DISABLED)\n");
#endif
  }

  //--------------------------------------------------------------------------
  void put_hmm_header_config(TokenWriter *writer, mootDynHMM& dyn)
  {
    writer->printf_raw(" DynHMM Configuration:\n");
    (&dyn)->tw_put_info(writer);
  }

  //--------------------------------------------------------------------------
  void put_hmm_header_end(TokenWriter *writer)
  {
    writer->put_comment_block_end();
  };

  //--------------------------------------------------------------------------
  void put_hmm_header(TokenWriter *writer, mootHMM    &hmm)
  {
    put_hmm_header_begin(writer);
    writer->printf_raw("\n");
    put_hmm_header_config(writer,hmm);
    writer->printf_raw("\n");
    put_hmm_header_end(writer);
  };

  //--------------------------------------------------------------------------
  void put_hmm_header(TokenWriter *writer, mootDynHMM &dyn)
  {
    put_hmm_header_begin(writer);
    writer->printf_raw("\n");
    put_hmm_header_config(writer, dyn);
    writer->printf_raw("\n");
    put_hmm_header_config(writer, (mootHMM&)dyn);
    writer->printf_raw("\n");
    put_hmm_header_end(writer);
  };

};
