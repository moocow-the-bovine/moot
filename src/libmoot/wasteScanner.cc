
#include "wasteScanner.h"

moot::wasteScanner::wasteScanner(const std::string &myname,
          size_t line, size_t column, mootio::ByteOffset byte, size_t init_bufsize)
        : GenericLexer(myname,line,column,byte),
          yybuffer(NULL),
          yycursor(NULL),
          yylimit(NULL),
          yytoken(NULL),
          yymarker(NULL),
          buffer_size(init_bufsize)
{
  yybuffer = new char[buffer_size];
  yycursor = yylimit = yytoken = yymarker = yybuffer;
}

moot::wasteScanner::~wasteScanner()
{
  if(yybuffer)
    delete[] yybuffer;
}

bool moot::wasteScanner::fill ( size_t n )
{
  if ( !mglin || mglin->eof() )
  {
    if ( ( yylimit - yycursor ) <= 0) {
      return false;
    }
  }

  ptrdiff_t restSize = yylimit - yytoken;
  if ( restSize + n >= buffer_size )
  {
    // extend buffer
    buffer_size *= 2;
    char* newBuffer = new char[buffer_size];
    for ( ptrdiff_t i = 0; i < restSize; ++i )
    {
      // memcpy
      *( newBuffer + i ) = *( yytoken + i );
    }
    yymarker = newBuffer + ( yymarker - yytoken );
    yycursor = newBuffer + ( yycursor - yytoken );
    yytoken = newBuffer;
    yylimit = newBuffer + restSize;

    delete[] yybuffer;
    yybuffer = newBuffer;
  }
  else
  {
    // move remained data to head.
    for ( ptrdiff_t i = 0; i < restSize; ++i )
    {
      //memmove( yybuffer, yytoken, (restSize)*sizeof(char) );
      *( yybuffer + i ) = * ( yytoken + i );
    }
    yymarker = yybuffer + ( yymarker - yytoken );
    yycursor = yybuffer + ( yycursor - yytoken );
    yytoken = yybuffer;
    yylimit = yybuffer + restSize;
  }

  // fill to buffer
  size_t read_size = buffer_size - restSize;
  yylimit += mglin->read ( yylimit, read_size );

  return true;
}

void moot::wasteScanner::reset ()
{
  GenericLexer::reset();
  yycursor = yylimit = yytoken = yymarker = yybuffer;
}

void moot::wasteScanner::from_mstream (mootio::mistream *in)
{
    if (in) {
      if (mglin && in != mglin && mgl_in_created) {
	delete mglin;
	mgl_in_created = false;
      }
      mglin = in;
    }
    reset ();
}
