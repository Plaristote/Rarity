#ifndef  RARITY_SCRIPTING_HPP
# define RARITY_SCRIPTING_HPP

# ifdef RARITY_RUBY
#  include "rarity.hpp"
# endif

# ifdef RARITY_ANGELSCRIPT
#  include "as/scriptengine.hpp"
#  include "as/object.hpp"
# endif

// Ruby || (Ruby && AngelScript)
# ifdef RARITY_RUBY
#  define ScriptBindingsDone
#  ifdef RARITY_ANGELSCRIPT
struct ScriptBindingsClass : public RarityClass, public AngelScript::AngelScriptClass
{
  ScriptBindingsClass(const char* str) : RarityClass(str), AngelScriptClass(str) {}
};
#  else
struct ScriptBindingsClass : public RarityClass
{
  ScriptBindingsClass(const char* str) : RarityClass(str) {}
};
#  endif
# endif

// AngelScript
# ifndef ScriptBindingsDone
#  ifdef RARITY_ANGELSCRIPT
#  define ScriptBindingsDone
struct ScriptBindingsClass : public AngelScript::AngelScriptClass
{
  ScriptBindingsClass(const char* str) : AngelScriptClass(str) {}
};
#  endif
# endif

# ifndef ScriptBindingsDone
struct ScriptBindingsClass
{
  ScriptBindingsClass(const char*) {}
};
# endif

# define ScriptBindings        public ScriptBindingsClass
# define script_bindings(name) ScriptBindingsClass(name)

#endif
