#include "MootPerl.h"

#undef VERSION
#include <mootConfig.h>

/*======================================================================
 * Constants
 */
const char *moot_version_string = PACKAGE_VERSION;

/*======================================================================
 * Conversions
 */

/*--------------------------------------------------------------*/
HV* token2hv(const mootToken *tok)
{
  HV *hv = newHV();

  //-- token: analyses
  AV *anav = newAV();
  for (mootToken::Analyses::const_iterator ai=tok->tok_analyses.begin();
       ai != tok->tok_analyses.end();
       ai++)
    {
      HV *anhv = newHV();
      hv_stores(anhv, "tag",     newSVpvn(ai->tag.data(), ai->tag.size()));
      hv_stores(anhv, "details", newSVpvn(ai->details.data(), ai->details.size()));
      hv_stores(anhv, "cost",    newSVnv(ai->prob));
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
mootToken *hv2token(HV *hv, mootToken *tok)
{
  SV **svpp, **avrvpp;
  if (!tok) tok = new mootToken();
  tok->tok_data = (void*)hv;

  if ((svpp=hv_fetchs(hv,"type",0))) tok->tok_type=(mootTokenType)SvUV(*svpp);
  if ((svpp=hv_fetchs(hv,"text",0))) tok->tok_text=SvPV_nolen(*svpp);
  if ((svpp=hv_fetchs(hv,"tag",0)))  tok->tok_besttag=SvPV_nolen(*svpp);

  if ((avrvpp=hv_fetchs(hv,"analyses",0))) {
    AV *anav = (AV*)SvRV(*avrvpp);
    I32 avlen = av_len(anav);
    for (I32 avi=0; avi <= avlen; avi++) {
      SV **anhvrv = av_fetch(anav,avi,0);
      if (!anhvrv || !*anhvrv) continue;
      HV *anhv = (HV*)SvRV(*anhvrv);
      mootToken::Analysis an;
      if ((svpp=hv_fetchs(anhv,"tag",0))) an.tag=SvPV_nolen(*svpp);
      if ((svpp=hv_fetchs(anhv,"details",0))) an.details=SvPV_nolen(*svpp);
      if ((svpp=hv_fetchs(anhv,"cost",0))) an.prob=SvNV(*svpp);
      tok->tok_analyses.push_back(an);
    }
  }

  return tok;
}

/*--------------------------------------------------------------*/
AV* sentence2av(const mootSentence *s)
{
  AV *sav = newAV();
  for (mootSentence::const_iterator si=s->begin(); si != s->end(); si++) {
    HV *tokhv = token2hv(&(*si));
    av_push(sav, newRV_inc((SV*)tokhv));    
  }
  sv_2mortal((SV*)sav);
  return sav;
}

/*--------------------------------------------------------------*/
mootSentence *av2sentence(AV *sav, mootSentence *s)
{
  if (!s) s = new mootSentence();

  I32 slen = av_len(sav);
  for (I32 si=0; si <= slen; si++) {
    SV **tokhvrv = av_fetch(sav,si,0);
    if (!tokhvrv || !*tokhvrv) continue;
    s->push_back(mootToken());
    hv2token((HV*)SvRV(*tokhvrv), &(s->back()));
  }

  return s;
}

/*======================================================================
 * Conversions: in-place
 */

/*--------------------------------------------------------------*/
void sentence2tokdata(mootSentence *s)
{
  for (mootSentence::iterator si=s->begin(); si != s->end(); si++) {
    HV *tokhv = (HV*)si->tok_data;
    hv_stores(tokhv, "tag", newSVpvn(si->tok_besttag.data(), si->tok_besttag.size()));
  }
}
