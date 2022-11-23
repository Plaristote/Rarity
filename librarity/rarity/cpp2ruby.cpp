#include "cpp2ruby.hpp"

namespace Ruby
{
  template<> std::shared_ptr<IRarityClass> to_ruby_type<const char*>(const char*& value)
  { return std::make_shared<Ruby::Object>(rb_str_new2(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<std::string>(std::string& value)
  { return std::make_shared<Ruby::Object>(rb_str_new2(value.c_str())); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<const std::string>(const std::string& value)
  { return std::make_shared<Ruby::Object>(rb_str_new2(value.c_str())); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<bool>(bool& value)
  { return std::make_shared<Ruby::Object>(value ? Qtrue : Qfalse); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<float>(float& value)
  { return std::make_shared<Ruby::Object>(rb_float_new(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<double>(double& value)
  { return std::make_shared<Ruby::Object>(DBL2NUM(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<int>(int& value)
  { return std::make_shared<Ruby::Object>(INT2NUM(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<long>(long& value)
  { return std::make_shared<Ruby::Object>(LONG2NUM(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<long long>(long long& value)
  { return std::make_shared<Ruby::Object>(LL2NUM(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<unsigned int>(unsigned int& value)
  { return std::make_shared<Ruby::Object>(UINT2NUM(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<unsigned long>(unsigned long& value)
  { return std::make_shared<Ruby::Object>(ULONG2NUM(value)); }

  template<> std::shared_ptr<IRarityClass> to_ruby_type<unsigned long long>(unsigned long long& value)
  { return std::make_shared<Ruby::Object>(ULL2NUM(value)); }
}
