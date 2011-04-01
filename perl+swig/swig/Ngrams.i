/* -*- Mode: C++ -*- */
/*
 * File: Ngrams.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: mootNgrams.h
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS

//==============================================================================
%inline %{
  typedef ProbT NgramCount;
%}

//==============================================================================
%{ typedef mootNgrams Ngrams; %}
class Ngrams {
public:
  NgramCount ugtotal;

public:
  Ngrams(void);
  ~Ngrams(void);
  void clear(void);

  size_t n_unigrams(void) const;
  size_t n_bigrams(void);
  size_t n_trigrams(void);

  void add_count(const mootTagString &tag, const NgramCount count);
  void add_count(const mootTagString &tag1, const mootTagString &tag2, const NgramCount count);
  void add_count(const mootTagString &tag1, const mootTagString &tag2, const mootTagString &tag3, const NgramCount count);

  %extend {
    void add_counts(const mootTagString &tag1, const NgramCount count)
    { $self->add_counts(mootNgrams::Ngram(tag1),count); };

    void add_counts(const mootTagString &tag1, const mootTagString &tag2, const NgramCount count)
    { $self->add_counts(mootNgrams::Ngram(tag1,tag2),count); };

    void add_counts(const mootTagString &tag1, const mootTagString &tag2, const mootTagString &tag3, const NgramCount count)
    { $self->add_counts(mootNgrams::Ngram(tag1,tag2,tag3),count); };
  };

  NgramCount lookup(const mootTagString &tag) const;
  NgramCount lookup(const mootTagString &tag1, const mootTagString &tag2) const;
  NgramCount lookup(const mootTagString &tag1, const mootTagString &tag2, const mootTagString &tag3) const;

  bool load(const char *filename);
  bool save(const char *filename, bool compact=false);
};
