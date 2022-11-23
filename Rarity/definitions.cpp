#include "definitions.hpp"
#include <map>
#include <regex>
#include <sstream>
#include <iostream>
#include <crails/utils/semantics.hpp>
#include <crails/utils/split.hpp>

using namespace std;

string cxStringToStdString(const CXString&);

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

ParamDefinition::ParamDefinition(CXType type)
{
  auto it = type_to_name.find(type.kind);

  if (it != type_to_name.end())
    append(it->second);
  else
  {
    string spelt = cxStringToStdString(clang_getTypeSpelling(type));
    vector<string> tokens;
    int ii = 0;

    for (int i = 0 ; i < spelt.length() ; ++i)
    {
      if (spelt[i] == ' ')
      {
        tokens.push_back(spelt.substr(ii, i - ii));
        ii = i + 1;
      }
    }
    tokens.push_back(spelt.substr(ii));
    for (auto token : tokens)
    {
      if (token == "const")
        is_const = true;
      else if (token == "&")
        is_reference = true;
      else if (token == "*")
        is_pointer = true;
      else
      {
        if (length() > 0)
          append(1, ' ');
        append(token);
      }
    }
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
