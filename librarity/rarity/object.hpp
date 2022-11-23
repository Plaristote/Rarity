#ifndef  RARITY_OBJECT_HPP
# define RARITY_OBJECT_HPP

# include "class_interface.hpp"
# include <ruby.h>
# include <string>

namespace Ruby
{
  class Symbol
  {
  public:
    Symbol(void) {}
    Symbol(ID symbol)              : symbol(symbol) {}
    Symbol(const std::string& str) : symbol(rb_intern(str.c_str())) {}
    Symbol(const char* str)        : symbol(rb_intern(str)) {}

    operator ID() const { return (symbol); }
  private:
    ID    symbol;
  };

  class Lambda;
  class Array;

  class Object : public IRarityClass
  {
    friend class Lambda;
    friend class Array;
  public:
    Object(void) : instance(Qnil) {}
    Object(VALUE instance) : instance(instance) {}

    Object apply(Symbol method, unsigned int argc = 0, ...);

    VALUE  ruby_instance(void) const override { return (instance); }

    template<typename T>
    operator T() const
    {
      return (Ruby::to_cpp_type<T>(instance));
    }

    Object operator[](Object object)
    {
      return (apply(Symbol("[]"), 1, &object));
    }

  private:
    VALUE instance;

    // Call Wrapper
    static VALUE wrapped_apply(VALUE self);
    Symbol         method;
    unsigned int   argc;
    IRarityClass** argv;
    VALUE*         values;
    VALUE          result;
  };

  class Constant : public Object
  {
  public:
    Constant(Symbol symbol, VALUE base = rb_cObject) : Object(rb_const_get(base, symbol)) {}
  };
}

#endif
