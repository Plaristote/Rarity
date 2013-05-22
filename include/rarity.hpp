#ifndef  RARITY_HPP
# define RARITY_HPP

# include "ruby.h"
# include <string>
# include <iostream>

void RarityInitialize(void);
void RarityFinalize(void);

namespace Ruby
{
  void        PushIncludePath(const std::string& path);
  bool        Require(const std::string& path);
  VALUE       Evaluate(const std::string& code);
  const char* Inspect(VALUE value);
}

/*
 * RarityClass
 */
struct IRarityClass
{
  virtual ~IRarityClass() {}
  virtual VALUE GetRubyInstance(void) const = 0;
};

namespace Ruby
{
  template<typename TYPE>
  TYPE          ToCppType(VALUE value);
}

# include "rarity_object.hpp"

#include <vector>
#include <algorithm>
#include <sstream>

class RarityClass;

namespace Ruby
{
  VALUE SolveSymbol(const std::string&);

  template<bool>
  struct HandleRarityClass
  {
    template<typename TPL_TYPE>
    static IRarityClass* RubyType(TPL_TYPE& type)
    {
      TPL_TYPE* rarity_heap = new TPL_TYPE(type);

      rarity_heap->Initialize();
      {
        std::stringstream stream;
        Ruby::Constant    os("ObjectSpace");
        Ruby::Object      proc;

        stream << (long)rarity_heap;
        proc = Ruby::Evaluate("Proc.new do (" + rarity_heap->GetRubySymbol() + ".finalize " + stream.str() + ") end");
        os.Apply("define_finalizer", 2, rarity_heap, &proc);
      }
      return (new Ruby::Object(rarity_heap->GetRubyInstance()));
    }

    template<typename TYPE>
    static IRarityClass* RubyType(TYPE* type)
    {
      return (new Ruby::Object(type->GetRubyInstance()));
    }
  };

  template<>
  struct HandleRarityClass<false>
  {
    template<typename TYPE>
    static IRarityClass* RubyType(TYPE type)
    {
      rb_raise(Ruby::Constant("ArgumentError").GetRubyInstance(), "unsupported return type");
      return (0);
    }
  };

  template<typename A, typename B>
  struct IsBaseOf : public std::is_base_of<A, B> {};

  template<typename A, typename B>
  struct IsBaseOf<A, B*> : public std::is_base_of<A, B> {};

  template<typename TYPE>
  IRarityClass* ToRubyType(TYPE& type)
  {
    return (HandleRarityClass<IsBaseOf<RarityClass, TYPE>::value>::template RubyType(type));
  }

  template<> IRarityClass* ToRubyType<IRarityClass*>(IRarityClass*& type);

  template<int it, typename Current, typename... Args>
  void LoadParams(VALUE* arguments, Current arg, Args... args)
  {
    IRarityClass* i = ToRubyType(arg);

    arguments[it]   = i->GetRubyInstance();
    delete i;
    LoadParams<it + 1, Args...>(arguments, args...);
  }

  template<int it, typename Current>
  void LoadParams(VALUE* arguments, Current arg)
  {
    IRarityClass* i = ToRubyType(arg);

    arguments[it] = i->GetRubyInstance();
    delete i;
  }

  class Lambda : public Object
  {
  public:
    Lambda(VALUE instance) : Object(instance) {}

    template<typename RET, typename... Args>
    RET Call(Args... args)
    {
      unsigned int   arg_count = sizeof...(Args);
      VALUE          result;
      int            state     = 0;

      values = new VALUE[sizeof...(Args)];
      LoadParams<0, Args...>(values, args...);
      method = Symbol("call");
      argc   = arg_count;
      result = rb_protect(WrappedApply, reinterpret_cast<VALUE>(this), &state);
      if (state)
        throw new Ruby::Exception();
    }

    template<typename RET, typename... Args>
    operator std::function<RET (Args...)>() const
    {
      VALUE self = instance;

      return (std::function<RET (Args...)>([self](Args... args) -> RET
      {
        Lambda lambda(self);

        lambda.Call<RET, Args...>(args...);
      }));
    }

  private:
  };

  class Array : public Object
  {
  public:
    Array(void)       : Object(rb_ary_new()) {}
    Array(VALUE copy) : Object(copy)         {}

    template<typename TYPE>
    Array(const std::vector<TYPE>& array)
    {
      instance = rb_ary_new();
      std::for_each(array.begin(), array.end(), [this](TYPE& copy)
      {
        IRarityClass* tmp = Ruby::ToRubyType(copy);

        rb_ary_push(instance, tmp->GetRubyInstance());
        delete tmp;
      });
    }

    template<typename TYPE>
    std::vector<TYPE> AsVector()
    {
      unsigned int      i    = 0;
      unsigned int      size = NUM2UINT((VALUE)(Apply("count")));
      std::vector<TYPE> vector;

      for (; i < size ; ++i)
      {
        Ruby::Object value(rb_ary_entry(instance, i));

        vector.push_back(ToCppType<TYPE>(value));
      }
      return (vector);
    }

    template<typename TYPE>
    operator std::vector<TYPE>() const
    {
      Array cpy(instance);

      return (cpy.AsVector<TYPE>());
    }
  };

  template<bool IsArray>
  struct HandleArray
  {
    template<typename TYPE>
    static TYPE CppType(VALUE value)
    {
      Ruby::Array my_array(value);

      return (my_array);
    }
  };

  template<bool IsRarity>
  struct HandleRarity
  {
    template<typename TYPE>
    static TYPE CppType(VALUE value)
    {
      if (value != Qnil)
      {
        VALUE pointer = rb_ivar_get(value, rb_intern("@rarity_cpp_pointer"));

        if (pointer == Qnil)
        {
          rb_raise(Ruby::Constant("ArgumentError").GetRubyInstance(), "invalid rarity pointer: the C++ counterpart might have expired, or the type was not a proper Rarity object.");
        }
        return (reinterpret_cast<TYPE>(NUM2LONG(pointer)));
      }
      return (0);
    }
  };

  template<>
  struct HandleRarity<false>
  {
    template<typename TYPE>
    static TYPE CppType(VALUE value)
    {
      Ruby::Object       object(value);
      const std::string  rubyname = ToCppType<std::string>(object.Apply("class").Apply("name"));
      const std::string  cppname  = typeid(TYPE).name();
      const std::string  message  = "unhandled type conversion from [Ruby][" + rubyname + "] to [C++][" + cppname + ']';

      rb_raise(Ruby::Constant("Exception").GetRubyInstance(), message.c_str());
    }
  };

  template<>
  struct HandleArray<false>
  {
    template<typename TYPE>
    static TYPE CppType(VALUE value)
    {
      return (HandleRarity<IsBaseOf<RarityClass*, TYPE>::value>::template CppType<TYPE>(value));
    }
  };

  template<typename>
  struct is_std_vector : std::false_type {};

  template<typename T, typename A>
  struct is_std_vector<std::vector<T,A> > : std::true_type {};

  template<typename>
  struct is_std_function : public std::false_type {};

  template<typename A, typename... Args>
  struct is_std_function<std::function<A (Args...)> > : public std::true_type {};

  template<bool>
  struct HandleFunction
  {
    template<typename TYPE>
    static TYPE CppType(VALUE value)
    {
      Lambda lambda(value);

      return (lambda);
    }
  };

  template<>
  struct HandleFunction<false>
  {
    template<typename TYPE>
    static TYPE CppType(VALUE value)
    {
      return (HandleArray<is_std_vector<TYPE>::value>::template CppType<TYPE>(value));  
    }
  };

  template<typename TYPE>
  TYPE          ToCppType(VALUE value)
  {
    return (HandleFunction<is_std_function<TYPE>::value>::template CppType<TYPE>(value));
  }
}

#include <iostream>
struct Rarity
{
private:
  static VALUE WrappedProtect(VALUE value)
  {
    std::function<void (void)>* functor = reinterpret_cast<std::function<void (void)>*>(value);

    (*functor)();
    return (Qnil);
  }

public:
  static void Protect(std::function<void (void)> block)
  {
    int state = 0;

    rb_protect(Rarity::WrappedProtect, reinterpret_cast<VALUE>(&block), &state);
    if (state != 0)
      throw new Ruby::Exception();
  }
  
  static VALUE wrapping_module;
};

class RarityClass : public IRarityClass
{
public:
  RarityClass(const std::string& classname) : ruby_cpp_name(classname)
  {
    Initialize();
  }

  void Initialize()
  {
    rb_gc_register_address(&ruby_instance);
    Rarity::Protect([this]()
    {
    //name_symbol    = rb_intern(ruby_cpp_name.c_str());
    //ruby_class     = rb_const_get(Rarity::wrapping_module, name_symbol);
      ruby_class     = Ruby::SolveSymbol(ruby_cpp_name);
      if (ruby_class != Qnil)
      {
        rb_define_alias(ruby_class, "_real_initialize", "initialize");
        rb_define_alias(ruby_class, "initialize", "_initialize");

        ruby_instance  = rb_class_new_instance(0, 0, ruby_class);
        pointer_symbol = rb_intern("@rarity_cpp_pointer");

        rb_define_alias(ruby_class, "initialize", "_real_initialize");

        SetRubyInstance(ruby_instance);
      }
    });
  }

  ~RarityClass(void)
  {
    rb_ivar_set(ruby_instance, pointer_symbol, Qnil);
    rb_gc_unregister_address(&ruby_instance);
  }

  VALUE GetRubyType(void) const
  {
    return (rb_funcall2(ruby_instance, rb_intern("class"), 0, 0));
  }

  VALUE GetRubyInstance(void) const { return (ruby_instance); }

  void  SetRubyInstance(VALUE val)
  {
    ruby_instance = val;
    rb_ivar_set(ruby_instance, pointer_symbol, INT2FIX((long)this));
  }

  const std::string& GetRubySymbol(void) const
  {
    return (ruby_cpp_name);
  }

  operator VALUE() const
  {
    return (ruby_instance);
  }

//private:
  const std::string ruby_cpp_name;
  ID                pointer_symbol;
  ID                name_symbol;
  VALUE             ruby_instance;
  VALUE             ruby_class;
};

#endif
