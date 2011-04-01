/* -*- Mode: C++ -*- */
/*
 * File: Lexfreqs.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: mootLexfreqs.h
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS

//==============================================================================
%inline %{
  typedef ProbT LexfreqCount;
%}

//==============================================================================
%{ typedef mootLexfreqs Lexfreqs; %}
class Lexfreqs {
public:
  LexfreqCount       n_tokens;   /**< total number of tokens counted */

public:
  Lexfreqs(void);
  ~Lexfreqs(void);
  void clear(void);
  void compute_specials(void);
  size_t n_pairs(void);

  void add_count(const mootTokString &text, const mootTagString &tag, const LexfreqCount count);

  LexfreqCount taglookup(const mootTagString &tag) const;
  %extend {
    LexfreqCount tlookup(const mootTagString &tag)
    { return $self->taglookup(tag); }

    LexfreqCount wlookup(const mootTokString &tok)
    {
      mootLexfreqs::LexfreqTokTable::const_iterator wti = $self->lftable.find(tok);
      if (wti == $self->lftable.end()) return 0;
      return wti->second.count;
    }

    LexfreqCount wtlookup(const mootTokString &tok, const mootTagString &tag)
    {
      mootLexfreqs::LexfreqTokTable::const_iterator wti = $self->lftable.find(tok);
      if (wti == $self->lftable.end()) return 0;
      mootLexfreqs::LexfreqSubtable::const_iterator ti = wti->second.freqs.find(tag);
      if (ti == wti->second.freqs.end()) return 0;
      return ti->second;
    }
  };
  
  bool load(const char *filename);
  bool save(const char *filename);
};
