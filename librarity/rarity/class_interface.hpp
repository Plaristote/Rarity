#ifndef  IRARITYCLASS_HPP
# define IRARITYCLASS_HPP

# include <ruby.h>

struct IRarityClass
{
  virtual ~IRarityClass() {}
  virtual VALUE ruby_instance(void) const = 0;
  operator VALUE() const { return ruby_instance(); }
};

namespace Ruby
{
  template<typename TYPE>
  TYPE to_cpp_type(VALUE value);
}

#endif
