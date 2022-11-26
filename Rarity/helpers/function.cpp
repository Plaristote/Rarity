#include "definitions.hpp"
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>
#include <crails/utils/semantics.hpp>

using namespace std;

string binding_name(const FunctionDefinition& function)
{
  string result;

  for (int i = 0 ; i < function.full_name.length() ; ++i)
  {
    if (function.full_name[i] == ':')
      result += '_';
    else
      result += function.full_name[i];
  }
  return result;
}

string ruby_name(const FunctionDefinition& function)
{
  return function.name;
}

string cpp_context(const FunctionDefinition& function)
{
  return function.cpp_context();
}

string ruby_context(const FunctionDefinition& function)
{
  auto parts = Crails::split(cpp_context(function), ':');
  stringstream stream;

  for (const auto& part : parts)
    stream << "::" << Crails::camelize(part, Crails::UpperCamelcase);
  return stream.str();
}
