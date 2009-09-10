/* -*- Mode: C++ -*- */
/*
 * File: AssocVector.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: mootAssocVector<Key,Val>
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS

template<typename KeyT, typename ValT> struct AssocVectorNode : public std::pair<KeyT,ValT> {};

template<typename KeyT, typename ValT> class AssocVector {
public:
  AssocVector(void);
  AssocVector(size_t mysize);
  ~AssocVector(void);
  //
  void clear(void);
  size_t size(void) const;
  //
  %extend {
    int find_index(const KeyT &key) const {
      AssocVector<KeyT,ValT >::const_iterator ci = $self->find(key);
      return (ci == $self->end() ? -1 : (ci-$self->begin()));
    };
  }
  std::pair<KeyT,ValT > &nth(size_t n);
  std::pair<KeyT,ValT > &get_node(const KeyT &key);
  void insert(const KeyT &key, const ValT &val);
};

//-- instantiate a la mootHMM::LexProbSubTable: < mootHMM::TagID~mootEnumID, ProbT >
%template(IdProbAssocVectorNode) std::pair<mootEnumID,ProbT>;
%template(IdProbAssocVector)     AssocVector<mootEnumID,ProbT>;
