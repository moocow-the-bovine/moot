/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2011 by Bryan Jurish <moocow@cpan.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootBinIO.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moot PoS tagger : abstract templates for binary librarians
 *--------------------------------------------------------------------------*/

/** \file mootBinIO.h
 *  \brief abstract templates for binary I/O
 */

#ifndef _MOOT_BINIO_H
#define _MOOT_BINIO_H

//#include <stdlib.h> //-- via mootHMM -> mootTokenIO -> mootTokenLexer -> mootGenericLexer -> mootBufferIO

#include <mootHMM.h>

/** \brief Namespace for structured binary stream I/O */
namespace mootBinIO {
  using namespace std;
  using namespace moot;
  using namespace mootio;

  /*------------------------------------------------------------
   * Generic functions
   */
  /** Load a single binary item using its native size */
  template<class T>
  inline bool loadItem(mootio::mistream *is, T &x)
  {
      return is->read(reinterpret_cast<char *>(&x), sizeof(T)) == sizeof(T);
  };

  /** Save a single binary item using its native size */
  template<class T>
  inline bool saveItem(mootio::mostream *os, const T &x)
  {
    return os->write(reinterpret_cast<const char *>(&x), sizeof(T));
  };

  /*------------------------------------------------------------
   * Generic items
   */
  /** \brief Binary item I/O template class, used for binary HMM model files */
  template<class T> class Item {
  public:
    /** Load a single item */
    inline bool load(mootio::mistream *is, T &x) const
    {
      return loadItem<T>(is,x);
    };

    /** Save a single item */
    inline bool save(mootio::mostream *os, const T &x) const
    {
      return saveItem<T>(os,x);
    };

    /**
     * Load a C-array of items.
     *  'n' should hold the currently allocated length of 'x'.
     *  If the saved length is > n, 'x' will be re-allocated.
     *  The new size of the array will be stored in 'n' at completion.
     */
    inline bool load_n(mootio::mistream *is, T *&x, size_t &n) const {
      //-- get saved size
      Size saved_size;
      if (!loadItem<Size>(is,saved_size)) return false;

      //-- re-allocate if necessary
      if (saved_size > n) {
	if (x) free(x);
	x = reinterpret_cast<T*>(malloc(saved_size*sizeof(T)));
	if (!x) {
	  n = 0;
	  return false;
	}
      }

      //-- read in items
      ByteCount wanted = sizeof(T)*saved_size;
      if (is->read(reinterpret_cast<char *>(x), wanted) != wanted) return false;
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
      Size tmp = n;
      if (!saveItem<Size>(os,tmp)) return false;

      //-- save items
      return os->write(reinterpret_cast<const char *>(x), n*sizeof(T));
    };
  };

#ifdef MOOT_32BIT_FORCE
  /*------------------------------------------------------------
   * size_t : special case bashes to 32-bits
   */
  /** \brief Binary I/O template instantiation for signed integers
   *
   * Uses global typedef BinInt, i.e. may bash value to 32-bits
   */
  template<> class Item<size_t> {
  public:

  public:
    inline bool load(mootio::mistream *is, size_t &x) const
    {
      if (sizeof(size_t)==sizeof(Size)) {
	//-- 32-bit native sizes: just load raw data
	return loadItem<size_t>(is,x);
      }
      Size tmp;
      bool rc = loadItem<Size>(is,tmp);
      x = tmp;
      return rc;
    };
 
    inline bool save(mootio::mostream *os, const size_t &x) const
    {
      if (sizeof(size_t)==sizeof(Size)) {
	//-- 32-bit native sizes: just load raw data
	return saveItem<size_t>(os,x);
      }
      Size tmp = x;
      return saveItem<Size>(os, tmp);
    };
  };
#endif /* MOOT_32BIT_FORCE */


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
  template<class KeyT, class ValT, class HashFuncT, class EqualFuncT>
  class Item<hash_map<KeyT,ValT,HashFuncT,EqualFuncT> > {
  public:
    Item<KeyT> key_item;
    Item<ValT> val_item;
  public:
    inline bool load(mootio::mistream *is, hash_map<KeyT,ValT,HashFuncT,EqualFuncT> &x) const
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

    inline bool save(mootio::mostream *os, const hash_map<KeyT,ValT,HashFuncT,EqualFuncT> &x) const
    {
      //-- save size
      Item<size_t> size_item;
      if (!size_item.save(os, x.size())) return false;

      //-- save items
      for (typename hash_map<KeyT,ValT,HashFuncT,EqualFuncT>::const_iterator xi = x.begin();
	   xi != x.end();
	   xi++)
	{
	  if (!key_item.save(os,xi->first) || !val_item.save(os,xi->second))
	    return false;
	}
      return true;
    };
  };

  /*------------------------------------------------------------
   * STL: pair
   */
  /** \brief Binary I/O template instantiation for STL map<> */
  template<class T1, class T2>
  class Item<std::pair<T1,T2> > {
  public:
    Item<T1>   item1;
    Item<T2>   item2;
  public:
    inline bool load(mootio::mistream *is, std::pair<T1,T2> &x) const
    {
      return (item1.load(is,x.first) && !item2.load(is,x.second));
    };

    inline bool save(mootio::mostream *os, const std::pair<T1,T2> &x) const
    {
      return (item1.save(os,x.first) && item2.save(os,x.second));
    };
  };

  /*------------------------------------------------------------
   * moot types: Trigram
   */
  template <>
  class Item<mootHMM::Trigram> {
  public:
    Item<mootHMM::TagID> tagid_item;
  public:
    inline bool load(mootio::mistream *is, mootHMM::Trigram &x) const
    {
      return (tagid_item.load(is, x.tag1)
	      && tagid_item.load(is, x.tag2)
	      && tagid_item.load(is, x.tag3));
    };
    inline bool save(mootio::mostream *os, const mootHMM::Trigram &x) const
    {
      return (tagid_item.save(os, x.tag1)
	      && tagid_item.save(os, x.tag2)
	      && tagid_item.save(os, x.tag3));
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
   * moot types: AssocVector
   */
  template<typename KeyT, typename ValT>
  class Item<AssocVector<KeyT,ValT> >
  {
  public:
    typedef typename AssocVector<KeyT,ValT>::assoc_vector_type assoc_vector_type;
    Item<assoc_vector_type> vec_item;
  public:
    inline bool load(mootio::mistream *is, AssocVector<KeyT,ValT> &x) const
    { return vec_item.load(is,x); };
    inline bool save(mootio::mostream *os, const AssocVector<KeyT,ValT> &x) const
    { return vec_item.save(os,x); };
  };

  /*------------------------------------------------------------
   * moot types: TrieVectorNode
   */
  template <typename DataT, typename CharT, typename UCharT>
  class Item<TrieVectorNode<DataT,CharT,UCharT> > {
  public:
    Item<CharT> char_item;
    Item<UCharT> uchar_item;
    Item<DataT> data_item;
    Item<size_t> size_item;
  public:
    inline bool load(mootio::mistream *is, TrieVectorNode<DataT,CharT,UCharT> &x) const
    {
      return (size_item.load(is,x.mother)
	      && size_item.load(is,x.mindtr)
	      && char_item.load(is,x.label)
	      && uchar_item.load(is,x.ndtrs)
	      && data_item.load(is,x.data));
    };
    inline bool save(mootio::mostream *os, const TrieVectorNode<DataT,CharT,UCharT> &x) const
    {
      return (size_item.save(os,x.mother)
	      && size_item.save(os,x.mindtr)
	      && char_item.save(os,x.label)
	      && uchar_item.save(os,x.ndtrs)
	      && data_item.save(os,x.data));
    };
  };
  

  /*------------------------------------------------------------
   * moot types: SuffixTrie
   */
  /** \brief Binary I/O template instantiation for SuffixTrie */
  template<>
  class Item<SuffixTrie> {
  public:
    Item<SuffixTrie::vector_type> vec_item;
    Item<CountT>                  maxcount_item;
    Item<ProbT>                   theta_item;
  public:
    inline bool load(mootio::mistream *is, SuffixTrie &x) const
    {
      x.clear();
      return (maxcount_item.load(is, x.maxcount)
	      && theta_item.load(is, x.theta)
	      && vec_item.load(is,x));
    };
    inline bool save(mootio::mostream *os, const SuffixTrie &x) const
    {
      return (maxcount_item.save(os, x.maxcount)
	      && theta_item.save(os, x.theta)
	      && vec_item.save(os, x));
    };
  };

}; //-- mootBinIO


#endif /* _MOOT_BINIO_H */
