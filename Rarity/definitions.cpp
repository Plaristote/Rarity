#include "definitions.hpp"
#include <map>
#include <regex>
#include <sstream>
#include <iostream>
#include <optional>
#include <crails/utils/semantics.hpp>
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>

using namespace std;

string cxStringToStdString(const CXString&);

TypeDefinition& TypeDefinition::load_from(CXType type)
{
  string spelt = cxStringToStdString(clang_getTypeSpelling(type));
  list<string> tokens;

  for (int i = spelt.size() - 1 ; i >= 0 ; --i)
  {
    if (spelt[i] == '*')
      is_pointer = true;
    else if (spelt[i] == '&')
      is_reference = true;
    else if (spelt[i] != ' ')
    {
      spelt = spelt.substr(0, i + 1);
      break ;
    }
  }

  tokens  = Crails::split(spelt, ' ');
  if (*tokens.begin() == "const")
  {
    is_const = true;
    spelt = Crails::join(++tokens.begin(), tokens.end(), ' ');
  }

  short template_depth = 0;
  string part;

  for (int i = 0 ; i < spelt.length() ; ++i)
  {
    if (spelt[i] == '<')
      template_depth++;
    else if (spelt[i] == '>')
      template_depth--;
    if (spelt[i] == ':' && spelt[i + 1] == ':' && template_depth == 0)
    {
      scopes.push_back(part);
      part.clear();
      ++i;
    }
    else
      part += spelt[i];
  }
  name = part;
  return *this;
}

unsigned char TypeDefinition::type_match(const TypeDefinition& b)
{
  if (name == b.name)
  {
    auto a_it = scopes.rbegin();
    auto b_it = b.scopes.rbegin();

    while (a_it != scopes.rend() && b_it != b.scopes.rend() && *a_it == *b_it)
    {
      a_it++;
      b_it++;
    }
    if (a_it == scopes.rend())
      return b_it == b.scopes.rend() ? 2 : 1;
  }
  return 0;
}

optional<TypeDefinition> TypeDefinition::find_parent_type(const std::vector<TypeDefinition>& known_types)
{
  optional<TypeDefinition> match;

  for (const auto& candidate : known_types)
  {
    char match_type = type_match(candidate);

    if (match_type > 0)
      match = candidate;
    if (match_type == 2)
      break ;
  }
  return match;
}

std::string TypeDefinition::solve_type(const std::vector<TypeDefinition>& known_types)
{
  auto match = find_parent_type(known_types);

  if (match)
    return match->type_full_name;
  return Crails::join(scopes, "") + "::" + name;
}

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

    param_type.load_from(type);
    parent_type = param_type.find_parent_type(known_types);

    if (parent_type)
    {
      append(parent_type->type_full_name);
      is_const = param_type.is_const || parent_type->is_const;
      is_reference = param_type.is_reference || parent_type->is_reference;
      is_pointer = param_type.is_pointer || parent_type->is_pointer;
    }
    else
      append(Crails::join(param_type.scopes, "::") + "::" + param_type.name);
  }
}

std::string NamespaceDefinition::binding_name() const
{
  std::string result;

  for (int i = 0 ; i < full_name.length() ; ++i)
  {
    switch (full_name[i])
    {
    case ':':
      result += '_';
    case '<':
    case '>':
      result += '_';
      break ;
    default:
      result += full_name[i];
      break ;
    }
  }
  return result;
}

std::string MethodDefinition::binding_name() const
{
  const std::regex operator_overload_pattern("^operator(<<|>>|==|!=|<=|>=|\\[\\]|[+\\-%*<>\\/])$");
  const std::map<std::string, std::string> replace_map{
    {"<<", "_in_redirection"},
    {">>", "_out_redirection"},
    {"==", "_equal"}, {"!=", "_different"},
    {"<=", "_lte"}, {">=", "_gte"}, {"<", "_smaller"}, {">", "_greater"},
    {"+", "_plus"}, {"-", "_minus"}, {"/", "_divide"}, {"*", "_times"}, {"%", "_modulo"},
    {"[]", "_array_index"}
  };

  if (std::sregex_iterator(name.begin(), name.end(), operator_overload_pattern) != std::sregex_iterator())
  {
    std::string operator_name = name.substr(std::string("operator").length(), 2);
    auto it = replace_map.find(operator_name);

    if (it != replace_map.end())
      return "operator" + it->second;
  }
  return name;
}

std::string NamespaceDefinition::ruby_name() const
{
  return ruby_context() + "::" + Crails::camelize(name, Crails::UpperCamelcase);
}

std::string MethodDefinition::ruby_name() const
{
  const std::regex operator_overload_pattern("^operator(<<|>>|==|!=|<=|>=|\\[\\]|[+\\-%*<>\\/])$");

  if (std::sregex_iterator(name.begin(), name.end(), operator_overload_pattern) != std::sregex_iterator())
    return name.substr(std::string("operator").length(), 2);
  return Crails::underscore(name);
}

std::string NamespaceDefinition::cpp_context() const
{
  auto parts = Crails::split(full_name, ':');
  auto last = parts.begin();
  stringstream stream;

  if (parts.size() > 1)
  {
    std::advance(last, parts.size() - 1);
    parts.erase(last);
    for (const auto& part : parts)
      stream << "::" << part;
  }
  else
    stream << "::";
  return stream.str();
}

std::string NamespaceDefinition::ruby_context() const
{
  const auto parts = Crails::split(cpp_context(), ':');
  stringstream stream;

  for (const auto& part : parts)
    stream << "::" << Crails::camelize(part, Crails::UpperCamelcase);
  return stream.str();
}
