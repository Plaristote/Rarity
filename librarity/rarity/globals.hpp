#ifndef  RARITY_GLOBALS_HPP
# define RARITY_GLOBALS_HPP

# include <ruby.h>
# include <string>

namespace Ruby
{
  void        push_include_path(const std::string& path);
  bool        require(const std::string& path);
  VALUE       evaluate(const std::string& code);
  const char* inspect(VALUE value);
}

#endif
