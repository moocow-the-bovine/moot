#include "MootPerl.h"

#undef VERSION
#include <mootConfig.h>

/*======================================================================
 * macros
 */
#if defined(SvPVutf8_nolen)
# define sv2stdstring(sv,str,utf8) \
  str = ((utf8) ? SvPVutf8_nolen(sv) : SvPV_nolen(sv))
#else
  static inline
  void sv2stdstring(SV *sv, std::string &str, U32 utf8)
  {
    STRLEN len;
    char *pv = sv_2pvutf8(sv, &len);
    str.assign(pv,len);
  }
#endif /* defined(SvPVutf8_nolen) */

#if defined(newSVpvn_utf8)
# define stdstring2sv(str,utf8) \
  newSVpvn_utf8((str).data(), (str).size(), utf8)
#else
 static inline
 SV* stdstring2sv(const std::string &str, U32 utf8)
 {
   SV *sv = newSVpvn(str.data(), str.size());
   SvUTF8_on(sv);
   return sv;
 }
#endif /* defined(newSVpvn_utf8) */

/*======================================================================
 * Constants
 */
const char *moot_version_string = PACKAGE_VERSION;

/*======================================================================
 * Conversions
 */

/*--------------------------------------------------------------*/
HV* token2hv(const mootToken *tok, U32 utf8)
{
  HV *hv = newHV();

  //-- token: analyses
  AV *anav = newAV();
  for (mootToken::Analyses::const_iterator ai=tok->tok_analyses.begin();
       ai != tok->tok_analyses.end();
       ai++)
    {
      HV *anhv = newHV();
      hv_stores(anhv, "tag",     stdstring2sv(ai->tag,utf8));
      hv_stores(anhv, "details", stdstring2sv(ai->details,utf8));
      hv_stores(anhv, "prob",    newSVnv(ai->prob));
      //sv_2mortal((SV*)anhv); //-- combine with newRV_inc() in av_push()?
      av_push(anav, newRV_noinc((SV*)anhv));
    }
  //sv_2mortal((SV*)anav); //-- use in combination with newRV_inc() in hv_stores()?

  //-- token: hash
  hv_stores(hv, "type",     newSVuv(tok->tok_type));
  hv_stores(hv, "text",     newSVpvn(tok->tok_text.data(), tok->tok_text.size()));
  hv_stores(hv, "tag",      newSVpvn(tok->tok_besttag.data(), tok->tok_besttag.size()));
  hv_stores(hv, "analyses", newRV_noinc((SV*)anav));

  sv_2mortal((SV*)hv);
  return hv;
}


/*--------------------------------------------------------------*/
mootToken *hv2token(HV *hv, mootToken *tok, U32 utf8)
{
  SV **svpp, **avrvpp;
  if (!tok) tok = new mootToken();
  tok->tok_data = (void*)hv;

  if ((svpp=hv_fetchs(hv,"type",0))) tok->tok_type=(mootTokenType)SvUV(*svpp);
  if ((svpp=hv_fetchs(hv,"text",0))) sv2stdstring(*svpp,tok->tok_text,utf8);
  if ((svpp=hv_fetchs(hv,"tag",0)))  sv2stdstring(*svpp,tok->tok_besttag,utf8);

  if ((avrvpp=hv_fetchs(hv,"analyses",0))) {
    AV *anav = (AV*)SvRV(*avrvpp);
    I32 avlen = av_len(anav);
    for (I32 avi=0; avi <= avlen; avi++) {
      SV **anhvrv = av_fetch(anav,avi,0);
      if (!anhvrv || !*anhvrv) continue;
      HV *anhv = (HV*)SvRV(*anhvrv);
      mootToken::Analysis an;
      if ((svpp=hv_fetchs(anhv,"tag",0))) sv2stdstring(*svpp,an.tag,utf8);
      if ((svpp=hv_fetchs(anhv,"details",0))) sv2stdstring(*svpp,an.details,utf8);
      if ((svpp=hv_fetchs(anhv,"prob",0))) an.prob=SvNV(*svpp);
      tok->tok_analyses.push_back(an);
    }
  }

  return tok;
}

/*--------------------------------------------------------------*/
AV* sentence2av(const mootSentence *s, U32 utf8)
{
  AV *sav = newAV();
  for (mootSentence::const_iterator si=s->begin(); si != s->end(); si++) {
    HV *tokhv = token2hv(&(*si), utf8);
    av_push(sav, newRV_inc((SV*)tokhv));    
  }
  sv_2mortal((SV*)sav);
  return sav;
}

/*--------------------------------------------------------------*/
mootSentence *av2sentence(AV *sav, mootSentence *s, U32 utf8)
{
  if (!s) s = new mootSentence();

  I32 slen = av_len(sav);
  for (I32 si=0; si <= slen; si++) {
    SV **tokhvrv = av_fetch(sav,si,0);
    if (!tokhvrv || !*tokhvrv) continue;
    s->push_back(mootToken());
    hv2token((HV*)SvRV(*tokhvrv), &(s->back()), utf8);
  }

  return s;
}

/*--------------------------------------------------------------*/
mootTagSet *av2tagset(AV *tsav, mootTagSet *tagset, U32 utf8)
{
  if (!tagset) tagset = new mootTagSet();

  I32 tslen = av_len(tsav);
  for (I32 si=0; si <= tslen; si++) {
    SV **svpp = av_fetch(tsav,si,0);
    if (!svpp || !*svpp) continue;
    std::string tagstr;
    sv2stdstring(*svpp, tagstr, utf8);
    tagset->insert(tagstr);
  }

  return tagset;
}

/*======================================================================
 * Conversions: in-place
 */

/*--------------------------------------------------------------*/
void sentence2tokdata(mootSentence *s, U32 utf8)
{
  for (mootSentence::iterator si=s->begin(); si != s->end(); si++) {
    HV *tokhv = (HV*)si->tok_data;
    hv_stores(tokhv, "tag", stdstring2sv(si->tok_besttag,utf8));
  }
}

/*======================================================================
 * mootPerlInputFH
 */

//--------------------------------------------------------------
mootPerlInputFH::mootPerlInputFH(SV *sv) 
  : ioref(sv),
    io(NULL)
{
  if (ioref) {
    SvREFCNT_inc(ioref);
    io = IoIFP( sv_2io(ioref) );
  }
}

//--------------------------------------------------------------
mootPerlInputFH::~mootPerlInputFH(void)
{
  if (ioref) {
    SvREFCNT_dec(ioref);
  }
}

//--------------------------------------------------------------
bool mootPerlInputFH::eof(void)
{
  bool rc = (io == NULL || PerlIO_eof(io));
  if (!rc) {
    int c = PerlIO_getc(io);
    if (c==EOF) rc = true;
    else PerlIO_ungetc(io,c);
  }
  return rc;
}

//--------------------------------------------------------------
bool mootPerlInputFH::valid(void)
{
  bool rc = (io != NULL && !PerlIO_error(io));
  return rc;
}

//--------------------------------------------------------------
int mootPerlInputFH::getbyte(void)
{
  if (eof() || !valid()) return EOF;
  return PerlIO_getc(io);
}

//--------------------------------------------------------------
mootio::ByteCount mootPerlInputFH::read(char *buf, size_t n)
{
  if (eof() || !valid())
    return -1;
  return (mootio::ByteCount)PerlIO_read(io, buf, n);
};


/*======================================================================
 * mootPerlInputBuf
 */

//--------------------------------------------------------------
mootPerlInputBuf::mootPerlInputBuf(SV *svbuf) 
  : micbuffer(NULL,0),
    sv(svbuf)
{
  if (sv) {
    STRLEN len;
    SvREFCNT_inc(sv);
    const char *data = SvPVutf8(svbuf, len);
    this->assign(data,len);
  }
}

//--------------------------------------------------------------
mootPerlInputBuf::~mootPerlInputBuf(void)
{
  if (sv) {
    SvREFCNT_dec(sv);
  }
}

