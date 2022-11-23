#ifndef  RARITY_ARRAY_HPP
# define RARITY_ARRAY_HPP

# include "object.hpp"
# include <memory>
# include <vector>
# include <list>
# include <set>

namespace Ruby
{
  template<typename TYPE>
  std::shared_ptr<IRarityClass> to_ruby_type(TYPE&);

  class Array : public Object
  {
  public:
    struct iterator
    {
      iterator(VALUE instance, long position = 0) : position(position), instance(instance) {}

      Ruby::Object operator*() const { return rb_ary_entry(instance, position); }
      Ruby::Object operator->() const { return Ruby::Object(**this); }

      iterator& operator++() { position++; return *this; }
      iterator& operator++(int i) { position += i; return *this; }
      bool operator==(const iterator& other) const;
      bool operator!=(const iterator& other) const { return !(*this == other); }

      long position = 0;
      VALUE instance;
    };

    Array(void)       : Object(rb_ary_new()) {}
    Array(VALUE copy) : Object(copy)         {}

    template<typename TYPE>
    Array(const std::vector<TYPE>& array)
    {
      instance = rb_ary_new();
      insert(begin(), array.begin(), array.end());
    }

    template<typename ITERATOR>
    Array(ITERATOR it, const ITERATOR end)
    {
      instance = rb_ary_new();
      insert(begin(), it, end);
    }

    std::size_t size() const { return rb_array_len(instance); }
    Object operator[](int index);
    iterator begin() const { return iterator(instance); }
    iterator end() const { return iterator(instance, -1); }
    iterator erase(iterator at);
    iterator insert(iterator at, VALUE value);

    template<typename ITERATOR>
    iterator insert(iterator at, ITERATOR it, ITERATOR end)
    {
      while (it != end)
      {
        auto tmp = Ruby::to_ruby_type(*it);

        insert(at, tmp->ruby_instance());
        ++it;
        ++at;
      }
      return at;
    }

    void push_back(VALUE value)
    {
      rb_ary_push(instance, value);
    }

    void push_front(VALUE value)
    {
      rb_ary_store(instance, 0, value);
    }

    template<typename TYPE>
    std::vector<TYPE> as_vector() const
    {
      std::vector<TYPE> vector;

      vector.reserve(size());
      for (auto it = begin() ; it != end() ; ++it)
        vector.push_back(to_cpp_type<TYPE>(*it));
      return vector;
    }

    template<typename TYPE>
    std::list<TYPE> as_list() const
    {
      std::list<TYPE> list;

      for (auto it = begin() ; it != end() ; ++it)
        list.push_back(to_cpp_type<TYPE>(*it));
      return list;
    }

    template<typename TYPE>
    std::set<TYPE> as_set() const
    {
      std::set<TYPE> set;

      for (auto it = begin() ; it != end() ; ++it)
        set.push_back(to_cpp_type<TYPE>(*it));
      return set;
    }

    template<typename TYPE>
    operator std::vector<TYPE>() const
    {
      return as_vector<TYPE>();
    }

    template<typename TYPE>
    operator std::list<TYPE>() const
    {
      return as_list<TYPE>();
    }

    template<typename TYPE>
    operator std::set<TYPE>() const
    {
      return as_set<TYPE>();
    }
  };
}

# include "cpp2ruby.hpp"

#endif
