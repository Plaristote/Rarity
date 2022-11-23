#ifndef  RARITY_LAMBDA_HPP
# define RARITY_LAMBDA_HPP

# include "globals.hpp"
# include "object.hpp"
# include "exception.hpp"
# include "cpp2ruby.hpp"
# include <functional>

namespace Ruby
{
  template<int it, typename Current, typename... Args>
  void LoadParams(VALUE* arguments, Current arg, Args... args)
  {
    auto i = to_ruby_type(arg);

    arguments[it] = i->ruby_instance();
    LoadParams<it + 1, Args...>(arguments, args...);
  }

  template<int it, typename Current>
  void LoadParams(VALUE* arguments, Current arg)
  {
    auto i = to_ruby_type(arg);

    arguments[it] = i->ruby_instance();
  }

  class Lambda : public Object
  {
  public:
    Lambda(VALUE instance) : Object(instance) {}

    template<typename RET, typename... Args>
    RET call(Args... args)
    {
      unsigned int   arg_count = sizeof...(Args);
      VALUE          result;
      int            state     = 0;

      values = new VALUE[sizeof...(Args)];
      LoadParams<0, Args...>(values, args...);
      method = Symbol("call");
      argc   = arg_count;
      result = rb_protect(wrapped_apply, reinterpret_cast<VALUE>(this), &state);
      if (state)
        throw new Ruby::Exception();
    }

    template<typename RET, typename... Args>
    operator std::function<RET (Args...)>() const
    {
      return (as_function<RET, Args...>());
    }

    template<typename RET, typename... Args>
    std::function<RET (Args...)> as_function() const
    {
      VALUE self = instance;

      return (std::function<RET (Args...)>([self](Args... args) -> RET
      {
        Lambda lambda(self);

        lambda.call<RET, Args...>(args...);
      }));
    }
  };
}

#endif
