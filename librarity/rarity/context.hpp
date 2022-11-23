#ifndef  RARITY_CONTEXT_HPP
# define RARITY_CONTEXT_HPP

# include <ruby.h>
# include <functional>
# include "object.hpp"

class Rarity
{
  static bool is_initialized;
  static VALUE wrapped_protect(VALUE value);
public:
  Rarity(const std::vector<std::string>& options = {});
  ~Rarity();

  static void protect(std::function<void (void)> block);
};

#endif
