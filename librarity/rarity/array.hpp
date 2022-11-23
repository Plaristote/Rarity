#ifndef  RARITY_ARRAY_HPP
# define RARITY_ARRAY_HPP

# include "object.hpp"
# include "cpp2ruby.hpp"
# include <vector>
# include <algorithm>

namespace Ruby
{
  class Array : public Object
  {
  public:
    Array(void)       : Object(rb_ary_new()) {}
    Array(VALUE copy) : Object(copy)         {}

    template<typename TYPE>
    Array(const std::vector<TYPE>& array)
    {
      instance = rb_ary_new();
      for (TYPE& copy : array)
      {
        auto tmp = Ruby::to_ruby_type(copy);

        rb_ary_push(instance, tmp->ruby_instance());
      }
    }

    template<typename TYPE>
    std::vector<TYPE> as_vector()
    {
      unsigned int      i    = 0;
      unsigned int      size = NUM2UINT((VALUE)(apply("count")));
      std::vector<TYPE> vector;

      for (; i < size ; ++i)
      {
        Ruby::Object value(rb_ary_entry(instance, i));

        vector.push_back(to_cpp_type<TYPE>(value));
      }
      return (vector);
    }

    template<typename TYPE>
    operator std::vector<TYPE>() const
    {
      Array cpy(instance);

      return (cpy.as_vector<TYPE>());
    }
  };
}

#endif
