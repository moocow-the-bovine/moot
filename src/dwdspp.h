/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdspp.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + tokenizer for KDWDS project : shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDSPP_H_
#define _DWDSPP_H_

#include <stdio.h>

// dwdspp_tokenize_stream(in,out,morph,syms,want_attrs)
//  + tag a whole stream
extern bool dwdspp_tokenize_stream(FILE *in, FILE *out);

#endif // _DWDSPP_H_



