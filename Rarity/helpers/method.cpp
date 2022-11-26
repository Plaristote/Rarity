#include "definitions.hpp"
#include <crails/utils/semantics.hpp>
#include <regex>
#include <map>

using namespace std;

string binding_name(const MethodDefinition& method)
{
  const regex operator_overload_pattern("^operator(<<|>>|==|!=|<=|>=|\\[\\]|[+\\-%*<>\\/])$");
  const map<string, string> replace_map{
    {"<<", "_in_redirection"},
    {">>", "_out_redirection"},
    {"==", "_equal"}, {"!=", "_different"},
    {"<=", "_lte"}, {">=", "_gte"}, {"<", "_smaller"}, {">", "_greater"},
    {"+", "_plus"}, {"-", "_minus"}, {"/", "_divide"}, {"*", "_times"}, {"%", "_modulo"},
    {"[]", "_array_index"}
  };

  if (sregex_iterator(method.name.begin(), method.name.end(), operator_overload_pattern) != std::sregex_iterator())
  {
    string operator_name = method.name.substr(std::string("operator").length(), 2);
    auto it = replace_map.find(operator_name);

    if (it != replace_map.end())
      return "operator" + it->second;
  }
  return method.name;
}

string ruby_name(const MethodDefinition& method)
{
  const regex operator_overload_pattern("^operator(<<|>>|==|!=|<=|>=|\\[\\]|[+\\-%*<>\\/])$");

  if (sregex_iterator(method.name.begin(), method.name.end(), operator_overload_pattern) != std::sregex_iterator())
    return method.name.substr(std::string("operator").length(), 2);
  return Crails::underscore(method.name);
}
