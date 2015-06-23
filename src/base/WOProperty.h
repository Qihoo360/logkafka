// a write-only property calling a
// user-defined setter
template <class T, class Object,
          T (Object::*real_setter)(T const &)>
class WOProperty {
  Object * my_object;
public:
  WOProperty() : my_object(0) {}
  WOProperty(Object * me)
               : my_object(me) {}

  // this function must be called by the
  // containing class, normally in a
  // constructor, to initialize the
  // WOProperty so it knows where its real
  // implementation code can be found
  void operator()(Object * obj) {
    my_object = obj;
  }
  // function call syntax
  T operator()(T const & value) {
    return (my_object->*real_setter)(value);
  }
  // get/set syntax
  T get() const;
            // reserved but not implemented,
            // per C++/CLI
  T set(T const & value) {
    return (my_object->*real_setter)(value);
  }

  // access with '=' sign
  T operator=(T const & value) {
    return (my_object->*real_setter)(value);
  }

  typedef T value_type;
            // might be useful for template
            // deductions
};
