/*--------------------------------------------------------------------------
 * File: dwdstCHMM.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : 1st-order CHMM tagger/disambiguator
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <FSMRegexCompiler.h>

#include "dwdstCHMM.h"
#include "dwdstTaggerLexer.h"

#include <zlib.h>
#include "dwdstBinIO.h"
#include "dwdstBinStream.h"

using namespace std;
using namespace dwdstBinIO;

/*--------------------------------------------------------------------------
 * clear, freeing dynamic data
 *--------------------------------------------------------------------------*/

void dwdstCHMM::clear(bool wipe_everything)
{
  //-- superclass clear
  dwdstHMM::clear(wipe_everything);

  //-- clear class probabilities
  lcprobs.clear();

  //-- clear cache(s)
  morphcache.clear();
  tmmaps.clear();

  //-- check complete reset
  if (wipe_everything) {
    classids.clear();
    n_classes = 0;
    uclassid = 0;
  }
}

/*--------------------------------------------------------------------------
 * Compilation
 *--------------------------------------------------------------------------*/
bool dwdstCHMM::compile(const dwdstLexfreqs &lexfreqs,
			const dwdstNgrams &ngrams,
			const dwdstTagString &start_tag_str)
{
  //-- sanity check
  if (!morph.can_tag()) {
    carp("dwdstCHMM::compile(): Error: invalid morphological analyzer!\n");
    return false;
  }

  //-- superclass compile
  if (!dwdstHMM::compile(lexfreqs, ngrams, start_tag_str))
    return false;

  //-- generate "unknown" class data
  if (!compile_unknown_lclass())
    return false;

  //-- compilation variables
  ClassID     classid;  //-- current class-id
  TagMorphMap tmm;      //-- morph-map for current token

  //-- First, get set of raw token-strings
  hash_set<dwdstTokString> tokstrs(n_toks);
  for (dwdstLexfreqs::LexfreqStringTable::const_iterator lfti = lexfreqs.lftable.begin();
       lfti != lexfreqs.lftable.end();
       lfti++)
    {
      if (token2type(lfti->first.first) != TokTypeAlpha) continue; //-- ignore special types
      tokstrs.insert(lfti->first.first);
    };

  //-- compile lexical-class IDs and probabilities : for all token-strings (*ti)
  for (hash_set<dwdstTokString>::const_iterator ti = tokstrs.begin(); ti != tokstrs.end(); ti++) {
    //if (tokid==0) continue; //-- ignore "unknown" token [why ?!]
    const dwdstTokString &tokstr = *ti;
    const ProbT         toktotal = (ProbT)lexfreqs.lookup(tokstr);
    TokID                  tokid = token2id(tokstr);
    const LexProbSubTable   &lps = lexprobs[tokid];

    //-- get morph-map (tmm) and lexical class (lc)
    get_token_morphmap(tokstr, tmm);
    LexClass &lc = get_morphmap_class(tmm);

    if (lc.empty()) {
      //-- whoa: it's morph-unknown
      tmm     = morphcache[0];
      classid = uclassid;
      lc      = classids.id2name(uclassid);
    } else {
      //-- get or assign class-id
      classid = classids.nameExists(lc)
	? classids.name2id(lc)
	: classids.insert(lc);
      
      //-- possibly cache morphmap
      if (toktotal > morph_cache_threshhold) morphcache[tokid] = tmm;
    }

    //-- get p(class|tag)-probability subtable
    if (classid >= lcprobs.size()) lcprobs.resize(classid+1);
    LexClassProbSubTable &lcps = lcprobs[classid];
    
    //-- add _ALL_ straight lexical probabilities to class probabilities (restrict later)
    for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++) {
      const TagID tagid    = lpsi->first;
      const ProbT ptok_tag = lpsi->second;
      
      LexClassProbSubTable::iterator lcpsi = lcps.find(tagid);
      if (lcpsi == lcps.end()) {
	lcps[tagid] = ptok_tag;
      } else {
	lcps[tagid] += ptok_tag;
      }
    } //-- for (...lpsi++)
  } //-- for (...lpi++,tokid++)
  
  //-- update n_classes
  n_classes = classids.size();

  //-- cleanup: reset morphology
  morph.ntokens = 0;
  morph.nunknown = 0;
  
  return true;
}


/*--------------------------------------------------------------------------
 * Compilation utlities: "unknown" lexical class data
 *--------------------------------------------------------------------------*/
bool dwdstCHMM::compile_unknown_lclass(void)
{
  TagMorphMap &ummap = morphcache[0];            //-- "unknown" morph-map

  //-- "unknown" class data: morph-map
  get_token_morphmap(tokids.id2name(0), ummap);  //-- try to lookup "unknown" token
  if (ummap.empty()) {
    //-- ARGH: morphology can't handle "@UNKNOWN" convention: must use FSMRegexCompiler
    string regex;
    FSMRegexCompiler regc;
    regc.use_symbol_spec(morph.syms);
    FSM *ufsm = NULL;

    if (lexprobs.size() > 0 && !lexprobs[0].empty()) {
      //-- build regex : just use tags for "unknown" token
      const LexProbSubTable &lps = lexprobs[0];
      for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++) {
	const dwdstTagString &tagstr = tagids.id2name(lpsi->first);
	if (tagstr.empty()) continue;  //-- ignore empty tags
	if (!regex.empty()) regex.push_back('|');
	regex.push_back('(');
	regex.append(tagstr);
	regex.push_back(')');
      }
    } //-- !lexprobs[0].empty()

    if (regex.empty()) {
      //-- build regex: no "unknown" tokens in lexicon: use all tags
      for (TagID tagid = 0; tagid < n_tags; tagid++) {
	const dwdstTagString &tagstr = tagids.id2name(tagid);
	if (tagstr.empty()) continue; //-- ignore empty tags
	if (!regex.empty()) regex.push_back('|');
	regex.push_back('(');
	regex.append(tagstr);
	regex.push_back(')');
      }
    }
    regex.push_back('\n');

    //-- compile regex and get analyses
    ufsm = regc.parse_from_string(regex.c_str());
    MorphAnalysisSet mas;
    if (ufsm && *ufsm) ufsm->fsm_symbol_vectors(mas, false);

    //-- sanity check
    if (!ufsm || !*ufsm || mas.empty()) {
      carp("dwdstCHMM::compile(): could not populate default ambiguity class!");
      if (ufsm) delete ufsm;
      return false;
    }

    //-- populate ummap
    analyses2morphmap(mas, ummap);

    //-- cleanup
    delete ufsm;
  } //-- ummap.empty()

  //-- "unknown" class data: class-id (no probability auto-generation)
  //    can't use zero, otherwise we can't recognize previously unknown
  //    REAL classes at runtime!
  //--  no auto-generation allowed!
  uclassid  = class2classid(get_morphmap_class(ummap), false);

  return true;
}


/*--------------------------------------------------------------------------
 * Top-level tagging interface: stream
 *--------------------------------------------------------------------------*/
bool dwdstCHMM::tag_stream(FILE *in, FILE *out, char *srcname)
{
  dwdstTaggerLexer lexer;

  // -- sanity check
  if (!morph.can_tag()) {
    carp("dwdstCHMM::tag_stream(): cannot tag with invalid morphology!\n");
    return false;
  }

  //-- prepare lexer
  lexer.step_streams(in,out);
  lexer.theLine   = 1;
  lexer.theColumn = 0;

  //-- prepare variables
  int tok;
  tokens.clear();
  tmmaps.clear();
  viterbi_clear();

  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
      switch (tok) {

      case dwdstTaggerLexer::TOKEN:
	tokens.push_back((const char *)lexer.yytext);
	viterbi_step(tokens.back());
        ntokens++;
        break;

      case dwdstTaggerLexer::EOS:
	viterbi_finish();
	tag_print_best_path(out);
	tokens.clear();
	viterbi_clear();
	nsents++;
	break;

      case dwdstTaggerLexer::TAG:
      case dwdstTaggerLexer::EOT:
	/* -- ignore TAGs in the input file */
	break;

      default:
	carp("%s: Error: unknown token '%s' in file '%s' at line %d, column %d\n",
	     "dwdstCHMM::tag_stream()",
	     (srcname ? srcname : "(unknown)"),
	     lexer.yytext,
	     lexer.theLine,
	     lexer.theColumn);
	break;
      }
  }
  return true;
}


/*--------------------------------------------------------------------------
 * Top-level: strings
 *--------------------------------------------------------------------------*/
void dwdstCHMM::tag_strings(int argc, char **argv, FILE *out, char *infilename)
{
  //-- prepare variables
  tokens.clear();
  viterbi_clear();

  for ( ; --argc >= 0; argv++) {
    tokens.push_back((const char *)*argv);
    viterbi_step(tokens.back());
    ntokens++;
  }
  viterbi_finish();
  tag_print_best_path(out);
  nsents++;

  //-- cleanup
  tokens.clear();
  viterbi_clear();
}


/*--------------------------------------------------------------------------
 * Binary I/O: save
 *--------------------------------------------------------------------------*/

const HeaderInfo::VersionT BINCOMPAT_MIN_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_MIN_REV = 4;

const HeaderInfo::VersionT BINCOMPAT_VER = 1;
const HeaderInfo::VersionT BINCOMPAT_REV = 4;

bool dwdstCHMM::save(const char *filename, int compression_level)
{
  //-- sanity checks
  if (compression_level != Z_DEFAULT_COMPRESSION
      && (compression_level > Z_BEST_COMPRESSION || compression_level < Z_NO_COMPRESSION))
    {
      carp("dwdstHMM::save(): bad compression level %d defaults to %d\n",
	   compression_level, Z_DEFAULT_COMPRESSION);
      compression_level = Z_DEFAULT_COMPRESSION;
    }

  //-- open file
  gzFile gzf = gzopen(filename, "wb");
  if (!gzf) {
    carp("dwdstHMM::save(): open failed for file '%s': %s",
	 filename, gzerror(gzf, &errno));
    return false;
  }
  gzsetparams(gzf, compression_level, Z_DEFAULT_STRATEGY);

  //-- setup stream-hack
  dwdstBinStream::ozBinStream ozs(gzf);

  //-- and save
  bool rc = save(ozs, filename);
  ozs.close();
  return rc;
}

bool dwdstCHMM::save(dwdstBinStream::oBinStream &obs, const char *filename=NULL)
{
  HeaderInfo hi(string("dwdstCHMM"),
		BINCOMPAT_VER,     BINCOMPAT_REV,
		BINCOMPAT_MIN_VER, BINCOMPAT_MIN_REV,
		0);
  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<char *>     cmt_item;
  char comment[512];
  sprintf(comment, "\ndwdstCHMM Version %u.%u\n", BINCOMPAT_VER, BINCOMPAT_REV);

  //-- get checksum
  size_t crc =
    start_tagid + n_tags + n_toks + n_classes + uclassid + morph.checksum();
  if (! (hi_item.save(obs, hi)
	 && size_item.save(obs, crc)
	 && cmt_item.save(obs, comment)
	 ))
    {
      carp("dwdstCHMM::save(): could not save header%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  return _bindump(obs, filename);
}


bool dwdstCHMM::_bindump(dwdstBinStream::oBinStream &obs, const char *filename=NULL)
{
  if (!dwdstHMM::_bindump(obs,filename))
    return false;

  //-- variables
  Item<dwdstMorph>        morph_item;
  Item<size_t>            size_item;
  Item<ClassIDTable>      classids_item;
  Item<LexClassProbTable> lcprobs_item;
  Item<TokMorphCache>     morphcache_item;
  Item<ClassID>           classid_item;
  Item<ProbT>             probt_item;

  if (! (morph_item.save(obs, morph)
	 && probt_item.save(obs, morph_cache_threshhold)
	 && size_item.save(obs, n_classes)
	 && classids_item.save(obs, classids)
	 && lcprobs_item.save(obs, lcprobs)
	 && morphcache_item.save(obs, morphcache)
	 && classid_item.save(obs, uclassid)
	 ))
    {
      carp("dwdstCHMM::save(): could not save data%s%s\n",
	   (filename ? " to file " : ""), (filename ? filename : ""));
      return false;
    }

  return true;
}


/*--------------------------------------------------------------------------
 * Binary I/O: load
 *--------------------------------------------------------------------------*/

bool dwdstCHMM::load(const char *filename=NULL)
{
  //-- setup gzFile
  gzFile gzs = gzopen(filename, "rb");
  if (!gzs) {
    carp("dwdstHMM::load(): could not open file '%s' for read: %s",
	 filename, strerror(errno));
    return false;
  }

  //-- setup stream-hack
  dwdstBinStream::izBinStream izs(gzs);

  bool rc = load(izs, filename);
  izs.close();
  return rc;
}

bool dwdstCHMM::load(dwdstBinStream::iBinStream &ibs, const char *filename=NULL)
{
  clear(true); //-- make sure the object is empty

  HeaderInfo hi, hi_magic("dwdstCHMM");
  size_t     crc;

  Item<HeaderInfo> hi_item;
  Item<size_t>     size_item;
  Item<string>     cmt_item;
  string comment;

  //-- load headers
  if (! (hi_item.load(ibs, hi)
	 && size_item.load(ibs, crc)
	 && cmt_item.load(ibs, comment)
	 ))
    {
      carp("dwdstCHMM::load(): could not load header%s%s",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }
  else if (hi.magic != hi_magic.magic)
    {
      carp("dwdstCHMM::load(): bad magic 0x%x%s%s",
	   hi.magic,
	   (filename ? " in file " : ""), (filename ? filename : ""));
      return false;
    }
  else if (hi.version < BINCOMPAT_MIN_VER
	   || (hi.version == BINCOMPAT_MIN_VER && hi.revision < BINCOMPAT_MIN_REV)
	   || BINCOMPAT_VER < hi.minver
	   || (BINCOMPAT_VER == hi.minver && BINCOMPAT_MIN_REV < hi.minrev))
    {
      carp("dwdstCHMM::load(): incompatible file version %u.%u%s%s",
	   hi.version, hi.revision,
	   (filename ? " in file " : ""), (filename ? filename : ""));
    }

  if(!_binload(ibs, filename))
    return false;

  if (crc != (start_tagid + n_tags + n_toks + n_classes + uclassid + morph.checksum())) {
    carp("dwdstCHMM::load(): checksum failed%s%s",
	 (filename ? " for file " : ""), (filename ? filename : ""));
  }

  viterbi_clear(); //-- initialize Viterbi table
  return true;
}


bool dwdstCHMM::_binload(dwdstBinStream::iBinStream &ibs, const char *filename=NULL)
{
  //-- superclass load
  if (!dwdstHMM::_binload(ibs, filename))
    return false;

  //-- variables
  Item<dwdstMorph>        morph_item;
  Item<size_t>            size_item;
  Item<ClassIDTable>      classids_item;
  Item<LexClassProbTable> lcprobs_item;
  Item<TokMorphCache>     morphcache_item;
  Item<ClassID>           classid_item;
  Item<ProbT>             probt_item;

  if (! (morph_item.load(ibs, morph)
	 && probt_item.load(ibs, morph_cache_threshhold)
	 && size_item.load(ibs, n_classes)
	 && classids_item.load(ibs, classids)
	 && lcprobs_item.load(ibs, lcprobs)
	 && morphcache_item.load(ibs, morphcache)
	 && classid_item.load(ibs, uclassid)
	 ))
    {
      carp("dwdstCHMM::load(): could not load data%s%s\n",
	   (filename ? " from file " : ""), (filename ? filename : ""));
      return false;
    }

  if (morph.syms_filename != "" && !morph.load_morph_symbols()) {
    carp("dwdstCHMM::load(): could not load morphological symbols from '%s'\n",
	 morph.syms_filename.c_str());
    return false;
  }

  if (morph.mfst_filename != "" && !morph.load_morph_fst()) {
    carp("dwdstCHMM::load(): could not load morphological FST from '%s'\n",
	 morph.mfst_filename.c_str());
    return false;
  }

  if (morph.xfst_filename != "" && !morph.load_tagx_fst()) {
    carp("dwdstCHMM::load(): could not load tag-extraction FST from '%s'\n",
	 morph.xfst_filename.c_str());
    return false;
  }
  
  return true;
}


/*--------------------------------------------------------------------------
 * Debug / Dump
 *--------------------------------------------------------------------------*/
void dwdstCHMM::txtdump(FILE *file)
{
  fprintf(file, "%%%% dwdstCHMM text dump\n");

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Smoothing Constants\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%start_tag\tID=%u(\"%s\")\n", start_tagid, tagids.id2name(start_tagid).c_str());
  fprintf(file, "%%nglambda1\t%g\n", nglambda1);
  fprintf(file, "%%nglambda2\t%g\n", nglambda2);
#ifdef DWDST_USE_TRIGRAMS
  fprintf(file, "%%nglambda3\t%g\n", nglambda3);
#endif
  fprintf(file, "%%wlambda1\t%g\n", wlambda1);
  fprintf(file, "%%wlambda2\t%g\n", wlambda2);
  //fprintf(file, "%%wlambdaC\t%g\n", wlambdaC);

  //-- common variables
  TokID tokid;
  ClassID classid;
  TagID tagid, ptagid;
  ProbT prob;

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Lexical Probabilities\n");
  fprintf(file, "%%%% TokID(\"TokStr\")\tTagID(\"TagStr\")\tp(TokID|TagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  LexProbTable::const_iterator lpi;
  for (lpi = lexprobs.begin() , tokid = 0;
       lpi != lexprobs.end()  ;
       lpi++                  , tokid++)
    {
      for (LexProbSubTable::const_iterator lpsi = lpi->begin(); lpsi != lpi->end(); lpsi++)
	{
	  TagID tagid = lpsi->first;
	  ProbT prob  = lpsi->second;
	  fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%g\n",
		  tokid, tokids.id2name(tokid).c_str(),
		  tagid, tagids.id2name(tagid).c_str(),
		  prob);
	}
    }

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Lexical-Class Probabilities\n");
  fprintf(file, "%%%% ClassID(\"ClassStr\")\tTagID(\"TagStr\")\tp(ClassID|TagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  LexClassProbTable::const_iterator lcpi;
  for (lcpi = lcprobs.begin() , classid = 0;
       lcpi != lcprobs.end()  ;
       lcpi++                , classid++)
    {
      string classString("{");
      const LexClass &lc = classids.id2name(classid);
      for (LexClass::const_iterator lci = lc.begin(); lci != lc.end(); lci++) {
	if (lci != lc.begin()) classString.push_back(',');
	classString.append(tagids.id2name(*lci));
      }
      classString.append("}");

      for (LexClassProbSubTable::const_iterator lcpsi = lcpi->begin(); lcpsi != lcpi->end(); lcpsi++)
	{
	  TagID tagid = lcpsi->first;
	  ProbT prob  = lcpsi->second;
	  fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%g\n",
		  classid, classString.c_str(),
		  tagid, tagids.id2name(tagid).c_str(),
		  prob);
	}
    }


  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Unigram Probabilities\n");
  fprintf(file, "%%%% TagID(\"TagStr\")\tp(TagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  if (ngprobs1 != NULL) {
    for (tagid = 0; tagid < n_tags; tagid++) {
      prob = ngprobs1[tagid];
      if (prob == 0) continue;
      fprintf(file, "%u(\"%s\")\t%g\n",
	      tagid, tagids.id2name(tagid).c_str(),
	      prob);
    }
  }
  else {
    fprintf(file, "%% (NULL)\n");
  }

  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Bigram Probabilities\n");
  fprintf(file, "%%%% PrevTagID(\"PrevTagStr\")\tTagID(\"TagStr\")\tp(TagID|PrevTagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  if (ngprobs2 != NULL) {
    for (ptagid = 0; ptagid < n_tags; ptagid++) {
      for (tagid = 0; tagid < n_tags; tagid++) {
	prob = ngprobs2[(n_tags*ptagid)+tagid];
	if (prob == 0) continue;
	fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%g\n",
		ptagid,  tagids.id2name(ptagid).c_str(),
		tagid, tagids.id2name(tagid).c_str(),
		prob);
      }
    }
  } else {
    fprintf(file, "%% (NULL)\n");
  }

#ifdef DWDST_USE_TRIGRAMS
  fprintf(file, "\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  fprintf(file, "%%%% Trigram Probabilities\n");
  fprintf(file, "%%%% PrevPrevPrevTagID(\"PrevPrevTagStr\")\tPrevTagID(\"PrevTagStr\")\tTagID(\"TagStr\")\tp(TagID|PrevPrevTagID,PrevTagID)\n");
  fprintf(file, "%%%%-----------------------------------------------------\n");
  TagID pptagid;
  if (!ngprobs3.empty()) {
    for (TrigramProbTable::const_iterator tgti = ngprobs3.begin();
	 tgti != ngprobs3.end();
	 tgti++)
      {
	pptagid = tgti->first.tag1;
	ptagid  = tgti->first.tag2;
	tagid   = tgti->first.tag3;
	prob    = tgti->second;
	if (prob == 0) continue;
	fprintf(file, "%u(\"%s\")\t%u(\"%s\")\t%u(\"%s\")\t%g\n",
		pptagid,  tagids.id2name(pptagid).c_str(),
		ptagid,  tagids.id2name(ptagid).c_str(),
		tagid, tagids.id2name(tagid).c_str(),
		prob);
      }
  }
  else {
    fprintf(file, "%% (empty)\n");
  }
#endif /* DWDST_USE_TRIGRAMS */

  fprintf(file, "\n");
}



/*--------------------------------------------------------------------------
 * Error reporting
 *--------------------------------------------------------------------------*/

//void dwdstCHMM::carp(char *fmt, ...) //-- inherited
