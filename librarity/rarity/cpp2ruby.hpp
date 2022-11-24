#ifndef  RARITY_CPP2RUBY_HPP
# define RARITY_CPP2RUBY_HPP

# include <ruby.h>
# include <memory>
# include <sstream>
# include "globals.hpp"
# include "class.hpp"
# include "object.hpp"
# include "lambda.hpp"
# include "class_interface.hpp"
# include "rarity_type_traits.hpp"

namespace Ruby
{
  namespace Cpp2Ruby
  {
    template<bool>
    struct HandleRarityClass
    {
      template<typename TPL_TYPE>
      static std::shared_ptr<IRarityClass> ruby_type(TPL_TYPE& value)
      {
        TPL_TYPE* rarity_heap = new TPL_TYPE(value);

        rarity_heap->template initialize_rarity_bindings<TPL_TYPE>();
        {
          std::stringstream stream;
          Ruby::Constant    os("ObjectSpace");
          Ruby::Object      proc;

          stream << (long)rarity_heap;
          proc = Ruby::evaluate("Proc.new do (" + rarity_heap->ruby_symbol() + ".finalize " + stream.str() + ") end");
          os.apply("define_finalizer", 2, rarity_heap, &proc);
        }
        return std::make_shared<Ruby::Object>(rarity_heap->ruby_instance());
      }

      template<typename TYPE>
      static std::shared_ptr<IRarityClass> ruby_type(TYPE* value)
      {
        return std::make_shared<Ruby::Object>(value->ruby_instance());
      }
    };

    template<>
    struct HandleRarityClass<false>
    {
      template<typename TYPE>
      static std::shared_ptr<IRarityClass> ruby_type(TYPE)
      {
        rb_raise(Ruby::Constant("ArgumentError").ruby_instance(), "unsupported return type");
        return nullptr;
      }
    };

    template<bool>
    struct HandleArray
    {
      template<typename TYPE>
      static std::shared_ptr<IRarityClass> ruby_type(TYPE value)
      {
        return std::make_shared<Ruby::Array>(value.begin(), value.end());
      }
    };

    template<>
    struct HandleArray<false>
    {
      template<typename TYPE>
      static std::shared_ptr<IRarityClass> ruby_type(TYPE value)
      {
        return (HandleRarityClass<is_base_of<RarityClass, TYPE>::value>::template ruby_type(value));
      }
    };

    template<bool>
    struct HandleFunction
    {
      template<typename TYPE>
      static std::shared_ptr<IRarityClass> ruby_type(TYPE value)
      {
        return HandleArray<is_iterable<TYPE>::value>::template ruby_type<TYPE>(value);
      }
    };

    template<>
    struct HandleFunction<true>
    {
      template<typename RET, typename... ARGS>
      static std::shared_ptr<IRarityClass> ruby_type(std::function<RET(ARGS...)> value)
      {
        return std::make_shared<Ruby::Lambda>(Lambda::Factory<RET, ARGS...>::create(value));
      }
    };
  }

  template<typename TYPE>
  std::shared_ptr<IRarityClass> to_ruby_type(TYPE& value)
  {
    using namespace Cpp2Ruby;
    return HandleFunction<is_std_function<TYPE>::value>::template ruby_type(value);
  }

  class Object;

  template<> std::shared_ptr<IRarityClass> to_ruby_type<Ruby::Object>(Ruby::Object& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<IRarityClass*>(IRarityClass*& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<const char*>(const char*& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<std::string>(std::string& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<const std::string>(const std::string& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<bool>(bool& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<float>(float& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<double>(double& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<int>(int& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<long>(long& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<long long>(long long& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<unsigned int>(unsigned int& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<unsigned long>(unsigned long& value);
  template<> std::shared_ptr<IRarityClass> to_ruby_type<unsigned long long>(unsigned long long& value);
}

#endif
