/* -*- Mode: C++ -*- */

/*
   libmoot : moocow's part-of-speech tagging library
   Copyright (C) 2003-2017 by Bryan Jurish <moocow@cpan.org>

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

/*============================================================================
 * File: mootEnum.h
 * Author:  Bryan Jurish <moocow@cpan.org>
 * Description:
 *    Templates & classes for runtime enumerations (Identifier<->unsigned maps)
 *============================================================================*/

/**
\file mootEnum.h
\brief runtime enumerations (1-1 maps: symbolic identifiers \<-\> unsigned integers)
*/

#ifndef _moot_ENUM_H
#define _moot_ENUM_H

#include <mootTypes.h>

using namespace std;
using namespace moot_STL_NAMESPACE;

/**
 * Value type for runtime enumerations.  The value '0' (zero) is reserved.
 */
typedef moot::UInt mootEnumID;

/**
 * Constant indicating failed ID lookup.
 */
const mootEnumID mootEnumNone = 0;

/**
 * Template class for runtime enumerations (symbolic_name<->unsigned_int maps)
 * using STL hash_map<> and vector<>.
 *
 * Currently just supports monotonically growing enumerations:
 * no support for single-item removal or re-assignment.
 *
 * Suggests: default constructor for NameType (for default mootEnum constructor).
 *
 * Requires: g++ flag "-ftemplate-depth-NN", where NN >= 19 for compilation
 * with gcc-2.95.4 using STLport-4.5.3.
 */
template <class NameType,
	  class NameHashFcn  = moot_hash <NameType>,
	  class NameEqualFcn = equal_to <NameType>  >
class mootEnum {
public:
  //------ public typedefs
  /** Type which maps symbolic names to numeric IDs */
  typedef hash_map<NameType,mootEnumID,NameHashFcn,NameEqualFcn> Name2IdMap;

  /** Type which maps numeric IDs back to symbolic names */
  typedef vector<NameType>                                       Id2NameMap;

public:
  //------ public data
  Name2IdMap names2ids;  ///< maps names to IDs
  Id2NameMap ids2names;  ///< maps IDs to names

public:
  /** Default constructor */
  mootEnum(void)
  {
    unknown_name(NameType());
  };
  
  /** Default constructor */
  mootEnum(const NameType &unknownName)
  {
    unknown_name(unknownName);
  };

  /** Default destructor */
  ~mootEnum(void)
  {
    /*clear();*/
  };

  //------ access
  /** Set the 'unknown' name */
  inline void unknown_name(const NameType &name)
  {
    if (ids2names.empty()) ids2names.resize(1);
    ids2names[0]    = name;
    names2ids[name] = 0;
  };

  //------ sanity checking
  /** Check whether 'name' is a registered name. */
  inline bool nameExists(const NameType &name) const
  {
    return names2ids.find(name) != names2ids.end();
  };

  /** Check whether 'id' is a registered id. */
  inline bool idExists(const mootEnumID id) const
  {
    return id && ids2names.size() > id;
  };


  //------ access
  /** Return maximum id +1 */
  inline mootEnumID size(void) const
  {
    return ids2names.size();
  };

  /** Lookup ID for name, returns 0 if name is unregistered. */
  inline mootEnumID name2id(const NameType &name) const
  {
    typename Name2IdMap::const_iterator i = names2ids.find(name);
    return i == names2ids.end() ? 0 : i->second;
  };

  /**
   * Lookup name for id.  Returns the "unknown" name
   * if id is unregistered.
   */
  inline const NameType &id2name(const mootEnumID id) const
  {
    return ids2names.size() <= id ? ids2names[0] : ids2names[id];
  };

  //------ manipulation

  /**
   * Insert a new name<->id mapping.
   * If id is unspecified or zero, a new unique id will be assigned.
   * Returns the (new) id.
   */
  inline mootEnumID insert(const NameType &name, mootEnumID id=0)
  {
    if (!id) id = ids2names.size();
    if (ids2names.size() <= id) ids2names.resize(id+1);
    ids2names[id] = name;
    names2ids[name] = id;
    return id;
  };

  /**
   * Remove a name<->id mapping.
   * Effectively re-maps \c name to mootEnumNone==0 and \c id to unknown_name()
   * Will never shrink the object.
   */
  inline void remove(const NameType &name, mootEnumID id=mootEnumNone)
  {
    names2ids.erase(name);
    if (id != mootEnumNone) ids2names[id] = ids2names[mootEnumNone];
  };

  /** Remove a name<->id mapping, given only id */
  inline void remove(mootEnumID id)
  {
    remove(ids2names[id],id);
  };


  /**
   * Get ID for name, creating one if it doesn't already exist.
   */
  inline mootEnumID get_id(const NameType &name)
  {
    typename Name2IdMap::const_iterator i = names2ids.find(name);
    return i == names2ids.end() ? insert(name,0) : i->second;
  };

  /** Clears all mappings, but retains "unknown" name */
  inline void clear(void)
  {
    names2ids.clear();
    //--
    ids2names.resize(1); //-- keep "unknown" name
    //--
    //ids2names.clear();     //-- clear *everything*
  };

  /** Resize the object, leaving only \c newsize-1 IDs intact.
   *  Increasing object size may leave the object in an inconsistent state.
   */
  void resize(size_t newsize)
  {
    if (newsize < 1) newsize=1;  //-- minimum size = 1 (always keep "unknown")
    if (newsize > size()) {      //-- grow (only ids2names)
      ids2names.resize(newsize);
    }
    else if (newsize < size()) { //-- shrink
      for (mootEnumID id = newsize; id < size(); id++) {
	names2ids.erase(ids2names[id]);
      }
      ids2names.resize(newsize);
    }
  };
};

#endif /* _moot_ENUM_H */
