// a read/write property providing indexed
// access.
// this class simply encapsulates a std::map
// and changes its interface to functions
// consistent with the other property<>
// classes.
// note that the interface combines certain
// limitations of std::map with
// some others from indexed properties as
// I understand them.
// an example of the first is that
// operator[] on a map will insert a
// key/value pair if it isn't already there.
// A consequence of this is that it can't
// be a const member function (and therefore
// you cannot access a const map using
// operator [].)
// an example of the second is that indexed
// properties do not appear to have any
// facility for erasing key/value pairs
// from the container.
// C++/CLI properties can have
// multi-dimensional indexes: prop[2,3].
// This is not allowed by the current rules
// of standard C++
#include <map>
template <class Key,
          class T,
          class Compare = std::less<Key>,
          class Allocator
               = std::allocator<std::pair<
                           const Key, T> > >
class IndexedProperty {
  std::map<Key, T, Compare,
           Allocator> data;
  typedef typename std::map<Key, T, Compare,
                        Allocator>::iterator
          map_iterator;
public:

  // function call syntax
  T operator()(Key const & key) {
    std::pair<map_iterator, bool> result;
    result
      = data.insert(std::make_pair(key, T()));
    return (*result.first).second;
  }
  T operator()(Key const & key,
               T const & t) {
    std::pair<map_iterator, bool> result;
    result
      = data.insert(std::make_pair(key, t));
    return (*result.first).second;
  }

  // get/set syntax
  T get_Item(Key const & key) {
    std::pair<map_iterator, bool> result;
    result
      = data.insert(std::make_pair(key, T()));
    return (*result.first).second;
  }
  T set_Item(Key const & key,
             T const & t) {
    std::pair<map_iterator, bool> result;
    result
      = data.insert(std::make_pair(key, t));
    return (*result.first).second;
  }

  // operator [] syntax
  T& operator[](Key const & key) {
    return (*((data.insert(make_pair(
                   key, T()))).first)).second;
  }
};
