/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstBinIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : abstract librarian templates
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_BINIO_H
#define _DWDST_BINIO_H

#include <stdlib.h>

#include <vector>
#include <string>
#include <map>
#include <set>
#include <hash_map>
#include <hash_set>

#include <FSM.h>
#include <FSMTypes.h>

#include "dwdstEnum.h"
#include "dwdstMorph.h"
#include "dwdstCHMM.h"

#include "dwdstBinStream.h"

/** Namespace for structured binary stream I/O */
namespace dwdstBinIO {
  using namespace std;
  using namespace dwdstBinStream;

  /*------------------------------------------------------------
   * Generic items
   */
  template<class T> class Item {
  public:
    /** Load a single item */
    inline bool load(iBinStream &is, T &x) const
    {
      return is.getbytes((char *)&x, sizeof(T));
    };

    /** Save a single item */
    inline bool save(oBinStream &os, const T &x) const
    {
      return os.putbytes((char *)&x, sizeof(T));
    };

    /**
     * Load a C-array of items.
     *  'n' should hold the currently allocated length of 'x'.
     *  If the saved length is > n, 'x' will be re-allocated.
     *  The new size of the array will be stored in 'n' at completion.
     */
    inline bool load_n(iBinStream &is, T *&x, size_t &n) const {
      //-- get saved size
      Item<size_t> size_item;
      size_t saved_size;
      if (!size_item.load(is, saved_size)) return false;

      //-- re-allocate if necessary
      if (saved_size > n) {
	if (x) free(x);
	x = (T *)malloc(saved_size*sizeof(T));
	if (!x) {
	  n = 0;
	  return false;
	}
      }

      //-- read in items
      if (!is.getbytes((char *)x, sizeof(T)*saved_size)) return false;
      n=saved_size;
      return true;
    };

    /**
     * Save a C-array of items.
     * 'n' should hold the number of items in 'x', it will be stored too.
     */
    inline bool save_n(oBinStream &os, const T *x, size_t n) const {
      //-- get saved size
      Item<size_t> size_item;
      if (!size_item.save(os, n)) return false;

      //-- save items
      return os.putbytes((char *)x, n*sizeof(T));
    };
  };

  /*------------------------------------------------------------
   * C-strings
   */
  template<> class Item<char *> {
  public:
    Item<char> charItem;

  public:
    inline bool load(iBinStream &is, char *&x) const
    {
      size_t len=0;
      return charItem.load_n(is,x,len);
    };
 
    inline bool save(oBinStream &os, const char *x) const
    {
      if (x) {
	size_t len = strlen(x)+1;
	return charItem.save_n(os,x,len);
      } else {
	return charItem.save_n(os,"",1);
      }
    };
  };

  /*------------------------------------------------------------
   * C++ strings
   */
  template<> class Item<string> {
  public:
    Item<char *> cstr_item;
  public:
    inline bool load(iBinStream &is, string &x) const
    {
      char *buf=NULL;
      bool rc = cstr_item.load(is, buf);
      if (rc) x = buf;
      if (buf) free(buf);
      return rc;
    };

    inline bool save(oBinStream &os, const string &x) const
    {
      return cstr_item.save(os, x.c_str());
    };
  };

  /*------------------------------------------------------------
   * STL: vectors
   */
  template<class ValT> class Item<vector<ValT> > {
  public:
    Item<ValT> val_item;
  public:
    inline bool load(iBinStream &is, vector<ValT> &x) const
    {
      //-- get saved size
      Item<size_t> size_item;
      size_t len;
      if (!size_item.load(is, len)) return false;

      //-- resize
      x.clear();
      x.reserve(len);

      //-- read in items
      for ( ; len > 0; len--) {
	x.push_back(ValT());
	if (!val_item.load(is,x.back())) return false;
      }
      return len==0;
    };

    inline bool save(oBinStream &os, const vector<ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (vector<ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!val_item.save(os,*xi)) return false;
      }
      return true;
    };
  };


  /*------------------------------------------------------------
   * STL: set<>
   */
  template<class ValT> class Item<set<ValT> > {
  public:
    Item<ValT> val_item;
  public:
    inline bool load(iBinStream &is, set<ValT> &x) const
    {
      //-- load size
      Item<size_t> size_item;
      size_t len;
      if (!size_item.load(is, len)) return false;

      //-- clear
      x.clear();

      //-- read items
      ValT tmp;
      for ( ; len > 0; len--) {
	if (!val_item.load(is,tmp))
	  return false;
	x.insert(tmp);
      }
      return len==0;
    };

    inline bool save(oBinStream &os, const set<ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (set<ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!val_item.save(os,*xi)) return false;
      }
      return true;
    };
  };

  /*------------------------------------------------------------
   * STL: hash_set<>
   */
  template<class ValT> class Item<hash_set<ValT> > {
  public:
    Item<ValT> val_item;
  public:
    inline bool load(iBinStream &is, hash_set<ValT> &x) const
    {
      //-- load size
      Item<size_t> size_item;
      size_t len;
      if (!size_item.load(is, len)) return false;

      //-- clear & resize
      x.clear();
      x.resize(len);

      //-- read items
      ValT tmp;
      for ( ; len > 0; len--) {
	if (!val_item.load(is,tmp)) return false;
	x.insert(tmp);
      }
      return len==0;
    };

    inline bool save(oBinStream &os, const hash_set<ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (hash_set<ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!val_item.save(os,*xi)) return false;
      }
      return true;
    };
  };


  /*------------------------------------------------------------
   * STL: map<>
   */
  template<class KeyT, class ValT> class Item<map<KeyT,ValT> > {
  public:
    Item<KeyT> key_item;
    Item<ValT> val_item;
  public:
    inline bool load(iBinStream &is, map<KeyT,ValT> &x) const
    {
      //-- load size
      Item<size_t> size_item;
      size_t len;
      if (!size_item.load(is, len)) return false;

      //-- clear
      x.clear();

      //-- read items
      KeyT key_tmp;
      ValT val_tmp;
      for ( ; len > 0; len--) {
	if (!key_item.load(is,key_tmp) || !val_item.load(is,val_tmp))
	  return false;
	x[key_tmp] = val_tmp;
      }
      return len==0;
    };

    inline bool save(oBinStream &os, const map<KeyT,ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (map<KeyT,ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!key_item.save(os,xi->first) || !val_item.save(os,xi->second))
	  return false;
      }
      return true;
    };
  };


  /*------------------------------------------------------------
   * STL: hash_map<>
   */
  template<class KeyT, class ValT> class Item<hash_map<KeyT,ValT> > {
  public:
    Item<KeyT> key_item;
    Item<ValT> val_item;
  public:
    inline bool load(iBinStream &is, hash_map<KeyT,ValT> &x) const
    {
      //-- load size
      Item<size_t> size_item;
      size_t len;
      if (!size_item.load(is, len)) return false;

      //-- clear & resize
      x.clear();
      x.resize(len);

      //-- read items
      KeyT key_tmp;
      ValT val_tmp;
      for ( ; len > 0; len--) {
	if (!key_item.load(is,key_tmp) || !val_item.load(is,val_tmp))
	  return false;
	x[key_tmp] = val_tmp;
      }
      return len==0;
    };

    inline bool save(oBinStream &os, const hash_map<KeyT,ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (hash_map<KeyT,ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!key_item.save(os,xi->first) || !val_item.save(os,xi->second))
	  return false;
      }
      return true;
    };
  };

  /*------------------------------------------------------------
   * FSM types: FSMWeightedIOPair
   */
  template<class StrType>
  class Item<FSM::FSMWeightedIOPair<StrType> > {
  public:
    Item<StrType>   string_item;
    Item<FSMWeight> weight_item;
  public:
    inline bool load(iBinStream &is, FSM::FSMWeightedIOPair<StrType> &x) const
    {
      return (string_item.load(is, x.istr)
	      && string_item.load(is, x.ostr)
	      && weight_item.load(is, x.weight));
    };

    inline bool save(oBinStream &os, const FSM::FSMWeightedIOPair<StrType> &x) const
    {
      return (string_item.save(os, x.istr)
	      && string_item.save(os, x.ostr)
	      && weight_item.save(os, x.weight));
    };
  };

  /*------------------------------------------------------------
   * dwdst types: dwdstEnum
   */
  template<class NameT, class HashFunc, class NameEqlFunc>
  class Item<dwdstEnum<NameT,HashFunc,NameEqlFunc> > {
  public:
    Item<dwdstEnum<NameT,HashFunc,NameEqlFunc>::Id2NameMap> i2n_item;
  public:
    inline bool load(iBinStream &is, dwdstEnum<NameT,HashFunc,NameEqlFunc> &x) const
    {
      if (i2n_item.load(is, x.ids2names)) {
	x.names2ids.resize(x.ids2names.size());
	unsigned u;
	dwdstEnum<NameT,HashFunc,NameEqlFunc>::Id2NameMap::const_iterator ni;
	for (ni = x.ids2names.begin(), u = 0; ni != x.ids2names.end(); ni++, u++)
	  {
	    x.names2ids[*ni] = u;
	  }
	return true;
      }
      return false;
    };

    inline bool save(oBinStream &os, const dwdstEnum<NameT,HashFunc,NameEqlFunc> &x) const
    {
      return i2n_item.save(os, x.ids2names);
    };
  };


  /*------------------------------------------------------------
   * dwdst types: dwdstCHMM::TagMorphAnalysisSet
   */
  template<>
  class Item<dwdstCHMM::TagMorphAnalysisSet> {
  public:
    Item<set<dwdstCHMM::TagMorphAnalysis> > aset_item;
    Item<FSMWeight>                         weight_item;
  public:
    inline bool load(iBinStream &is, dwdstCHMM::TagMorphAnalysisSet &x) const
    {
      return aset_item.load(is, x.analyses) && weight_item.load(is, x.weight);
    };

    inline bool save(oBinStream &os, const dwdstCHMM::TagMorphAnalysisSet &x) const
    {
      return aset_item.save(os, x.analyses) && weight_item.save(os, x.weight);
    };
  };


  /*------------------------------------------------------------
   * dwdst types: dwdstMorph (HACK!)
   */
  template<>
  class Item<dwdstMorph> {
  public:
    Item<string> string_item;
    Item<bool>   bool_item;
    Item<int>    int_item;
  public:
    inline bool load(iBinStream &is, dwdstMorph &x) const
    {
      return (bool_item.load(is, x.want_avm)
	      && bool_item.load(is, x.want_mabbaw_format)
	      && bool_item.load(is, x.do_dequote)
	      && int_item.load(is, x.verbose)
	      && string_item.load(is, x.syms_filename)
	      && string_item.load(is, x.mfst_filename)
	      && string_item.load(is, x.xfst_filename)
	      //&& (x.sym_filename == "" || x.load_morph_symbols())
	      //&& (x.mfst_filename == "" || x.load_morph_fst())
	      //&& (x.xfst_filename == "" || x.load_tagx_fst())
	      );
    };

    inline bool save(oBinStream &os, const dwdstMorph &x) const
    {
      return (bool_item.save(os, x.want_avm)
	      && bool_item.save(os, x.want_mabbaw_format)
	      && bool_item.save(os, x.do_dequote)
	      && int_item.save(os, x.verbose)
	      && string_item.save(os, x.syms ? x.syms_filename : string(""))
	      && string_item.save(os, x.mfst ? x.mfst_filename : string(""))
	      && string_item.save(os, x.xfst ? x.xfst_filename : string(""))
	      );
    };
  };
 

  /*------------------------------------------------------------
   * public typedefs: Generic header information
   */
  /** Header information struct */
  class HeaderInfo {
  public:
    /** Typedef for a version component */
    typedef unsigned int VersionT;
    
    /** Typedef for a "magic number" component */
    typedef unsigned int MagicT;
    
    /** Typedef for a generic "flags" component */
    typedef unsigned long int FlagsT;
  public:
    MagicT    magic;      /**< Magic number */
    VersionT  version;    /**< Major version */
    VersionT  revision;   /**< Minor version */
    VersionT  minver;     /**< Minimum compatible version */
    VersionT  minrev;     /**< Minimum compatible revision */
    FlagsT    flags;      /**< Some user flags (unused) */
  public:
    /** Default constructor */
    HeaderInfo(MagicT mag=0,
	       VersionT ver=0, VersionT rev=0,
	       VersionT mver=0, VersionT mrev=0,
	       FlagsT f=0)
      : magic(mag),
	version(ver),
	revision(rev),
	minver(mver),
	minrev(mrev),
	flags(f)
    {};

    /** Useful constructor: generate "magic" from IDstring by hashing */
    HeaderInfo(const string &IDstring,
	       VersionT ver=0, VersionT rev=0,
	       VersionT mver=0, VersionT mrev=0,
	       FlagsT f=0)
      : version(ver),
	revision(rev),
	minver(mver),
	minrev(mrev),
	flags(f)
    {
      magic = 0;
      for (string::const_iterator si = IDstring.begin(); si != IDstring.end(); si++) {
	magic = (magic<<5)-magic + (MagicT)*si;
      }
    };
  };

}; //-- dwdstBinIO


#endif /* _DWDST_BINIO_H */
