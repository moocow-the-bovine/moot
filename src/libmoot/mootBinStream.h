/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: mootBinStream.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : Binary stream hacks
 *--------------------------------------------------------------------------*/

#ifndef _moot_BINSTREAM_H
#define _moot_BINSTREAM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <zlib.h>

#include <iostream>
#include <fstream>

namespace mootBinStream {
  using namespace std;
  using namespace mootBinStream;

  /*------------------------------------------------------------
   * Stream Wrappers
   */

  /** Abstract class for binary streams -- ugly, but useful */
  class BinStream {
  public:
    /** Default constructor */
    BinStream(void) {};

    /** Destructor */
    virtual ~BinStream(void) { this->close(); };

    /** Open a file descriptor */
    virtual bool dopen(int fd) { return false; };

    /** Close the stream */
    virtual void close(void) {};
  };

  /** Abstract class for binary input streams -- ugly, but useful */
  class iBinStream : public BinStream {
  public:
    /** Constructors */
    iBinStream(void) {};
    virtual bool dopen(int fd) { return false; };

    /** Read n bytes from the stream into the string buf */
    virtual bool getbytes(char *buf, const size_t n) { return false; };
  };

  /** Abstract class for binary output streams -- ugly, but useful */
  class oBinStream {
  public:
    /** Constructor */
    oBinStream(void) {};
    oBinStream(int fd) {};

    /** Write n bytes from buf to the stream */
    virtual bool putbytes(char *buf, const size_t n) { return false; };
  };

  /** Binary input stream using C "FILE *" */
  class icBinStream : public iBinStream {
  public:
    FILE *file;
  public:
    /** Constructor */
    icBinStream(FILE *f=NULL) : file(f) {};

    /* Open */
    virtual bool dopen(int fd) {
      if (file) fclose(file);
      file = fdopen(dup(fd), "r");
      return file ? true : false;
    };

    /* Close */
    virtual void close(void) { fclose(file); file=NULL; }

    /** Read n bytes from the stream into the string buf */
    virtual bool getbytes(char *buf, const size_t n)
    {
      return fread(buf, 1, n, file)==n;
    };
  };

  /** Binary output stream using C "FILE *" */
  class ocBinStream : public oBinStream {
  public:
    FILE *file;
  public:
    /** Constructor */
    ocBinStream(FILE *f=NULL) : file(f) {};

    /* Open */
    virtual bool dopen(int fd) {
      if (file) fclose(file);
      file = fdopen(dup(fd), "w");
      return file ? true : false;
    };

    /* Close */
    virtual void close(void) { fclose(file); file=NULL; }

    /** Write n bytes from buf to the stream */
    virtual bool putbytes(char *buf, const size_t n)
    {
      return fwrite(buf, 1, n, file)==n;
    };
  };

  /** Binary input stream using C++ 'istream' */
  class iccBinStream : public iBinStream {
  public:
    istream *is;
  public:
    /** Constructor */
    iccBinStream(istream *ins=NULL) : is(ins) {};

    /* Open */
    /*
    virtual bool dopen(int fd) {
      if (is) {
	delete is;
	is = NULL;
      }
      is = new ifstream(fd);
      return *is;
    };
    */

    /* Close */
    virtual void close(void) {
      if (is) {
	delete is;
	is = NULL;
      }
    };

    /** Read n bytes from the stream into the string buf */
    virtual bool getbytes(char *buf, const size_t n)
    {
      return is != NULL && is->read(buf, n).good();
    };
  };

  /** Binary output stream using C++ "ostream" */
  class occBinStream : public oBinStream {
  public:
    ostream *os;
  public:
    /** Constructor */
    occBinStream(ostream *outs=NULL) : os(outs) {};

    /* Open */
    /*
    virtual bool dopen(int fd) {
      if (os) {
	delete os;
	os = NULL;
      }
      os = new ofstream(fd);
      return *os;
    };
    */

    /* Close */
    virtual void close(void) {
      if (os) {
	delete os;
	os = NULL;
      }
    };

    /** Write n bytes from buf to the stream */
    virtual bool putbytes(char *buf, const size_t n)
    {
      return os != NULL && os->write(buf, n).good();
    };
  };

  /** Binary input stream using libz 'gzFile' */
  class izBinStream : public iBinStream {
  public:
    gzFile zf;
  public:
    /** Constructor */
    izBinStream(gzFile zin=NULL) : zf(zin) {};

    /** Open */
    virtual bool dopen(int fd) {
      if (zf) gzclose(zf);
      zf = gzdopen(dup(fd), "rb");
      return zf != NULL;
    };

    /** Close */
    virtual void close(void) {
      if (zf) gzclose(zf);
      zf = NULL;
    };

    /** Read n bytes from the stream into the string buf */
    virtual bool getbytes(char *buf, const size_t n)
    {
      return gzread(zf, buf, n)==(int)n;
    };
  };

  /** Binary output stream using zlib 'gzFile' */
  class ozBinStream : public oBinStream {
  public:
    gzFile zf;
  public:
    /** Constructor */
    ozBinStream(gzFile zout=NULL) : zf(zout) {};

    /** Open */
    virtual bool dopen(int fd) {
      if (zf) gzclose(zf);
      zf = gzdopen(dup(fd), "wb");
      return zf != NULL;
    };

    /** Close */
    virtual void close(void) {
      if (zf) gzclose(zf);
      zf = NULL;
    };

    /** Write n bytes from buf to the stream */
    virtual bool putbytes(char *buf, const size_t n)
    {
      return gzwrite(zf, buf, n)==(int)n;
    };
  };

};

#endif /* _moot_BINSTREAM_H */
