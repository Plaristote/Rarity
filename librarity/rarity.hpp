#ifndef  RARITY_HPP
# define RARITY_HPP

# include <ruby.h>
# include "rarity/object.hpp"
# include "rarity/lambda.hpp"
# include "rarity/array.hpp"
# include "rarity/exception.hpp"
# include "rarity/class.hpp"
# include "rarity/context.hpp"
# include "rarity/cpp2ruby.hpp"
# include "rarity/ruby2cpp.hpp"

namespace Ruby
{
  long get_instance_pointer(VALUE self);

  VALUE finalize_rarity_class(VALUE, VALUE);

  VALUE solve_symbol(const std::string&);
}

#endif
