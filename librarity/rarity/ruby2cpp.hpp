#ifndef  RARITY_RUBY2CPP_HPP
# define RARITY_RUBY2CPP_HPP

# include "lambda.hpp"
# include "array.hpp"
# include "class.hpp"
# include "rarity_type_traits.hpp"

namespace Ruby
{
  namespace Ruby2Cpp
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

    template<bool isRarity>
    struct HandleRarityCopy
    {
      template<typename TYPE>
      static TYPE cpp_type(VALUE value)
      {
        TYPE* source = HandleRarity<true>::template cpp_type<TYPE*>(value);
        TYPE copy(*source);

        return copy;
      }
    };

    template<>
    struct HandleRarityCopy<false>
    {
      template<typename TYPE>
      static TYPE cpp_type(VALUE value)
      {
        return HandleRarity<false>::template cpp_type<TYPE>(value);
      }
    };

    template<bool isPointer>
    struct HandlePointers
    {
      template<typename TYPE>
      static TYPE cpp_type(VALUE value)
      {
        return (HandleRarity<std::is_base_of<RarityClass, typename std::pointer_traits<TYPE>::element_type>::value>::template cpp_type<TYPE>(value));
      }
    };

    template<>
    struct HandlePointers<false>
    {
      template<typename TYPE>
      static TYPE cpp_type(VALUE value)
      {
        return HandleRarityCopy<std::is_base_of<RarityClass, TYPE>::value>::template cpp_type<TYPE>(value);
      }
    };

    template<>
    struct HandleArray<false>
    {
      template<typename TYPE>
      static TYPE cpp_type(VALUE value)
      {
        return HandlePointers<std::is_pointer<TYPE>::value>::template cpp_type<TYPE>(value);
      }
    };

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
        return (HandleArray<is_iterable<TYPE>::value>::template cpp_type<TYPE>(value));
      }
    };
  }

  template<typename TYPE>
  TYPE to_cpp_type(VALUE value)
  {
    using namespace Ruby2Cpp;
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

  template<typename TYPE>
  TYPE as_cpp_type(VALUE value)
  {
    return to_cpp_type<TYPE>(value);
  }

  template<> std::string as_cpp_type<std::string>(VALUE value);
  template<> float as_cpp_type<float>(VALUE value);
  template<> double as_cpp_type<double>(VALUE value);
  template<> char as_cpp_type<char>(VALUE value);
  template<> int as_cpp_type<int>(VALUE value);
  template<> long as_cpp_type<long>(VALUE value);
  template<> long long as_cpp_type<long long>(VALUE value);
  template<> short as_cpp_type<short>(VALUE value);
  template<> unsigned char as_cpp_type<unsigned char>(VALUE value);
  template<> unsigned int as_cpp_type<unsigned int>(VALUE value);
  template<> unsigned long as_cpp_type<unsigned long>(VALUE value);
  template<> unsigned long long as_cpp_type<unsigned long long>(VALUE value);
  template<> unsigned short as_cpp_type<unsigned short>(VALUE value);
}

#endif
