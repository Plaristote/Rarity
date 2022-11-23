#include "../rarity.hpp"
#include <cstdarg>
#include <iostream>

Ruby::Object Ruby::Object::apply(Ruby::Symbol method, unsigned int argc, ...)
{
  this->method = method;
  this->argc   = argc;
  argv         = 0;
  values       = new VALUE[argc];
  result       = Qnil;

  if (argc > 0)
  {
    va_list ar;

    argv = ALLOCA_N(IRarityClass*, argc);
    va_start(ar, argc);
    for (unsigned int i = 0 ; i < argc ; ++i)
    {
      argv[i]     = va_arg(ar, IRarityClass*);
      if (argv[i] != 0)
        values[i] = argv[i]->ruby_instance();
      else
        values[i] = Qnil;
    }
    va_end(ar);
  }
  {
    int state = 0;

    result = rb_protect(wrapped_apply, reinterpret_cast<VALUE>(this), &state);
    if (state)
      throw Ruby::Exception();
  }
  delete[] values;
  return (result);
}

VALUE Ruby::Object::wrapped_apply(VALUE self)
{
  Ruby::Object& _this = *(reinterpret_cast<Ruby::Object*>(self));

  return (rb_funcall2(_this.instance, _this.method, _this.argc, _this.values));
}
