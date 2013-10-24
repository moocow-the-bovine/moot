#include "wasteLexer.h"

namespace moot
{

//----------------------------------------------------------------------
wasteLexer::wasteLexer(int fmt, const std::string &myname)
  : TokenReader(fmt, myname)
{
  tr_token = &wl_token;
  tr_sentence = &wl_sentence;
}

//----------------------------------------------------------------------
wasteLexer::~wasteLexer()
{
}

//----------------------------------------------------------------------
void wasteLexer::from_mstream(mootio::mistream *mistreamp)
{
  scanner->from_mstream(mistreamp);
}

//----------------------------------------------------------------------
void wasteLexer::close(void)
{
  scanner->close();
}

//----------------------------------------------------------------------
mootTokenType wasteLexer::get_token(void)
{
  wl_token.clear();

  // if scanner has not been set
  if (!scanner)
    return wl_token.toktype(TokTypeEOF);

  //TODO: dehyphenation
  mootTokenType toktyp = scanner->get_token();
  wl_token = *(scanner->token ());

  //-- token type
  switch ( toktyp ) {
  case TokTypeVanilla:		break;
  default:			return toktyp;
  }

  /*
  //-- token text
  if (wts_token.text().empty())
    wts_token.text(scanner.yytext());

  //-- token location
  wts_token.location( scanner.theByte-scanner.yyleng(), scanner.yyleng() );

  //-- +Tagged: set tag text
  if ( tr_format&tiofTagged )
    wts_token.besttag( wasteScannerTypeNames[scantyp] );

  //-- +Analyzed: set analysis text
  if ( tr_format&tiofAnalyzed )
    wts_token.insert( wasteScannerTypeNames[scantyp], "" );

  //-- return
  return wts_token.toktype();
  */
  return wl_token.toktype();
}

//----------------------------------------------------------------------
mootTokenType wasteLexer::get_sentence(void)
{
  wl_sentence.clear();
  mootTokenType toktyp;
  for (toktyp=get_token(); toktyp != TokTypeEOF; toktyp=get_token()) {
    wl_sentence.push_back( wl_token );
  }
  return toktyp;
}


}; //-- /namespace moot;
