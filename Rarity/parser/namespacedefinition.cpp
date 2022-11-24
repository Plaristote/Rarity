#include "definitions.hpp"
#include <sstream>
#include <crails/utils/semantics.hpp>
#include <crails/utils/split.hpp>

using namespace std;

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


std::string NamespaceDefinition::ruby_name() const
{
  return ruby_context() + "::" + Crails::camelize(name, Crails::UpperCamelcase);
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
