/* -*- Mode: C++ -*- */

/*============================================================================
 * File: dwdstEnum.h
 * Author:  Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *    Templates & classes for runtime enumerations (Identifier<->unsigned maps)
 *============================================================================*/

#ifndef _DWDST_ENUM_H
#define _DWDST_ENUM_H

#include <hash_map>
#include <vector>

using namespace std;

/**
 * Value type for runtime enumerations.  The value '0' (zero) is reserved.
 */
typedef unsigned int dwdstEnumID;

/**
 * Constant indicating failed ID lookup.
 */
const dwdstEnumID dwdstEnumNone = 0;

/**
 * Template class for runtime enumerations (symbolic_name<->unsigned_int maps)
 * using STL hash_map<> and vector<>.
 *
 * Currently just supports monotonically growing enumerations:
 * no support for single-item removal or re-assignment.
 *
 * Suggests: default constructor for NameType (for default dwdstEnum constructor).
 *
 * Requires: g++ flag "-ftemplate-depth-NN", where NN >= 19 for compilation
 * with gcc-2.95.4 using STLport-4.5.3.
 */
template <class NameType, class NameHashFcn, class NameEqualFcn>
class dwdstEnum {
public:
  //------ public typedefs
  /** Type which maps symbolic names to numeric IDs */
  typedef hash_map<NameType,dwdstEnumID,NameHashFcn,NameEqualFcn> Name2IdMap;

  /** Type which maps numeric IDs back to symbolic names */
  typedef vector<NameType>                                        Id2NameMap;

public:
  //------ public data
  Name2IdMap names2ids;  ///< maps names to IDs
  Id2NameMap ids2names;  ///< maps IDs to names

public:
  /** Default constructor */
  dwdstEnum(void)
  {
    ids2names.push_back(NameType());
  };
  
  /** Default constructor */
  dwdstEnum(const NameType &unknownName)
  {
    ids2names.push_back(unknownName);
  };

  /** Default destructor */
  ~dwdstEnum(void)
  {
    clear();
  };

  //------ access
  /** Set the 'unknown' name */
  inline void unknown_name(const NameType &name)
  {
    names2ids[name] = 0;
    ids2names[0] = name;
  };

  //------ sanity checking
  /** Check whether 'name' is a registered name. */
  inline bool nameExists(const NameType &name) const
  {
    return names2ids.find(name) != names2ids.end();
  };

  /** Check whether 'id' is a registered id. */
  inline bool idExists(const dwdstEnumID id) const
  {
    return id && ids2names.size() > id;
  };


  //------ access
  /** Return maximum id */
  inline dwdstEnumID size(void) const
  {
    return ids2names.size();
  };

  /** Lookup ID for name, returns 0 if name is unregistered. */
  inline dwdstEnumID name2id(const NameType &name) const
  {
    Name2IdMap::const_iterator i = names2ids.find(name);
    return i == names2ids.end() ? 0 : i->second;
  };

  /**
   * Lookup name for id.  Returns the "unknown" name
   * if id is unregistered.
   */
  inline const NameType &id2name(const dwdstEnumID id) const
  {
    return ids2names.size() <= id ? ids2names[0] : ids2names[id];
  };

  //------ manipulation

  /**
   * Insert a new name<->id mapping.
   * If id is unspecified or zero, a new unique id will be assigned.
   * Returns the (new) id.
   */
  inline dwdstEnumID insert(const NameType &name, dwdstEnumID id=0)
  {
    if (!id) id = ids2names.size();
    if (ids2names.size() <= id) ids2names.resize(id+1);
    ids2names[id] = name;
    names2ids[name] = id;
    return id;
  };

  /** Clears all mappings, but retains "unknown" name. */
  inline void clear(void)
  {
    names2ids.clear();
    ids2names.resize(1);  // keep "unknown" name
  };
};

#endif /* _DWDST_ENUM_H */
