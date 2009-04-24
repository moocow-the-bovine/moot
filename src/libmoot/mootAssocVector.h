/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2009 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*--------------------------------------------------------------------------
 * File: mootAssocVector.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : LISP-style assoc vectors
 *--------------------------------------------------------------------------*/

/** \file mootAssocVector.h
 *  \brief LISP-style assoc vectors
 */

#ifndef MOOT_ASSOC_VECTOR_H
#define MOOT_ASSOC_VECTOR_H

#include <vector>
#include <algorithm>

namespace moot {

  //======================================================================
  // AssocVectorNode
  /// \brief template class for individual AssocVector nodes
  template<typename KeyT, typename ValT>
  struct AssocVectorNode
    : public std::pair<KeyT,ValT>
  {
    //----------------------------------------------------------
    // Node: types
    typedef KeyT                        key_type;
    typedef ValT                        value_type;
    typedef std::pair<KeyT,ValT>        PairT;
    typedef AssocVectorNode<KeyT,ValT>  ThisT;

    //----------------------------------------------------------
    // Node: Data
    //KeyT  node_key;
    //ValT  node_val;

    //----------------------------------------------------------
    // Node: Methods

    //--------------------------------------------------
    /// \name Constructors etc.
    //@{
    /** Default constructor */
    inline AssocVectorNode(void)
      : PairT()
    {};

    /** Constructor given key only */
    inline AssocVectorNode(const KeyT &key)
      : PairT(key,ValT())
    {};

    /** Constructor given key & data */
    inline AssocVectorNode(const KeyT &key, const ValT &val)
      : PairT(key,val)
    {};

    /** Copy constructor */
    inline AssocVectorNode(const ThisT &x)
      : PairT(x.first,x.second)
    {};

    /** Destructor */
    ~AssocVectorNode(void) {};
    //@}

    //--------------------------------------------------
    /// \name Accessors
    //@{
    inline       key_type   &key   (void)       { return this->first; };
    inline const key_type   &key   (void) const { return this->first; };

    inline       value_type &value (void)       { return this->second; };
    inline const value_type &value (void) const { return this->second; };
    //@}

    //--------------------------------------------------
    /// \name Operators
    //@{
    inline bool operator<(const ThisT &x) const
    {
      return (this->first < x.first
	      ? true
	      : (this->first > x.first
		 ? false
		 : this->second < x.second));
    };
    //@}
  }; //-- /AssocVectorNode


  //======================================================================
  // AssocVector

  /** \brief LISP-style assoc list using vector<>:
   *  map-like class with small memory footprint.
   *  Useful for small associative arrays.
   *  Lookup and insert are linear time.
   *
   * Requirements for KeyT
   * \li should support operator== and copy constructor
   *
   * Requirements for ValT
   * \li should support operator==(), copy constructor, and operator=
   */
  template<typename KeyT, typename ValT>
  class AssocVector  : public std::vector<AssocVectorNode<KeyT,ValT> >
  {
  public:
    //--------------------------------------------------------------------
    // AssocVector: Types
    typedef KeyT                                                  assoc_key_type;
    typedef ValT                                                  assoc_value_type;
    typedef AssocVectorNode<KeyT,ValT>                            assoc_node_type;
    typedef std::vector<assoc_node_type>                          assoc_vector_type;

    typedef typename assoc_vector_type::iterator                  iterator;
    typedef typename assoc_vector_type::const_iterator            const_iterator;

    typedef typename assoc_vector_type::reverse_iterator          reverse_iterator;
    typedef typename assoc_vector_type::const_reverse_iterator    const_reverse_iterator;

  public:
    //--------------------------------------------------------------------
    // AssocVector: Utility Types

    /** Pseudo-function for sort_byvalue() */
    struct value_sort_func {
      inline bool operator()(const assoc_node_type &x, const assoc_node_type &y) const
      {
	return (x.second < y.second
		? true
		: (x.second > y.second
		   ? false
		   : x.first < y.first));
      };
    };

  public:
    //--------------------------------------------------------------------
    // AssocVector: Data
    //(empty)

  public:
    //--------------------------------------------------------------------
    // AssocVector: Methods

    //--------------------------------------------------
    /// \name Constructors etc.
    //@{
    /** Default constructor */
    AssocVector(void) 
      : assoc_vector_type()
    {};

    /** Constructor given size */
    inline AssocVector(const size_t mysize)
    {
      this->reserve(mysize);
    };

    /** Destructor */
    ~AssocVector(void) {};
    //@}

    //--------------------------------------------------
    /// \name Iterator Utilities
    //@{
    /** Return a read/write iterator pointing to the node for \p key,
     *  or end() if no such node exists */
    inline iterator find(const KeyT &key)
    {
      iterator i;
      for (i = this->begin(); i != this->end() && i->first != key; i++) ;
      return i;
    };

    /** Return a read-only iterator pointing to the node for \p key,
     *  or \c end() if no such node exists */
    inline const_iterator find(const KeyT &key) const
    {
      const_iterator i;
      for (i = this->begin(); i != this->end() && i->first != key; i++) ;
      return i;
    };

    /** Return a read/write iterator pointing a
     *  node for \p key, adding such a node if not already present */
    inline iterator get(const KeyT &key)
    {
      iterator i = find(key);
      return (i == this->end() ? assoc_vector_type::insert(i,key) : i);
    };

    /**
     * Insert association \p key => \p value into the list.
     * Returns an iterator pointing to the (possibly new)
     * association.
     */
    inline iterator insert(const KeyT &key, const ValT &val)
    {
      iterator i  = get(key);
      i->second = val;
      return i;
    };
    //@}

    //--------------------------------------------------
    /// \name Get/Insert
    //@{
    /**
     * Get assocation for \p key , possibly creating a new one.
     * Returns a reference to the (new) node.
     */
    inline assoc_node_type &get_node(const KeyT &key)
    { return *(get(key)); };

    /** Alias for get_node(key)->second */
    inline assoc_value_type &get_value(const KeyT &key)
    { return get_node(key).second; };

    /** Get/insert the \p n th node */
    inline assoc_node_type &nth(const size_t n)
    { return assoc_vector_type::operator[](n); };
    //@}

    //--------------------------------------------------
    /// \name Operators
    //@{
    /** Alias for get_data(key) */
    inline assoc_value_type &operator[](const KeyT &key)
    { return get_node(key).second; };

    /** Alias for find_data(key) */
    inline const assoc_value_type &operator[](const KeyT &key) const
    { return find(key)->second; };
    //@}

    //--------------------------------------------------
    /// \name Sorting
    //@{
    /** Sort contents in ascending order by key,value */
    inline void sort_bykey(void)
    { std::sort(this->begin(),this->end()); };

    /** Sort contents in descending order by key,value */
    inline void rsort_bykey(void)
    { std::sort(this->rbegin(),this->rend()); };

    /** Sort contents in ascending order by value,key */
    inline void sort_byvalue(void)
    { std::sort(this->begin(),this->end(),value_sort_func()); };

    /** Sort contents in descending order by value,key */
    inline void rsort_byvalue(void)
    { std::sort(this->rbegin(),this->rend(),value_sort_func()); };
    //@}

  }; //-- /AssocVector

}; //-- /namespace moot

#endif // MOOT_ASSOC_VECTOR_H
