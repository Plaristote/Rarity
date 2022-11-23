#ifndef  RARITY_RUBY2CPP_HPP
# define RARITY_RUBY2CPP_HPP

# include "lambda.hpp"
# include "array.hpp"
# include "class.hpp"
# include <functional>
# include <list>
# include <type_traits>

namespace Ruby
{
  template<bool IsArray>
  struct HandleArray
  {
    template<typename TYPE>
    static TYPE cpp_type(VALUE value)
    {
      Ruby::Array my_array(value);

      return (my_array);
    }
  };

  template<bool IsRarity>
  struct HandleRarity
  {
    template<typename TYPE>
    static TYPE cpp_type(VALUE value)
    {
      if (value != Qnil)
      {
        VALUE pointer = rb_ivar_get(value, rb_intern("@rarity_cpp_pointer"));

        if (pointer == Qnil)
        {
          rb_raise(Ruby::Constant("ArgumentError").ruby_instance(), "invalid rarity pointer: the C++ counterpart might have expired, or the type was not a proper Rarity object.");
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
    static TYPE cpp_type(VALUE value)
    {
      Ruby::Object       object(value);
      const std::string  rubyname = to_cpp_type<std::string>(object.apply("class").apply("name"));
      const std::string  cppname  = typeid(TYPE).name();
      const std::string  message  = "unhandled type conversion from [Ruby][" + rubyname + "] to [C++][" + cppname + ']';

      rb_raise(Ruby::Constant("Exception").ruby_instance(), message.c_str());
    }
  };

  template<>
  struct HandleArray<false>
  {
    template<typename TYPE>
    static TYPE cpp_type(VALUE value)
    {
      return (HandleRarity<IsBaseOf<RarityClass*, TYPE>::value>::template cpp_type<TYPE>(value));
    }
  };

  template<typename Test, template<typename...> class Ref>
  struct is_specialization : std::false_type {};

  template<template<typename...> class Ref, typename... Args>
  struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

  template<typename T>
  struct is_std_vector : is_specialization<T, std::vector> {};

  template<typename T>
  struct is_std_list : is_specialization<T, std::list> {};

  template<typename>
  struct is_std_function : public std::false_type {};

  template<typename A, typename... Args>
  struct is_std_function<std::function<A (Args...)> > : public std::true_type {};

  template<bool>
  struct HandleFunction
  {
    template<typename TYPE>
    static TYPE cpp_type(VALUE value)
    {
      Lambda lambda(value);

      return (lambda);
    }
  };

  template<>
  struct HandleFunction<false>
  {
    template<typename TYPE>
    static TYPE cpp_type(VALUE value)
    {
      return (HandleArray<is_std_vector<TYPE>::value || is_std_list<TYPE>::value>::template cpp_type<TYPE>(value));
    }
  };

  template<typename TYPE>
  TYPE to_cpp_type(VALUE value)
  {
    return (HandleFunction<is_std_function<TYPE>::value>::template cpp_type<TYPE>(value));
  }

  template<> Ruby::Object to_cpp_type<Ruby::Object>(VALUE value);
  template<> std::string to_cpp_type<std::string>(VALUE value);
  template<> bool to_cpp_type<bool>(VALUE value);
  template<> float to_cpp_type<float>(VALUE value);
  template<> double to_cpp_type<double>(VALUE value);
  template<> char to_cpp_type<char>(VALUE value);
  template<> int to_cpp_type<int>(VALUE value);
  template<> long to_cpp_type<long>(VALUE value);
  template<> long long to_cpp_type<long long>(VALUE value);
  template<> short to_cpp_type<short>(VALUE value);
  template<> unsigned char to_cpp_type<unsigned char>(VALUE value);
  template<> unsigned int to_cpp_type<unsigned int>(VALUE value);
  template<> unsigned long to_cpp_type<unsigned long>(VALUE value);
  template<> unsigned long long to_cpp_type<unsigned long long>(VALUE value);
  template<> unsigned short to_cpp_type<unsigned short>(VALUE value);
}

#endif
