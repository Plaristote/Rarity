#include "ruby2cpp.hpp"

namespace Ruby
{
  template<> Ruby::Object to_cpp_type<Ruby::Object>(VALUE value)
  { return (value); }

  template<> std::string to_cpp_type<std::string>(VALUE value)
  { return (std::string(RSTRING_PTR(value))); }

  template<> bool to_cpp_type<bool>(VALUE value)
  { return (value == Qtrue ? true : false); }

  template<> float to_cpp_type<float>(VALUE value)
  { return (NUM2DBL(value)); }

  template<> double to_cpp_type<double>(VALUE value)
  { return (NUM2DBL(value)); }

  template<> char to_cpp_type<char>(VALUE value)
  { return (NUM2CHR(value)); }

  template<> int to_cpp_type<int>(VALUE value)
  { return (NUM2INT(value)); }

  template<> long to_cpp_type<long>(VALUE value)
  { return (NUM2LONG(value)); }

  template<> long long to_cpp_type<long long>(VALUE value)
  { return (NUM2LL(value)); }

  template<> short to_cpp_type<short>(VALUE value)
  { return (NUM2SHORT(value)); }

  template<> unsigned char to_cpp_type<unsigned char>(VALUE value)
  { return (NUM2CHR(value)); }

  template<> unsigned int to_cpp_type<unsigned int>(VALUE value)
  { return (NUM2UINT(value)); }

  template<> unsigned long to_cpp_type<unsigned long>(VALUE value)
  { return (NUM2ULONG(value)); }

  template<> unsigned long long to_cpp_type<unsigned long long>(VALUE value)
  { return (NUM2ULL(value)); }

  template<> unsigned short to_cpp_type<unsigned short>(VALUE value)
  { return (NUM2USHORT(value)); }
}
