/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdst.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : shared headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_H_
#define _DWDST_H_

#include <FSMTypes.h>
#include <FSMSymSpec.h>
#include <FSM.h>

// dwdst_tag_stream(in,out,morph,syms,want_attrs)
//  + tag a whole stream
extern bool dwdst_tag_stream(FILE *in,
			     FILE *out,
			     FSM *morph,
			     FSMSymSpec *syms,
			     bool want_attrs=false);

// dwdst_tag_token(token,out,morph,syms,want_attrs)
//  + tag a single token
extern bool dwdst_tag_token(char *tok,
			    FILE *out,
			    FSM *morph,
			    FSMSymSpec *syms,
			    bool want_attrs=false);

#endif // _DWDST_H_



