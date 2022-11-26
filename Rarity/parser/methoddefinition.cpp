#include "definitions.hpp"
#include <regex>
#include <map>
#include <crails/utils/semantics.hpp>

using namespace std;

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

std::string MethodDefinition::ruby_name() const
{
  const std::regex operator_overload_pattern("^operator(<<|>>|==|!=|<=|>=|\\[\\]|[+\\-%*<>\\/])$");

  if (std::sregex_iterator(name.begin(), name.end(), operator_overload_pattern) != std::sregex_iterator())
    return name.substr(std::string("operator").length(), 2);
  return Crails::underscore(name);
}

bool MethodDefinition::operator==(const MethodDefinition& other) const
{
  if (name == other.name && params.size() == other.params.size())
  {
    for (int i = 0 ; i < params.size() ; ++i)
    {
      if (params[i] != other.params[i])
        return false;
    }
    return true;
  }
  return false;
}

