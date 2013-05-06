#ifndef  RARITY_SCRIPTING_HPP
# define RARITY_SCRIPTING_HPP

# ifdef RARITY_RUBY
#  include "rarity.hpp"

#  ifdef ScriptBindings
#   define ScriptBindingsSwap ScriptBindings
#   undef  ScriptBindings
#   undef  script_bindings
#   define ScriptBindings ScriptBindings, public RarityClass
#   define script_bindings(name) RarityClass(name)
#  else
#   define ScriptBindings public RarityClass
#   define script_bindings(name) RarityClass(name)
#  endif

# endif

# ifndef ScriptBindings
struct RarityEmptyBindings {};

#  define ScriptBindings RarityEmptyBindings
#  define script_bindings(name) RarityEmptyBindings()
# endif

#endif
