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
 * File: mootAssocVector.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + moocow's PoS tagger : LISP-style assoc vectors
 *--------------------------------------------------------------------------*/

#ifndef MOOT_ASSOC_VECTOR_H
#define MOOT_ASSOC_VECTOR_H

#include <vector>

namespace moot {

  //======================================================================
  // AssocVectorNode
  /// \brief template class for individual AssocVector nodes
  template<typename KeyT, typename ValT>
  struct AssocVectorNode {
    //----------------------------------------------------------
    // Node: types
    typedef KeyT                        key_type;
    typedef ValT                        value_type;
    typedef AssocVectorNode<KeyT,ValT>  ThisT;

    //----------------------------------------------------------
    // Node: Data
    KeyT  node_key;
    ValT  node_val;

    //----------------------------------------------------------
    // Node: Methods

    //--------------------------------------------------
    /// \name Constructors etc.
    //@{
    /** Default constructor */
    inline AssocVectorNode(void) {};

    /** Constructor given key only */
    inline AssocVectorNode(const KeyT &key)
      : node_key(key)
    {};

    /** Constructor given key & data */
    inline AssocVectorNode(const KeyT &key, const ValT &val)
      : node_key(key), node_val(val)
    {};

    /** Copy constructor */
    inline AssocVectorNode(const ThisT &x)
      : node_key(x.node_key), node_val(x.node_val)
    {};

    /** Destructor */
    inline ~AssocVectorNode(void) {};
    //@}

    //--------------------------------------------------
    /// \name Accessors
    //@{
    inline       key_type   &key   (void)       { return node_key; };
    inline const key_type   &key   (void) const { return node_key; };

    inline       value_type &value (void)       { return node_val; };
    inline const value_type &value (void) const { return node_val; };
    //@}

    //--------------------------------------------------
    /// \name Operators
    //@{
    inline bool operator==(const ThisT &x) const
    { return node_key==x.node_key && node_val==x.node_val; };

    inline bool operator<(const ThisT &x) const
    {
      return (node_key < x.node_key
	      ? true
	      : (node_key > x.node_key
		 ? false
		 : node_val < x.node_val));
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
    // AssocVector: Data
    //(empty)

  public:
    //--------------------------------------------------------------------
    // AssocVector: Methods

    //--------------------------------------------------
    /// \name Constructors etc.
    //@{
    /** Default constructor */
    inline AssocVector(void) {};

    /** Constructor given size */
    inline AssocVector(const size_t mysize)
    {
      reserve(mysize);
    };

    /** Destructor */
    inline ~AssocVector(void) {};
    //@}

    //--------------------------------------------------
    /// \name Iterator Utilities
    //@{
    /** Return a read/write iterator pointing to the node for @key,
     *  or end() if no such node exists */
    inline iterator find(const KeyT &key)
    {
      iterator i;
      for (i = begin(); i != end() && i->node_key != key; i++) ;
      return i;
    };

    /** Return a read-only iterator pointing to the node for @key,
     *  or end() if no such node exists */
    inline const_iterator find(const KeyT &key) const
    {
      const_iterator i;
      for (i = begin(); i != end() && i->node_key != key; i++) ;
      return i;
    };

    /** Return a read/write iterator pointing a
     *  node for @key, adding such a node if not already present */
    inline iterator get(const KeyT &key)
    {
      iterator i = find(key);
      return (i == end() ? assoc_vector_type::insert(i,key) : i);
    };

    /**
     * Insert association @key => @value into the list.
     * Returns an iterator pointing to the (possibly new)
     * association.
     */
    inline iterator insert(const KeyT &key, const ValT &val)
    {
      iterator i  = get(key);
      i->node_val = val;
      return i;
    };
    //@}

    //--------------------------------------------------
    /// \name Get/Insert
    //@{
    /**
     * Get assocation for @key , possibly creating a new one.
     * Returns a reference to the (new) node.
     */
    inline assoc_node_type &get_node(const KeyT &key)
    { return *(get(key)); };

    /** Alias for get_node(key)->node_val */
    inline assoc_value_type &get_value(const KeyT &key)
    { return get_node(key).node_val; };

    /** Get/insert the @n th node */
    inline assoc_node_type &nth(const size_t n)
    { return assoc_vector_type::operator[](n); };
    //@}

    //--------------------------------------------------
    /// \name Operators
    //@{
    /** Alias for get_data(key) */
    inline assoc_value_type &operator[](const KeyT &key)
    { return get_node(key).node_val; };

    /** Alias for find_data(key) */
    inline const assoc_value_type &operator[](const KeyT &key) const
    { return find(key)->node_val; };
    //@}

  }; //-- /AssocVector

}; //-- /namespace moot

#endif // MOOT_ASSOC_VECTOR_H
