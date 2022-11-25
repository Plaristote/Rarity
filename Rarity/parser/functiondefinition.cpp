#include "definitions.hpp"
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>
#include <crails/utils/semantics.hpp>

using namespace std;

string FunctionDefinition::binding_name() const
{
  string result;

  for (int i = 0 ; i < full_name.length() ; ++i)
  {
    if (full_name[i] == ':')
      result += '_';
    else
      result += full_name[i];
  }
  return result;
}

string FunctionDefinition::ruby_name() const
{
  return name;
}

string FunctionDefinition::cpp_context() const
{
  auto parts = Crails::split(full_name, ':');
  auto last = parts.begin();

  std::advance(last, parts.size() - 1);
  parts.erase(last);
  return "::" + Crails::join(parts, "::");
}

string FunctionDefinition::ruby_context() const
{
  auto parts = Crails::split(cpp_context(), ':');
  stringstream stream;

  for (const auto& part : parts)
    stream << "::" << Crails::camelize(part, Crails::UpperCamelcase);
  return stream.str();
}
