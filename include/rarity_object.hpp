#ifndef  RARITY_OBJECT_HPP
# define RARITY_OBJECT_HPP

# include "rarity.hpp"

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

  class Object : public IRarityClass
  {
  public:
    Object(void) : instance(Qnil) {}
    Object(VALUE instance) : instance(instance) {}

    Object Apply(Symbol method, unsigned int argc = 0, ...);

    VALUE  GetRubyInstance(void) const { return (instance); }

    operator VALUE() const { return (instance); }

  protected:
    VALUE instance;

    // Call Wrapper
    static VALUE WrappedApply(VALUE self);
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

  struct Exception : public std::exception, public Object
  {
    Exception(void) : Object(rb_gv_get("$!"))
    {
      VALUE inspect = Apply("inspect");

      message = RSTRING_PTR(inspect);
    }

    const char* what(void) const throw()
    {
      return (message);
    }

  private:
    const char* message;
  };
}

#endif
