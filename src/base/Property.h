// Some utility templates for emulating
// properties - preferring a library solution
// to a new language feature
// Each property has three sets of redundant
// acccessors:
// 1. function call syntax
// 2. get() and set() functions
// 3. overloaded operator =

// a read-write property with data store and
// automatically generated get/set functions.
// this is what C++/CLI calls a trivial scalar
// property
template <class T>
class Property {
  T data;
public:

  // access with function call syntax
  Property() : data() { }
  T operator()() const {
    return data;
  }
  T operator()(T const & value) {
    data = value;
    return data;
  }

  // access with get()/set() syntax
  T get() const {
    return data;
  }
  T set(T const & value) {
    data = value;
    return data;
  }

  // access with '=' sign
  // in an industrial-strength library,
  // specializations for appropriate types
  // might choose to add combined operators
  // like +=, etc.
  operator T() const {
    return data;
  }
  T operator=(T const & value) {
    data = value;
    return data;
  }
  typedef T value_type;
            // might be useful for template
            // deductions
};
