// a read-write property which invokes
// user-defined functions
template <class T,
          class Object,
          T (Object::*real_getter)(),
          T (Object::*real_setter)(T const &)>
class RWProperty {
  Object * my_object;
public:
  RWProperty() : my_object(0) {}
  RWProperty(Object * me)
               : my_object(me) {}

  // this function must be called by the
  // containing class, normally in a
  // constructor, to initialize the
  // ROProperty so it knows where its
  // real implementation code can be
  // found
  void operator()(Object * obj) {
    my_object = obj;
  }

  // function call syntax
  T operator()() const {
    return (my_object->*real_getter)();
  }
  T operator()(T const & value) {
    return (my_object->*real_setter)(value);
  }

  // get/set syntax
  T get() const {
    return (my_object->*real_getter)();
  }
  T set(T const & value) {
    return (my_object->*real_setter)(value);
  }
  // access with '=' sign
  operator T() const {
    return (my_object->*real_getter)();
  }
  T operator=(T const & value) {
    return (my_object->*real_setter)(value);
  }

  typedef T value_type;
            // might be useful for template
            // deductions
};
