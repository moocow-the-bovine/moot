/* -*- Mode: C++ -*- */
/*
 * File: Enum.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: mootEnum
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS


/*----------------------------------------------------------------------
 * Basic Types & Constants
 */
typedef unsigned int mootEnumID;
extern const mootEnumID mootEnumNone;

/*----------------------------------------------------------------------
 * Classes
 */

template <class NameType,
	  class NameHashFcn  = hash     <NameType>,
	  class NameEqualFcn = equal_to <NameType>  >
class mootEnum {
public:
  /** Default constructor */
  mootEnum(void);
  mootEnum(const NameType &unknownName);
  ~mootEnum(void);
  void clear(void);
  void unknown_name(const NameType &name);
  bool nameExists(const NameType &name) const;
  bool idExists(const mootEnumID id) const;
  mootEnumID size(void) const;
  mootEnumID name2id(const NameType &name) const;
  const NameType &id2name(const mootEnumID id) const;
  mootEnumID insert(const NameType &name, mootEnumID id=0);
  %extend {
    const NameType &unknown_name(void) { return $self->id2name(mootEnumNone); }
  }
};

%template(StringEnum) mootEnum<std::string>;
//%template(TokStringEnum) mootEnum<mootTokString>;
//%template(TagStringEnum) mootEnum<mootTagString>;
