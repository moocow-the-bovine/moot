/* -*- Mode: C++ -*- */
/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2012 by Bryan Jurish <moocow@cpan.org>

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
 * File: moot.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : top level headers
 *--------------------------------------------------------------------------*/

/**
 * \file moot.h
 * \brief \b top-level header file (for lazy programmers)
 */

#ifndef _MOOT_H
#define _MOOT_H

/*----------------------------------------------------------------------
 * autoheader configuration (safe)
 */
#include <mootConfig.h>

//----------------------------------------------------------------------
// Top-Level includes
#include <mootHMMTrainer.h>   /*-- tagger model training --*/
#include <mootHMM.h>          /*-- runtime tagging --*/
#include <mootDynHMM.h>       /*-- runtime tagging, dynamic model --*/
#include <mootEval.h>         /*-- tagger output evaluation --*/

//----------------------------------------------------------------------
// Low-level utilties
#include <mootSTLHacks.h>
#include <mootTypes.h>
#include <mootUtils.h>
#include <mootModelSpec.h>
#include <mootEnum.h>

//----------------------------------------------------------------------
// Low-level I/O
#include <mootIO.h>
#include <mootCIO.h>
#include <mootZIO.h>
#include <mootCxxIO.h>
#include <mootBufferIO.h>
#include <mootBinIO.h>

//----------------------------------------------------------------------
// mootToken I/O
#include <mootToken.h>
#include <mootFlavor.h>
#include <mootTokenIO.h>
#include <mootGenericLexer.h>
#include <mootPPLexer.h>
#include <mootTokenLexer.h>
#include <mootRecode.h>
#include <mootExpatParser.h>
#include <mootTokenExpatIO.h>
#include <mootXmlDoc.h>
#include <mootTokenXmlDoc.h>

//----------------------------------------------------------------------
// lexical-class frequency tables
#include <mootClassfreqs.h>
#include <mootClassfreqsLexer.h>
#include <mootClassfreqsParser.h>
#include <mootClassfreqsCompiler.h>

//----------------------------------------------------------------------
// lexical frequency tables
#include <mootLexfreqsCompiler.h>
#include <mootLexfreqs.h>
#include <mootLexfreqsLexer.h>
#include <mootLexfreqsParser.h>

//----------------------------------------------------------------------
// n-gram frequency tables
#include <mootNgramsCompiler.h>
#include <mootNgrams.h>
#include <mootNgramsLexer.h>
#include <mootNgramsParser.h>

//----------------------------------------------------------------------
// Suffix tries (buggy: avoid!)
#include <mootAssocVector.h>
#include <mootTrieVector.h>
#include <mootSuffixTrie.h>

//----------------------------------------------------------------------
// WASTE tokenization framework
#include <wasteTypes.h>
#include <wasteCase.h>
#include <wasteScanner.h>
#include <wasteLexicon.h>
#include <wasteLexer.h>
#include <wasteDecoder.h>
#include <wasteAnnotator.h>
#include <wasteTrainWriter.h>

#endif /* _MOOT_H */
