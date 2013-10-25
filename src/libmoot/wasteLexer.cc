#include "wasteLexer.h"

#include <sstream>

namespace moot
{

//----------------------------------------------------------------------
wasteLexer::wasteLexer(int fmt, const std::string &myname)
  : TokenReader(fmt, myname),
    scanner(NULL)
{
  // wasteLexer has local contents
  tr_token = &wl_token;
  tr_sentence = &wl_sentence;

  // build the tagset
  const char* cls_strings[n_cls] = {"stop","rom","alpha","num","$.","$,","$:","$;","$?","$(","$)","$-","$+","$/","$\"","$\'","$~","other"};
  const char* cas_strings[n_cas] = {"*","lo","up","cap"};
  const char* abbr_strings[n_binary] = {"0","1"};
  const char* len_strings[n_len] = {"0","1","3","5","N"};
  const char* blanks_strings[n_binary] = {"0","1"};

  wl_tagset.resize(n_cls);
  for (int i = 0; i < n_cls; ++i)
  {
    wl_tagset[i].resize(n_cas);
    for (int j = 0; j < n_cas; ++j)
    {
      wl_tagset[i][j].resize(n_binary);
      for (int k = 0; k < n_binary; ++k)
      {
        wl_tagset[i][j][k].resize(n_len);
        for (int l = 0; l < n_len; ++l)
        {
          wl_tagset[i][j][k][l].resize(n_binary);
          for (int m = 0; m < n_binary; ++m)
          {
            wl_tagset[i][j][k][l][m].resize(7);
            std::ostringstream tag;
            tag << cls_strings[i]
              << ',' << cas_strings[j]
              << ",a" << abbr_strings[k]
              << ",l" << len_strings[l]
              << ",b" << blanks_strings[m];
            wl_tagset[i][j][k][l][m][0] = tag.str ();
            wl_tagset[i][j][k][l][m][1] = tag.str () + ",s0,S0,w0";
            wl_tagset[i][j][k][l][m][2] = tag.str () + ",s0,S1,w0";
            wl_tagset[i][j][k][l][m][3] = tag.str () + ",s0,S0,w1";
            wl_tagset[i][j][k][l][m][4] = tag.str () + ",s0,S1,w1";
            wl_tagset[i][j][k][l][m][5] = tag.str () + ",s1,S0,w1";
            wl_tagset[i][j][k][l][m][6] = tag.str () + ",s1,S1,w1";
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------
wasteLexer::~wasteLexer()
{
}

//----------------------------------------------------------------------
void wasteLexer::from_mstream(mootio::mistream *mistreamp)
{
  if(scanner)
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

  wasteLexerTypeE lexertyp = waste_lexertype(wl_token.text());
  switch ( lexertyp ) {
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
