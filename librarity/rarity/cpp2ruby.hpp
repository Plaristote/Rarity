#ifndef  RARITY_CPP2RUBY_HPP
# define RARITY_CPP2RUBY_HPP

# include <ruby.h>
# include <memory>
# include <sstream>
# include "globals.hpp"
# include "class.hpp"
# include "object.hpp"
# include "class_interface.hpp"

namespace Ruby
{
  template<bool>
  struct HandleRarityClass
  {
    template<typename TPL_TYPE>
    static std::shared_ptr<IRarityClass> RubyType(TPL_TYPE& type)
    {
      TPL_TYPE* rarity_heap = new TPL_TYPE(type);

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
    static std::shared_ptr<IRarityClass> RubyType(TYPE* type)
    {
      return std::make_shared<Ruby::Object>(type->ruby_instance());
    }
  };

  template<>
  struct HandleRarityClass<false>
  {
    template<typename TYPE>
    static std::shared_ptr<IRarityClass> RubyType(TYPE)
    {
      rb_raise(Ruby::Constant("ArgumentError").ruby_instance(), "unsupported return type");
      return nullptr;
    }
  };

  template<typename A, typename B>
  struct IsBaseOf : public std::is_base_of<A, B> {};

  template<typename A, typename B>
  struct IsBaseOf<A, B*> : public std::is_base_of<A, B> {};

  template<typename TYPE>
  std::shared_ptr<IRarityClass> to_ruby_type(TYPE& type)
  {
    return (HandleRarityClass<IsBaseOf<RarityClass, TYPE>::value>::template RubyType(type));
  }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<IRarityClass*>(IRarityClass*& type);
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
