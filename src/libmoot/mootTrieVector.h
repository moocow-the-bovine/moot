/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2007 by Bryan Jurish <moocow@cpan.org>

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
 * File: TrieVector.h
 * Author: Bryan Jurish <moocow@cpan.org>
 * Description:
 *   + moocow's PoS tagger : tries
 *--------------------------------------------------------------------------*/

/**
\file mootTrieVector.h
\brief trie implementation using std::vector<> for underlying storage
*/

#ifndef MOOT_TRIE_VECTOR_H
#define MOOT_TRIE_VECTOR_H

#include <mootConfig.h>

#undef MOOT_TRIE_VECTOR_DEBUG

#ifdef MOOT_TRIE_VECTOR_DEBUG
# include <stdio.h>
# include <stdlib.h>
#endif

#include <ctype.h>
#include <vector>
#include <string>
#include <map>

namespace moot {

//======================================================================
// TrieVectorNodeBase
struct TrieVectorNodeBase
{
  size_t mother;  ///< index of this node's mother
  size_t mindtr;  ///< index of first arc from this node

  TrieVectorNodeBase(size_t mother_index=0, size_t mindtr_index=0)
    : mother(mother_index), mindtr(mindtr_index)
  {};
};

//======================================================================
// TrieVectorNode
/** Struct for single trie nodes */
template <typename DataT, typename CharT = char, typename UCharT = unsigned char>
struct TrieVectorNode : public TrieVectorNodeBase
{
  typedef DataT                                          data_type;
  typedef CharT                                          char_type;
  typedef UCharT                                         uchar_type;
  typedef TrieVectorNode<data_type,char_type,uchar_type>  node_type;

  CharT   label; ///< label of arc to this node
  UCharT  ndtrs; ///< number of daughters
  DataT   data;  ///< data associated with this node

  TrieVectorNode(size_t mother_index=0,
		size_t mindtr_index=0,
		CharT  node_label=0,
		UCharT node_ndtrs=0)
    : TrieVectorNodeBase(mother_index, mindtr_index),
      label(node_label),
      ndtrs(node_ndtrs)
  {};

  TrieVectorNode(size_t mother_index,
		size_t mindtr_index,
		CharT  node_label,
		UCharT node_ndtrs,
		const DataT  &node_data)
    : TrieVectorNodeBase(mother_index, mindtr_index),
      label(node_label),
      ndtrs(node_ndtrs),
      data(node_data)
  {};

  inline bool operator< (const TrieVectorNode &x) const
  { return mother < x.mother || label < x.label; };

  inline bool operator<= (const TrieVectorNode &x) const
  { return mother <= x.mother || label <= x.label; };

  inline bool operator== (const TrieVectorNode &x) const
  { return mother==x.mother && label==x.label; };
};


//======================================================================
// TrieVectorBase
/** \brief Base class for TrieVector */
class TrieVectorBase {
public:
  /** Typedef for node Ids */
  typedef size_t NodeId;

public:
  /** Failure flag for node searches */
  static const NodeId NoNode   = static_cast<size_t>(-1);

  /** Default maximum trie depth */
  static const size_t NoMaxLen = static_cast<size_t>(-1);

public:
  size_t   trie_maxlen;        ///< maximum trie depth
  bool     trie_use_case;      ///< whether to use case

public:
  TrieVectorBase(size_t maxlen=NoMaxLen, bool use_case=false)
    : trie_maxlen(maxlen),
      trie_use_case(use_case)
  {};

}; //-- /TrieVectorBase

//======================================================================
// TrieVector

/** \brief Top-level trie class-template using an adjaceny table.
 *
 * Requirements for DataT:
 * \li must have a default constructor
 *
 * Requirements for CharT
 * \li should support implicit conversion from integer constants (esp. zero)
 *
 * Requirements for UCharT
 * \li must be an unsigned type
 * \li should be at least as wide as CharT
 */
template<class DataT, typename CharT = char, typename UCharT = unsigned char>
class TrieVector
  : public TrieVectorBase,
    public std::vector<TrieVectorNode<DataT,CharT,UCharT> >
{
public:
  //--------------------------------------------------------------------
  // TrieVector: Types
  typedef DataT                              data_type;
  typedef CharT                              char_type;
  typedef UCharT                             uchar_type;

  typedef
     TrieVector<data_type,char_type,uchar_type>
     trie_type;

  typedef
     TrieVectorNode<data_type,char_type,uchar_type>
     node_type;

  typedef std::vector<node_type>                   vector_type;

  typedef typename vector_type::iterator           iterator;
  typedef typename vector_type::const_iterator     const_iterator;

  //-- moo: gcc-3.4 don't like this
  //typedef std::string<char_type>                     string_type;
  typedef std::string                                  string_type;

  typedef typename string_type::iterator               string_iterator;
  typedef typename string_type::const_iterator         const_string_iterator;
  typedef typename string_type::reverse_iterator       reverse_string_iterator;
  typedef typename string_type::const_reverse_iterator const_reverse_string_iterator;

  typedef std::map<string_type,NodeId>                 map_type;
  typedef typename map_type::iterator                  map_iterator;
  typedef typename map_type::const_iterator            const_map_iterator;

public:
  //--------------------------------------------------------------------
  // TrieVector: Data
  map_type  trie_pending;       ///< pending insertions
  data_type trie_default_data;  ///< default data

public:
  //--------------------------------------------------------------------
  // TrieVector: Methods

  //--------------------------------------------------
  /// \name Constructors etc.
  //@{
  /** Default constructor */
  TrieVector(size_t max_len=NoMaxLen, bool use_case=false)
    : TrieVectorBase(max_len,use_case),
      vector_type(),
      trie_pending(map_type()),
      trie_default_data(data_type())
  {};

  /** Destructor */
  ~TrieVector(void) {};

  /** Clear */
  inline void clear(void)
  {
    vector_type::clear();
    trie_pending.clear();
    //vector_type::push_back(node_type(0,0,0,0)); //-- add root: (mom,mindtr,label,ndtrs)
  };
  //@}

  //--------------------------------------------------
  /// \name Accessors
  //@{
  /** Get maximum length for this trie (read-only) */
  inline const size_t &maxlen(void) const
  { return trie_maxlen; };

  /** Get maximum length for this trie (read/write) */
  inline size_t &maxlen(void)
  { return trie_maxlen; };

  /** Check whether this trie is compiled */
  inline bool compiled(void) const
  { return !trie_pending.empty(); };

  /** Ensure that this trie is compiled */
  inline void ensure_compiled(void)
  { if (!compiled()) compile(); };

  /** Get default data (read-only) */
  inline const DataT &default_data(void) const
  { return trie_default_data; };

  /** Get default data (read-write) */
  inline DataT &default_data(void)
  { return trie_default_data; };
  //@}

  //--------------------------------------------------
  /// \name Utilties
  //@{

  /** Canonicalize case of a string @s */
  inline string_type trie_canonicalize(string_type &s) const
  {
    if (!trie_use_case) {
      for (string_iterator si = s.begin(); si != s.end(); si++) {
	*si = tolower(*si);
      }
    }
    return s;
  };

  /** Assign valid prefix-key of up to @max_len characters from @s to @dst */
  inline void trie_key(const string_type &s,
		       const size_t max_len,
		       string_type &dst)
    const
  {
    dst.assign(s,0,max_len);
    trie_canonicalize(dst);
  };

  /** Create and return a valid prefix-key of up to @max_len characters from @s */
  inline string_type trie_key(const string_type &s, const size_t max_len)
    const
  { 
    string_type key;
    trie_key(s,max_len,key);
    return key;
  };

  /** Create and return a valid prefix-key of up to @trie_maxlen characters from @s */
  inline string_type trie_key(const string_type &s) const
  { return trie_key(s,trie_maxlen); };


  /** Assign valid suffix-key of up to @max_len characters from @s to @dst */
  inline void trie_rkey(const string_type &s,
			const size_t max_len,
			string_type &dst)
    const
  {
    dst.assign(s.rbegin(), s.rbegin() + (max_len > s.size() ?
					 s.size() :
					 max_len));
    trie_canonicalize(dst);
  };

  /** Create and return a valid suffix-key of up to @max_len characters from @s */
  inline string_type trie_rkey(const string_type &s, size_t max_len) const
  {
    string_type key;
    trie_rkey(s, max_len, key);
    return key;
  };
  
  /** Create and return a valid suffix-key of up to @trie_maxlen characters from @s */
  inline string_type trie_rkey(const string_type &s) const
  { return trie_rkey(s,trie_maxlen); };
  //@}

  //--------------------------------------------------
  /// \name Insert
  //@{
  /** Insert a prefix-key into the pending-queue, giving maximum length */
  inline void trie_insert(const string_type &s, size_t max_len)
  { trie_pending[trie_key(s,max_len)] = 0; };

  /** Insert a prefix-key into the pending-queue, using trie-internal maximum length */
  inline void trie_insert(const string_type &s)
  { trie_pending[trie_key(s,trie_maxlen)] = 0; };

  /** Insert a suffix-key into the pending-queue, giving maximum length */
  inline void trie_rinsert(const string_type &s, size_t max_len)
  { trie_pending[trie_rkey(s,max_len)] = 0; };

  /** Insert a suffix-key into the pending-queue, using trie-internal maximum length */
  inline void trie_rinsert(const string_type &s)
  { trie_pending[trie_rkey(s,trie_maxlen)] = 0; };
  //@}


  //--------------------------------------------------
  /// \name Low-Level Utilities
  //@{
  /**
   * Daughter lookup, iterator, read/write
   *
   * Find the daughter of the node @from on label @lab.
   * On success, returns an iterator pointing to the daughter.
   * Returns end() if no such daughter exists.
   * \li Requires a compiled transition table.
   * \li @from should be a valid node
   */
  inline iterator find_dtr(const node_type &from, CharT label)
  {
    UCharT    dn;
    iterator  di;
    if (!trie_use_case) label = tolower(label);
    for (dn=0, di=this->begin()+from.mindtr; di != this->end() && dn < from.ndtrs; di++, dn++) {
      if (di->label == label) return di;
    }
    return this->end();
  };

  /** Daughter lookup, iterator, read-only */
  inline const_iterator find_dtr(const node_type &from, CharT label) const
  {
    UCharT         dn;
    const_iterator di;
    if (!trie_use_case) label = tolower(label);
    for (dn=0, di=this->begin()+from.mindtr; di != this->end() && dn < from.ndtrs; di++, dn++) {
      if (di->label == label) return di;
    }
    return this->end();
  };

  /** Daughter lookup, NodeId, read-only */
  inline NodeId find_dtr_id(NodeId fromid, CharT label) const
  {
    const_iterator di = find_dtr(*(this->begin()+fromid), label);
    return (di==this->end() ? NoNode : (di-this->begin()));
  };


  /**
   * Daughter iteration, read/write
   *
   * Find the first daughter of the node @from.
   * On success, returns an iterator pointing to the daughter.
   * Returns end() if no such daughter exists.
   * \li Requires a compiled transition table.
   * \li @from should be a valid node
   */
  inline iterator first_dtr(const node_type &from)
  { return ( from.ndtrs == 0 ? this->end() : (this->begin()+from.mindtr) ); };

  /** Daughter lookup, iterator, read-only */
  inline const_iterator first_dtr(const node_type &from) const
  { return ( from.ndtrs == 0 ? this->end() : (this->begin()+from.mindtr) ); };

  /**
   * Mother lookup, iterator, read/write
   *
   * Find the mother of the node @to.
   * On success, returns an iterator pointing to the mother.
   * Returns end() if @to is the root node.
   * \li Requires a compiled transition table.
   * \li @to should be a valid node
   */
  inline iterator find_mother(const node_type &to)
  { return (to.mother == NoNode ? this->end() : (this->begin()+to.mother)); };

  /** Mother lookup, iterator, read-only */
  inline const_iterator find_mother(const node_type &to) const
  { return (to.mother == NoNode ? this->end() : (this->begin()+to.mother)); };

  /** Mother lookup, NodeId, read-only */
  inline NodeId find_mother_id(NodeId toid) const
  { return (this->begin()+toid)->mother; };

  /** Get full path to node @node as a reverse string */
  inline string_type node_rstring(const node_type &node) const
  {
    if (node.mother == NoNode) return string_type();
    string_type s(1, node.label);
    const_iterator mi;
    for (mi=find_mother(node); mi != this->end() && mi->mother != NoNode; mi=find_mother(*mi)) {
      s.push_back(mi->label);
    }
    return s;
  };

  /** Get full path to node with id @nodeid as a string */
  inline string_type node_rstring(NodeId nodeid) const
  { return node_rstring(*(this->begin()+nodeid)); };

  /** Get full path to node @node as a string */
  inline string_type node_string(const node_type &node) const
  {
    string_type s = node_rstring(node);
    reverse(s.begin(),s.end());
    return s;
  };

  /** Get full path to node with id @nodeid as a string */
  inline string_type node_string(NodeId nodeid) const
  { return node_string(*(this->begin()+nodeid)); };


  /** Get depth of node @node */
  inline size_t node_depth(const node_type &node) const
  {
    size_t         depth = 0;
    const_iterator mi;
    for (mi=find_mother(node); mi != this->end() && mi->mother != NoNode; mi=find_mother(*mi)) {
      ++depth;
    }
    return depth;
  };

  /** Get depth of node with id @nodeid */
  inline size_t node_depth(NodeId nodeid) const
  { return node_depth(*(this->begin()+nodeid)); };
  //@}


  //--------------------------------------------------
  /// \name Compilation
  //@{
  /** Build transition table from the map of pending keys  */
  inline void compile(void)
  {
    vector_type::clear();
    //-- add root: (mom,mindtr,label,ndtrs,data)
    push_back(node_type(NoNode,NoNode,0,0,trie_default_data));

    map_iterator       pi;
    size_t             pos;
    bool               changed;
    char_type          dlabel;
    NodeId             dnodid;

    //-- foreach character position @pos
    for (pos=0, changed=true; pos < trie_maxlen && changed; pos++) {
      changed = false;

      //-- foreach pair *pi = (pending-key,node)
      for (pi=trie_pending.begin(); pi != trie_pending.end(); pi++) {
	const string_type &kstr   = pi->first;
	NodeId            &knodid = pi->second;
	if (kstr.size() <= pos) continue;           //-- we've exhausted this string

	dlabel           = kstr[pos];                  //-- get daughter-label
	dnodid           = find_dtr_id(knodid,dlabel); //-- check for extant daughter

	if (dnodid == NoNode) {                     //-- Ye Olde Guttes: add a daughter
	  dnodid = vector_type::size();

	  //reserve(dnodid);
	  push_back(node_type(knodid,               // (mom,
			      NoNode,               //  , mindtr
			      dlabel,               //  , label
			      0,                    //  , ndtrs
			      trie_default_data));  //  , data)

	  node_type &mnode = this->operator[](knodid);    //-- get mother-node
	  ++mnode.ndtrs;                            //-- update num/dtrs for mom

	  if (mnode.mindtr == NoNode)
	    mnode.mindtr = dnodid;                  //-- update min-dtr  for mom
	  
	  changed = true;
	}

	knodid = dnodid;                            //-- update "current" node in pending map
      }
    }
    //-- all pending arcs have been added: clear 'em
    trie_pending.clear();
  };
  //@}


  //--------------------------------------------------
  /// \name Lookup
  //@{
  /**
   * Forward lookup, read/write
   *
   * Lookup longest match (of up to @trie_maxlen characters)
   * for string @s, reading from left to right.
   * Returns a iterator pointing to the deepest matching node
   * and sets *@matchlen to the number of characters
   * actually matched.
   * Returns end() on failure.
   */
  inline iterator find_longest(const string_type &s,
			       size_t *matchlen=NULL)
  {
    const_string_iterator si;
    iterator              di, i = this->begin();
    size_t                pos;

    for (si  = s.begin() ,  di  = i        , pos=0;
	 si != s.end()                    && pos < trie_maxlen;
	 si++            ,   i  = di       , pos++)
      {
	di = find_dtr(*di,*si);
	if (di==this->end()) break;
      }
    if (matchlen) *matchlen = pos;
    return i;
  };

  /** Forward lookup, read-only */
  inline const_iterator find_longest(const string_type &s,
				     size_t *matchlen=NULL)
    const
  {
    const_string_iterator si;
    const_iterator        di, i = this->begin();
    size_t                pos;

    for (si  = s.begin() ,  di  = i        , pos=0;
	 si != s.end()                    && pos < trie_maxlen;
	 si++            ,   i  = di       , pos++)
      {
	di = find_dtr(*di,*si);
	if (di==this->end()) break;
      }
    if (matchlen) *matchlen = pos;
    return i;
  };

  /**
   * Reverse lookup, read/write
   *
   * Like find_longest(), but reads backwards from the end of @s.
   */
  inline iterator rfind_longest(const string_type &s,
				size_t *matchlen=NULL)
  {
    const_reverse_string_iterator si;
    iterator                      di, i = this->begin();
    size_t                        pos;

    for (si  = s.rbegin()  ,  di  = i        , pos=0;
	 si != s.rend()                     && pos < trie_maxlen;
	 si++              ,   i  = di       , pos++)
      {
	di = find_dtr(*di,*si);
	if (di==this->end()) break;
      }
    if (matchlen) *matchlen = pos;
    return i;
  };

  /** Reverse lookup, read-only */
  inline const_iterator rfind_longest(const string_type &s,
				      size_t *matchlen=NULL)
    const
  {
    const_reverse_string_iterator si;
    const_iterator                di, i = this->begin();
    size_t                        pos;

    for (si  = s.rbegin()  ,  di  = i        , pos=0;
	 si != s.rend()                     && pos < trie_maxlen;
	 si++              ,   i  = di       , pos++)
      {
	di = find_dtr(*di,*si);
	if (di==this->end()) break;
      }
    if (matchlen) *matchlen = pos;
    return i;
  };
  //@}

#ifdef MOOT_TRIE_VECTOR_DEBUG
  //--------------------------------------------------
  /// \name Dump
  //@{
  /** Dump the current contents of the trie to a C stream,
   *  using @delim as a delmiter
   */
  void dump(FILE *out, const CharT delim=0)
  {
    const_iterator   i, mi;
    for (i = this->begin(); i != this->end(); i++) {
      string_type s = node_rstring(*i);
      if (s.empty()) continue;
      s.push_back(delim);
      fwrite(s.data(), sizeof(CharT), s.size(), out);
    }
  };

  /** Dump binary table to a file */
  void bindump(FILE *out) {
    for (const_iterator i=this->begin(); i != this->end(); i++) {
      fwrite(&(*i), sizeof(node_type), 1, out);
    }
  };

  /** Dump arc table to a file */
  void arcdump(FILE *out) {
    for (const_iterator i=this->begin(); i != this->end(); i++) {
      fprintf(out,"node=%u\t mom=%u\t mindtr=%u\t label=%c\t ndtrs=%u\n",
	      i-this->begin(), i->mother, i->mindtr, i->label, i->ndtrs);
    }
  };
  //@}
#endif //-- /MOOT_TRIE_VECTOR_DEBUG
};

}; //-- /namespace moot

#endif //-- MOOT_TRIE_VECTOR_H
