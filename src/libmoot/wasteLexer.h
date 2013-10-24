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

namespace moot
{
  class wasteLexicon
  {
    public:
      typedef moot_STL_NAMESPACE::hash_set<std::string>  Lexicon;
      Lexicon lex;

      wasteLexicon();
      ~wasteLexicon;

      void load(mistream *mis);
      void load(const char *filename);
      void clear();
      bool lookup(const std::string &word);
  };

  class wasteLexer : public TokenReader
  {
    public:
      /*--------------------------------------------------------------------
       * wasteLexer: Data
       */
      /*------------------------------------------------------------*/
      /** \name Low-level data */
      //@{
      TokenReader    *scanner;          /**< Input source */
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
      virtual size_t column_number(void) { return scanner.theColumn; };

      /** Set current column number. Descendants may override this method. */
      virtual size_t column_number(size_t n) { return scanner.theColumn = n; };

      /** Get current byte number. Descendants may override this method. */
      virtual mootio::ByteOffset byte_number(void) { return scanner.theByte; };

      /** Get current byte number. Descendants may override this method. */
      virtual mootio::ByteOffset byte_number(size_t n) { return scanner.theByte = n; };
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
