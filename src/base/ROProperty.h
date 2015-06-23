// a read-only property calling a
// user-defined getter
template <typename T, typename Object,
          T (Object::*real_getter)()>
class ROProperty {
  Object * my_object;
public:
  ROProperty() : my_object(0) {}
  ROProperty(Object * me)
               : my_object(me) {}

  // this function must be called by the
  // containing class, normally in a
  // constructor, to initialize the
  // ROProperty so it knows where its
  // real implementation code can be
  // found.
  // obj is usually the containing
  // class, but need not be; it could be a
  // special implementation object.
  void operator()(Object * obj) {
    my_object = obj;
  }

  // function call syntax
  T operator()() const {
    return (my_object->*real_getter)();
  }

  // get/set syntax
  T get() const {
    return (my_object->*real_getter)();
  }
  void set(T const & value);
            // reserved but not implemented,
            // per C++/CLI

  // use on rhs of '='
  operator T() const {
    return (my_object->*real_getter)();
  }

  typedef T value_type;
            // might be useful for template
            // deductions
};

