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

//extern const char* const mootTokenTypeNames[NTokTypes]; //-- BROKEN: swig hates arrays
%inline %{
  const char* const TokenTypeName(mootTokenType typ) { return mootTokenTypeNames[typ]; };
%}

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

//extern const char* const mootTokenFlavorNames[NTokFlavors];  //-- BROKEN: swig hates arrays
%inline %{
  const char* const TokenFlavorName(mootTokenFlavor flav) { return mootTokenFlavorNames[flav]; };
%}


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
 * Class TokenAnalyses (-> std::list<mootToken::Analysis -> TokenAnalysis>)
 */
%template(TokenAnalyses)     std::list<TokenAnalysis>;
%inline %{
  //typedef mootToken::Analyses TokenAnalyses;
  typedef std::list<TokenAnalysis> TokenAnalyses;
%}


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

%rename(type)     Token::tok_type;
%rename(text)     Token::tok_text;
%rename(tag)      Token::tok_besttag;
%rename(analyses) Token::tok_analyses;
%rename(location) Token::tok_location;

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
  void erase(const TokenAnalysis &analysis);
  //void prune(void); //-- Tue, 08 Sep 2009 16:49:17 +0200: BROKEN: *** glibc detected *** /usr/bin/perl: double free or corruption (fasttop): 0x09f74300 ***
  %extend {
    mootTokenFlavor flavor(void) { return tokenFlavor($self->tok_text); };
  }
};

mootTokenFlavor tokenFlavor(const mootTokString &token_text);

/*----------------------------------------------------------------------
 * Class Sentence (-> std::list<mootToken -> Token>)
 */
%template(Sentence) std::list<Token>;
%inline %{
  //typedef mootSentence Sentence;
  typedef std::list<Token> Sentence;
%}
