/* -*- Mode: C++ -*- */
/*
 * File: Token.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: tokens
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS

//-- Token types
typedef std::string mootTagString;
typedef std::string mootTokString;

/*----------------------------------------------------------------------
 * Basic Token Types
 *----------------------------------------------------------------------*/
typedef enum {
  /* Output token-types */
  TokTypeUnknown = moot::TokTypeUnknown,   /**< we dunno what it is -- could be anything  */
  TokTypeVanilla = moot::TokTypeVanilla,   /**< plain "vanilla" token (+/-besttag,+/-analyses) */
  TokTypeLibXML = moot::TokTypeLibXML,    /**< plain XML token; much like 'Vanilla' */
  TokTypeXMLRaw = moot::TokTypeXMLRaw,    /**< Raw XML text (for lossless XML I/O) */
  TokTypeComment = moot::TokTypeComment,   /**< a comment, should be ignored by processing routines */
  TokTypeEOS = moot::TokTypeEOS,       /**< end-of-sentence */
  TokTypeEOF = moot::TokTypeEOF,       /**< end-of-file */
  TokTypeUser = moot::TokTypeUser,      /**< user-defined token type: use in conjunction with 'user_data' */
  NTokTypes = moot::NTokTypes         /**< number of token-types (not a type itself) */
} mootTokenType;

extern const char* mootTokenTypeNames[NTokTypes];

typedef enum {
  TokFlavorAlpha,      /**< (Mostly) alphabetic token: "foo", "bar", "foo2bar" */
  TokFlavorCard,       /**< \p \@CARD: Digits-only: "42" */
  TokFlavorCardPunct,  /**< \p \@CARDPUNCT: Digits single-char punctuation suffix: "42." */
  TokFlavorCardSuffix, /**< \p \@CARDSUFFIX: Digits with (almost any) suffix: "42nd" */
  TokFlavorCardSeps,   /**< \p \@CARDSEPS: Digits with interpunctuation: "420.24/7" */
  TokFlavorUnknown,    /**< \p \@UNKNOWN: Special "Unknown" token-type */
  //TokFlavorSpecial,    /* A literal "\@CARD", "\@CARDPUNCT", etc. */
  NTokFlavors          /**< Not really a token-type */
} mootTokenFlavor;

extern const char *mootTokenFlavorNames[NTokFlavors];

/*----------------------------------------------------------------------
 * Class TokenAnalysis
 */
%{
typedef mootToken::Analysis TokenAnalysis;
%}

//-- TokenAnalysis -> mootToken::Analysis
class TokenAnalysis {
public:
    mootTagString tag;
    mootTagString details;
    ProbT prob;
public:
  TokenAnalysis(const mootTagString &my_tag="", const mootTagString &my_details="", const ProbT my_prob=0);
  ~TokenAnalysis();
  void clear(void);
  bool empty(void) const;
  %extend {
    bool equal(const TokenAnalysis &a2) { return (*$self) == a2; };
  }
};

/*----------------------------------------------------------------------
 * Class TokenAnalyses
 */
%{
//typedef std::list<mootToken::Analysis> TokenAnalyses;
typedef mootToken::Analyses TokenAnalyses;
%}

class TokenAnalyses {
public:
  TokenAnalyses(void);
  ~TokenAnalyses(void);
  void clear(void);
  //
  size_t size(void);
  bool empty(void);
  //
  TokenAnalysis &front(void);
  TokenAnalysis &back(void);
  %extend {
    TokenAnalysis *nth(int n) { //-- BROKEN!
      if (n<0) {
	++n;
	TokenAnalyses::reverse_iterator ai = $self->rbegin();
	for (; ai != $self->rend(); ai++) ;
	return &(*ai);
      } else {
	TokenAnalyses::iterator ai = $self->begin();
	for (; ai != $self->end(); ai++) ;
	return &(*ai);
      }
    };
  }
  //
  void push_front(const TokenAnalysis &a);
  void push_back(const TokenAnalysis &a);
  void pop_front(void);
  void pop_back(void);
};

/*----------------------------------------------------------------------
 * Class TokenLocation
 */
%{
typedef mootToken::Location TokenLocation;
%}

typedef unsigned int OffsetT;

class TokenLocation {
public:
  OffsetT offset;
  OffsetT length;
public:
  TokenLocation(const OffsetT my_offset=0, const OffsetT my_length=0);
  ~TokenLocation(void);
  void clear(void);
};

/*----------------------------------------------------------------------
 * Class Token
 */
%{
typedef mootToken Token;
%}

class Token {
public:
  mootTokenType tok_type;
  mootTokString tok_text;
  mootTagString tok_besttag;
  TokenAnalyses tok_analyses;
  TokenLocation tok_location;

public:
  Token(mootTokenType type=TokTypeVanilla);
  Token(const mootTokString &text, mootTokenType type=TokTypeVanilla);
  Token(const mootTokString &text, const TokenAnalyses &analyses);
  Token(const mootTokString &text, const TokenAnalyses &analyses, const mootTagString &besttag);
  ~Token(void);
  void clear(void);

  //-- analysis manipulation
  void insert(const TokenAnalysis &analysis);
  void insert(const char *tag, const char *details);
  void prune(void);
  %extend {
    mootTokenFlavor flavor(void) { return tokenFlavor($self->tok_text); };
  }
};

mootTokenFlavor tokenFlavor(const mootTokString &token_text);
