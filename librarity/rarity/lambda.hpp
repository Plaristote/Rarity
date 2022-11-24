#ifndef  RARITY_LAMBDA_HPP
# define RARITY_LAMBDA_HPP

# include "globals.hpp"
# include "object.hpp"
# include "exception.hpp"
# include <functional>
# include <memory>

namespace Ruby
{
  template<typename TYPE>
  std::shared_ptr<IRarityClass> to_ruby_type(TYPE&);

  template<typename TYPE>
  TYPE to_cpp_type(VALUE);

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

  template<typename RET>
  struct LambdaFunctorReturn
  {
    static VALUE call(std::function<RET(void)> callback)
    {
      auto retval = callback();
      return to_ruby_type(retval)->ruby_instance();
    }
  };

  template<>
  struct LambdaFunctorReturn<void>
  {
    static VALUE call(std::function<void(void)> callback)
    {
      callback();
      return Qnil;
    }
  };

  template<typename RET, typename ARG1 = void, typename ARG2 = void, typename ARG3 = void, typename ARG4 = void>
  struct LambdaFunctor
  {
    typedef std::function<RET(ARG1,ARG2,ARG3,ARG4)> Function;
    static VALUE call(VALUE, VALUE ptr, int argc, const VALUE* argv, VALUE)
    {
      Function callback = *reinterpret_cast<Function*>(NUM2LONG(ptr));

      if (argc < 4)
        rb_raise(Ruby::Constant("ArgumentError").ruby_instance(), "expected at least 4 arguments.");
      auto arg1 = to_cpp_type<ARG1>(argv[0]);
      auto arg2 = to_cpp_type<ARG2>(argv[1]);
      auto arg3 = to_cpp_type<ARG3>(argv[2]);
      auto arg4 = to_cpp_type<ARG4>(argv[3]);

      return LambdaFunctorReturn<RET>::call(
        std::bind(callback, arg1, arg2, arg3, arg4)
      );
    }
  };

  template<typename RET, typename ARG1, typename ARG2, typename ARG3>
  struct LambdaFunctor<RET, ARG1, ARG2, ARG3, void>
  {
    typedef std::function<RET(ARG1,ARG2,ARG3)> Function;
    static VALUE call(VALUE, VALUE ptr, int argc, const VALUE* argv, VALUE)
    {
      Function callback = *reinterpret_cast<Function*>(NUM2LONG(ptr));

      if (argc < 3)
        rb_raise(Ruby::Constant("ArgumentError").ruby_instance(), "expected at least 3 arguments.");
      auto arg1 = to_cpp_type<ARG1>(argv[0]);
      auto arg2 = to_cpp_type<ARG2>(argv[1]);
      auto arg3 = to_cpp_type<ARG3>(argv[2]);

      return LambdaFunctorReturn<RET>::call(
        std::bind(callback, arg1, arg2, arg3)
      );
    }
  };

  template<typename RET, typename ARG1, typename ARG2>
  struct LambdaFunctor<RET, ARG1, ARG2, void, void>
  {
    typedef std::function<RET(ARG1,ARG2)> Function;
    static VALUE call(VALUE, VALUE ptr, int argc, const VALUE* argv, VALUE)
    {
      Function callback = *reinterpret_cast<Function*>(NUM2LONG(ptr));

      if (argc < 2)
        rb_raise(Ruby::Constant("ArgumentError").ruby_instance(), "expected at least 2 arguments.");
      auto arg1 = to_cpp_type<ARG1>(argv[0]);
      auto arg2 = to_cpp_type<ARG2>(argv[1]);

      return LambdaFunctorReturn<RET>::call(
        std::bind(callback, arg1, arg2)
      );
    }
  };

  template<typename RET, typename ARG1>
  struct LambdaFunctor<RET, ARG1, void, void, void>
  {
    typedef std::function<RET(ARG1)> Function;
    static VALUE call(VALUE, VALUE ptr, int argc, const VALUE* argv, VALUE)
    {
      Function callback = *reinterpret_cast<Function*>(NUM2LONG(ptr));

      if (argc < 1)
        rb_raise(Ruby::Constant("ArgumentError").ruby_instance(), "expected at least 1 arguments.");
      auto arg1 = to_cpp_type<ARG1>(argv[0]);

      return LambdaFunctorReturn<RET>::call(
        std::bind(callback, arg1)
      );
    }
  };

  template<typename RET>
  struct LambdaFunctor<RET, void, void, void, void>
  {
    typedef std::function<RET(void)> Function;
    static VALUE call(VALUE, VALUE ptr, int, const VALUE*, VALUE)
    {
      Function callback = *reinterpret_cast<Function*>(NUM2LONG(ptr));

      return LambdaFunctorReturn<RET>::call(callback);
    }
  };

  class Lambda : public Object
  {
  public:
    template<typename RET, typename... ARGS>
    struct Factory
    {
      static Lambda create(std::function<RET(ARGS...)> callback)
      {
        return rb_proc_new(LambdaFunctor<RET, ARGS...>::call, LONG2NUM(reinterpret_cast<long>(&callback)));
      }
    };

    Lambda(VALUE instance) : Object(instance) {}

    template<typename RET, typename... Args>
    VALUE call(Args... args)
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
        throw Ruby::Exception();
      return result;
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

        return Ruby::to_cpp_type<RET>(lambda.call<RET, Args...>(args...));
      }));
    }
  };
}

# include "cpp2ruby.hpp"
# include "ruby2cpp.hpp"

#endif
