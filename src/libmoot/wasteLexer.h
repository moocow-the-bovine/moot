/* -*- Mode: C++ -*- */

/**
 *
 * \file wasteLexer.h
 *
 * \brief Declarations of the mid level lexer for the waste tokenizer.
 *
 *
 *
 * \author kmw
 *
 * \date 2013
 * 
 */

#ifndef _WASTE_LEXER_H
#define _WASTE_LEXER_H

#include <mootTokenIO.h>
#include <mootSTLHacks.h>
#include <wasteTypes.h>

#include <vector>
#include <string>

namespace moot
{
  class wasteLexicon
  {
    public:
      typedef moot_STL_NAMESPACE::hash_set<std::string>  Lexicon;
      Lexicon lex;

      wasteLexicon();
      ~wasteLexicon();

      void load(mistream *mis);
      void load(const char *filename);
      void clear();
      bool lookup(const std::string &word);
  };

  class wasteLexer : public TokenReader
  {
    public:
      /*--------------------------------------------------------------------
       * wasteLexer: Types
       */
      /*------------------------------------------------------------*/
      /** Multi-dimensional vector for constant access on feature bundles */
      typedef std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<std::string> > > > > > wasteTagset;
      
      /** Possible states of the dehyphenation procedure */
      enum wasteLexer_state
      {
        ls_flush   = 0x01,
        ls_hyph    = 0x02,
        ls_head    = 0x04,
        ls_tail    = 0x08,
        ls_nl      = 0x10,
        ls_exclude = 0x20,
      };

      /** Token feature indexes */
      enum cls
      {
        stop     = 0,
        rom      = 1,
        alpha    = 2,
        num      = 3,
        dot      = 4,
        comma    = 5,
        colon    = 6,
        scolon   = 7,
        eos      = 8,
        lbr      = 9,
        rbr      = 10,
        hyphen   = 11,
        plus     = 12,
        slash    = 13,
        quote    = 14,
        apos     = 15,
        sc       = 16,
        other    = 17,
        n_cls    = 18
      };

      enum cas
      {
        non      = 0,
        lo       = 1,
        up       = 2,
        cap      = 3,
        n_cas    = 4
      };

      enum binary
      {
        uk       = 0,
        kn       = 1,
        n_binary = 2
      };

      enum len
      {
        le_null  = 0,
        le_one   = 1,
        le_three = 2,
        le_five  = 3,
        longer   = 4,
        n_len    = 5
      };

      /*--------------------------------------------------------------------
       * wasteLexer: Data
       */
      /*------------------------------------------------------------*/
      /** \name Low-level data */
      //@{
      TokenReader    *scanner;          /**< Input source */
      mootToken       wl_token;         /**< Local token */
      mootSentence    wl_sentence;      /**< Local sentence */
      wasteTagset     wl_tagset;        /**< Token feature bundles */
      //@}

      /** \name Lexica */
      //@{
      wasteLexicon   wl_stopwords;      /**< List of stopwords */
      wasteLexicon   wl_abbrevs;        /**< List of abbreviations */
      wasteLexicon   wl_conjunctions;   /**< List of conjunctions (for dehyphenating) */
      //@}

      /*--------------------------------------------------------------------
       * wasteLexer: Methods
       */
      /*------------------------------------------------------------*/
      /** \name Constructors etc. */
      //@{
      /** Default constructor */
      wasteLexer(int fmt=tiofUnknown, const std::string &myname="moot::wasteLexer");

      /** Destructor */
      virtual ~wasteLexer();
          
      //@}

      /*------------------------------------------------------------
       * TokenReader: Input Selection
       */
      /** \name Input Selection */
      //@{

      /** Select input from a mootio::mistream pointer. */
      virtual void from_mstream(mootio::mistream *mistreamp);

      /**
       * Finish input from currently selected source & perform any required
       * cleanup operations.
       */
      virtual void close(void);
      //@}

      /*------------------------------------------------------------
       * TokenReader: Token-Level Access
       */
      /** \name Token-Level Access */
      //@{
      /**
       * Get the next token from the buffer.
       * On completion, current token (if any) is in *tr_token.
       */
      virtual mootTokenType get_token(void);

      /**
       * Buffer the remaining tokens from the currently selected input
       * stream as a mootSentence.
       * On completion, buffered tokens (if any) are in *tr_sentence.
       */
      virtual mootTokenType get_sentence(void);
      //@}

      /*------------------------------------------------------------
       * TokenReader: Diagnostics
       */
      /** \name Diagnostics */
      //@{

      /** Get current line number. Descendants may override this method. */
      virtual size_t line_number(void) { return scanner ? scanner->line_number() : 0; };

      /** Set current line number. Descendants may override this method. */
      virtual size_t line_number(size_t n) { return scanner ? scanner->line_number(n) : 0; };

      /** Get current column number. Descendants may override this method. */
      virtual size_t column_number(void) { return scanner ? scanner->column_number() : 0; };

      /** Set current column number. Descendants may override this method. */
      virtual size_t column_number(size_t n) { return scanner ? scanner->column_number(n) : 0; };

      /** Get current byte number. Descendants may override this method. */
      virtual mootio::ByteOffset byte_number(void) { return scanner ? scanner->byte_number() : 0; };

      /** Get current byte number. Descendants may override this method. */
      virtual mootio::ByteOffset byte_number(size_t n) { return scanner ? scanner->byte_number(n) : 0; };
      //@}

      /*------------------------------------------------------------*/
      /** \name wasteLexer's own functions */
      //@{
      /** Set token source (usually some wasteScanner) */
      void from_reader(TokenReader *reader);

      /** Set stopword lexicon */
      void stopwords(const wasteLexicon &stopwords);

      /** Set conjunction lexicon */
      void conjunctions(const wasteLexicon &conjunctions);

      /** Set abbrev lexicon */
      void abbrevs(const wasteLexicon &abbrevs);
      //@}
  };

} // namespace moot


#endif /* _WASTE_LEXER_H */
