/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootBinIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moot PoS tagger : abstract templates for binary librarians
 *--------------------------------------------------------------------------*/

#ifndef _MOOT_BINIO_H
#define _MOOT_BINIO_H

#include <stdlib.h>

#include <vector>
#include <string>
#include <map>
#include <set>

#include <mootTypes.h>
#include <mootEnum.h>
#include <mootIO.h>


/** \brief Namespace for structured binary stream I/O */
namespace mootBinIO {
  using namespace std;
  using namespace moot;
  using namespace mootio;

  /*------------------------------------------------------------
   * Generic items
   */
  /** \brief Binary item I/O template class, used for binary HMM model files */
  template<class T> class Item {
  public:
    /** Load a single item */
    inline bool load(mootio::mistream *is, T &x) const
    {
      return is->read((char *)&x, sizeof(T)) == sizeof(T);
    };

    /** Save a single item */
    inline bool save(mootio::mostream *os, const T &x) const
    {
      return os->write((char *)&x, sizeof(T));
    };

    /**
     * Load a C-array of items.
     *  'n' should hold the currently allocated length of 'x'.
     *  If the saved length is > n, 'x' will be re-allocated.
     *  The new size of the array will be stored in 'n' at completion.
     */
    inline bool load_n(mootio::mistream *is, T *&x, size_t &n) const {
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
      ByteCount wanted = sizeof(T)*saved_size;
      if (is->read((char *)x, wanted) != wanted) return false;
      n=saved_size;
      return true;
    };

    /**
     * Save a C-array of items.
     * 'n' should hold the number of items in 'x', it will be
     * written first.
     */
    inline bool save_n(mootio::mostream *os, const T *x, size_t n) const {
      //-- get saved size
      Item<size_t> size_item;
      if (!size_item.save(os, n)) return false;

      //-- save items
      return os->write((char *)x, n*sizeof(T));
    };
  };

  /*------------------------------------------------------------
   * C-strings
   */
  /** \brief Binary I/O template instantiation for C strings
   *
   *  Terminating NULs are loaded/saved too.
   */
  template<> class Item<char *> {
  public:
    Item<char> charItem;

  public:
    inline bool load(mootio::mistream *is, char *&x) const
    {
      size_t len=0;
      return charItem.load_n(is,x,len);
    };
 
    inline bool save(mootio::mostream *os, const char *x) const
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
  /** \brief Binary I/O template instantiation for STL strings
   *
   * Terminating NULs are not loaded or saved.
   */
  template<> class Item<string> {
  public:
    Item<char> charItem;
  public:
    inline bool load(mootio::mistream *is, string &x) const
    {
      char *buf=NULL;
      size_t len=0;
      bool rc = charItem.load_n(is,buf,len);
      if (rc && len) x.assign(buf,len);
      if (buf) free(buf);
      return rc;
    };

    inline bool save(mootio::mostream *os, const string &x) const
    {
      return charItem.save_n(os,x.data(),x.size());
    };
  };

  /*------------------------------------------------------------
   * STL: vectors
   */
  /** \brief Binary I/O template instantiation for STL vector<> */
  template<class ValT> class Item<vector<ValT> > {
  public:
    Item<ValT> val_item;
  public:
    inline bool load(mootio::mistream *is, vector<ValT> &x) const
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

    inline bool save(mootio::mostream *os, const vector<ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (typename vector<ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!val_item.save(os,*xi)) return false;
      }
      return true;
    };
  };


  /*------------------------------------------------------------
   * STL: set<>
   */
  /** \brief Binary I/O template instantiation for STL set<> */
  template<class ValT> class Item<set<ValT> > {
  public:
    Item<ValT> val_item;
  public:
    inline bool load(mootio::mistream *is, set<ValT> &x) const
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

    inline bool save(mootio::mostream *os, const set<ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (typename set<ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!val_item.save(os,*xi)) return false;
      }
      return true;
    };
  };

  /*------------------------------------------------------------
   * STL: hash_set<>
   */
  /** \brief Binary I/O template instantiation for STL hash_set<> */
  template<class ValT> class Item<hash_set<ValT> > {
  public:
    Item<ValT> val_item;
  public:
    inline bool load(mootio::mistream *is, hash_set<ValT> &x) const
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

    inline bool save(mootio::mostream *os, const hash_set<ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (typename hash_set<ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!val_item.save(os,*xi)) return false;
      }
      return true;
    };
  };


  /*------------------------------------------------------------
   * STL: map<>
   */
  /** \brief Binary I/O template instantiation for STL map<> */
  template<class KeyT, class ValT> class Item<map<KeyT,ValT> > {
  public:
    Item<KeyT> key_item;
    Item<ValT> val_item;
  public:
    inline bool load(mootio::mistream *is, map<KeyT,ValT> &x) const
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

    inline bool save(mootio::mostream *os, const map<KeyT,ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (typename map<KeyT,ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!key_item.save(os,xi->first) || !val_item.save(os,xi->second))
	  return false;
      }
      return true;
    };
  };


  /*------------------------------------------------------------
   * STL: hash_map<>
   */
  /** \brief Binary I/O template instantiation for STL hash_map<> */
  template<class KeyT, class ValT> class Item<hash_map<KeyT,ValT> > {
  public:
    Item<KeyT> key_item;
    Item<ValT> val_item;
  public:
    inline bool load(mootio::mistream *is, hash_map<KeyT,ValT> &x) const
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

    inline bool save(mootio::mostream *os, const hash_map<KeyT,ValT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (typename hash_map<KeyT,ValT>::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	if (!key_item.save(os,xi->first) || !val_item.save(os,xi->second))
	  return false;
      }
      return true;
    };
  };

  /*------------------------------------------------------------
   * moot types: mootEnum
   */
  /** \brief Binary I/O template instantiation for mootEnum<> */
  template<class NameT, class HashFunc, class NameEqlFunc>
  class Item<mootEnum<NameT,HashFunc,NameEqlFunc> > {
  public:
    Item<typename mootEnum<NameT,HashFunc,NameEqlFunc>::Id2NameMap> i2n_item;
  public:
    inline bool load(mootio::mistream *is, mootEnum<NameT,HashFunc,NameEqlFunc> &x) const
    {
      if (i2n_item.load(is, x.ids2names)) {
	x.names2ids.resize(x.ids2names.size());
	unsigned u;
	typename mootEnum<NameT,HashFunc,NameEqlFunc>::Id2NameMap::const_iterator ni;
	for (ni = x.ids2names.begin(), u = 0; ni != x.ids2names.end(); ni++, u++)
	  {
	    x.names2ids[*ni] = u;
	  }
	return true;
      }
      return false;
    };

    inline bool save(mootio::mostream *os, const mootEnum<NameT,HashFunc,NameEqlFunc> &x) const
    {
      return i2n_item.save(os, x.ids2names);
    };
  };


  /*------------------------------------------------------------
   * public typedefs: Generic header information
   */
  /** \brief Header information structure, used for binary HMM model files */
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

}; //-- mootBinIO


#endif /* _MOOT_BINIO_H */
