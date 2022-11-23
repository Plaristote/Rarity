#include "rarity.hpp"
#include <string>
#include <sstream>

namespace Ruby
{
  long get_instance_pointer(VALUE self)
  {
    VALUE variable = rb_ivar_get(self, rb_intern("@rarity_cpp_pointer"));

    if (variable == Qnil)
      rb_raise(Ruby::Constant("ArgumentError"), "The current Rarity object you\'re trying to use is not uninitialized or has expired.");
    return (NUM2LONG(variable));
  }

  VALUE finalize_rarity_class(VALUE, VALUE param)
  {
    long ptr = NUM2LONG(param);

    delete ((RarityClass*)ptr);
    return (Qnil);
  }

  void push_include_path(const std::string& path)
  {
    ruby_incpush(path.c_str());
  }

  static VALUE rescuable_require(VALUE str_ptr)
  {
    return (rb_require(reinterpret_cast<const char*>(str_ptr)));
  }

  bool require(const std::string& path)
  {
    int state;

    rb_protect(rescuable_require, reinterpret_cast<VALUE>(path.c_str()), &state);
    if (state)
    {
      if (Exception::has_exception())
        throw Ruby::Exception();
      return (false);
    }
    return (true);
  }

  VALUE evaluate(const std::string& code)
  {
    return (rb_eval_string(code.c_str()));
  }

  const char* inspect(VALUE value)
  {
    VALUE ret = rb_funcall2(value, rb_intern("inspect"), 0, 0);

    return (RSTRING_PTR(ret));
  }

  template<>
  std::shared_ptr<IRarityClass> to_ruby_type<IRarityClass*>(IRarityClass*& type)
  {
    return std::make_shared<Object>(type->ruby_instance());
  }

  VALUE solve_symbol(const std::string& temp)
  {
    std::string              symbol   = temp;
    std::vector<std::string> parts;
    VALUE                    constant = rb_cObject; // should maybe match wrapping_module ?
    unsigned int             i        = 0;

    for (i = 0 ; i < symbol.size() ; ++i)
    {
      if (symbol[i] == ':' && symbol[i + 1] == ':')
      {
        parts.push_back(symbol.substr(0, i));
        symbol.erase(0, i + 1);
        i = 0;
      }
    }
    parts.push_back(symbol.substr(0, i));
    for (i = 0 ; i < parts.size() ; ++i)
    {
      VALUE new_constant = Qnil;

      try
      {
        Rarity::protect([&new_constant, constant, parts, i](void)
        {
          new_constant = Ruby::Constant(parts[i].c_str(), constant);
        });
      }
      catch (Ruby::Exception*)
      {
      }

      if (new_constant == Qnil)
        new_constant = rb_define_module_under(constant, parts[i].c_str());
      constant = new_constant;
    }
    return (constant);
  }
}

