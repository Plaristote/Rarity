#include "definitions.hpp"
#include <map>
#include <crails/utils/join.hpp>

using namespace std;

const map<CXTypeKind, string> type_to_name{
  {CXType_Bool,       "bool"},
  {CXType_Char_U,     "char"},
  {CXType_UChar,      "unsigned char"},
  {CXType_UShort,     "unsigned short"},
  {CXType_UInt,       "unsigned int"},
  {CXType_ULong,      "unsigned long"},
  {CXType_ULongLong,  "unsigned long long"},
  {CXType_Short,      "short"},
  {CXType_Int,        "int"},
  {CXType_Long,       "long"},
  {CXType_LongLong,   "long long"},
  {CXType_Float,      "float"},
  {CXType_Double,     "double"},
  {CXType_LongDouble, "long double"}
};

ParamDefinition::ParamDefinition(CXType type, const std::vector<TypeDefinition>& known_types)
{
  auto it = type_to_name.find(type.kind);

  if (it != type_to_name.end())
    append(it->second);
  else
  {
    TypeDefinition param_type;
    optional<TypeDefinition> parent_type;

    param_type.load_from(type, known_types);
    is_const = param_type.is_const;
    is_reference += param_type.is_reference;
    is_pointer += param_type.is_pointer;
    parent_type = param_type.find_parent_type(known_types);
    if (parent_type)
    {
      append(parent_type->type_full_name);
      is_const = is_const || parent_type->is_const;
      is_reference += parent_type->is_reference;
      is_pointer += parent_type->is_pointer;
    }
    else
      append(Crails::join(param_type.scopes, "::") + "::" + param_type.name);
  }
}

std::string ParamDefinition::to_string() const
{
  string result;

  if (is_const)
    result += "const ";
  result += *this;
  for (int i = 0 ; i < is_pointer ; ++i)
    result += '*';
  for (int i = 0 ; i < is_reference ; ++i)
    result += '&';
  return result;
}
