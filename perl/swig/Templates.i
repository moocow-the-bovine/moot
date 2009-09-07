/* -*- Mode: C++ -*- */
/*
 * File: Templates.i
 * Author: Bryan Jurish
 * Description: swig interface to moot: generic template stuff
 */

%{
  using namespace moot;
%}

//-- Suppress SWIG warning
#pragma SWIG nowarn=SWIGWARN_PARSE_NESTED_CLASS

// Language independent exception handlers
//%include exception.i

/*----------------------------------------------------------------------
 * std::list<T>
 */
//%include <std_list.i> //-- defined, but not as extensive
template<class T> class std::list {
 public:
  list(void);
  ~list(void);
  void clear(void);
  //
  size_t size(void);
  bool empty(void);
  //
  %extend {
    T &front(void) {
      if ($self->empty()) {
	//SWIG_Error(SWIG_IndexError,"attempt to index non-existent list element"); //--nope
	//SWIG_croak("attempt to access non-existent list element"); //--nope
	croak("attempt to access non-existent list element");
      }
      return $self->front();
    };
    T &back(void) {
      if ($self->empty()) {
	//SWIG_Error(SWIG_IndexError,"attempt to index non-existent list element"); //--nope
	//SWIG_croak("attempt to access non-existent list element"); //--nope
	croak("attempt to access non-existent list element");
      }
      return $self->back();
    };
    list *rotate(int n=1) {
      if ($self->empty()) return $self;
      for (; n>0; n--) {
	$self->push_back($self->front());
	$self->pop_front();
      }
      for (; n<0; n++) {
	$self->push_front($self->back());
	$self->pop_back();
      }
      return $self;
    };
  }
  //
  void push_front(const T &elt);
  void push_back(const T &elt);
  void pop_front(void);
  void pop_back(void);
};


/*----------------------------------------------------------------------
 * std::vector<T>
 *  + TODO: iterators, etc.
 *  + already implemented? (/usr/share/swig1.3/perl5/std_vector.i)
 */

template<class T> class std::vector<T> {
 public:
  vector(void);
  vector(size_t n);
  ~vector(void);
  //
  void clear(void);
  void reserve(size_t n);
  void resize(size_t n);
  //
  size_t size(void);
  bool empty(void);
  //
  %extend {
    T &front(void) {
      if ($self->empty()) {
	//SWIG_Error(SWIG_IndexError,"attempt to index non-existent vector element"); //--nope
	//SWIG_croak("attempt to access non-existent vector element"); //--nope
	croak("attempt to access non-existent vector element");
      }
      return $self->front();
    };
    T &back(void) {
      if ($self->empty()) {
	//SWIG_Error(SWIG_IndexError,"attempt to index non-existent vector element"); //--nope
	//SWIG_croak("attempt to access non-existent vector element"); //--nope
	croak("attempt to access non-existent vector element");
      }
      return $self->back();
    };
    T &nth(size_t n) {
      return (*$self)[n];
    };
  }
  //
  //void push_front(const T &elt);
  void push_back(const T &elt);
  //void pop_front(void);
  void pop_back(void);
};

//%include <std_vector.i>
%define extend_std_vector(T)
 /*
namespace std {
  %extend vector<T> {
    void reserve(size_t n) { $self->reserve(n); };
    void resize(size_t n)  { $self->resize(n); };
  };
}
 */
%enddef
 /*-- test
   %template(intVector) std::vector<int>;
   extend_std_vector(int);
 */
 //extend_std_vector(T);

/*----------------------------------------------------------------------
 * std::set<T>
 *  + TODO: iterators, find, ...
 */
template<class T> class std::set {
 public:
  set(void);
  ~set(void);
  void clear(void);
  //
  size_t size(void);
  bool empty(void);
};

/*----------------------------------------------------------------------
 * std::map<KeyT,ValT>
 *  + TODO: iterators, find, ...
 */
template<class KeyT, class ValT> class std::map {
 public:
  map(void);
  ~map(void);
  void clear(void);
  //
  size_t size(void);
  bool empty(void);
};

/*----------------------------------------------------------------------
 * std::pair<T1,T2>
 */
template<class T1, class T2> class std::pair {
public:
  T1 first;
  T2 second;
public:
  pair(void);
  pair(T1 x1, T2 x2);
  pair(std::pair<T1,T2> p1);
  ~pair(void);
};
