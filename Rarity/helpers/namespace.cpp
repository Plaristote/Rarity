#include "definitions.hpp"
#include <sstream>
#include <crails/utils/semantics.hpp>
#include <crails/utils/split.hpp>

using namespace std;

string binding_name(const NamespaceDefinition& ns)
{
  string result;

  for (int i = 0 ; i < ns.full_name.length() ; ++i)
  {
    switch (ns.full_name[i])
    {
    case ':':
      result += '_';
    case '<':
    case '>':
      result += '_';
      break ;
    default:
      result += ns.full_name[i];
      break ;
    }
  }
  return result;
}

string ruby_name(const NamespaceDefinition& ns)
{
  return ruby_context(ns) + "::" + Crails::camelize(ns.name, Crails::UpperCamelcase);
}

string cpp_context(const NamespaceDefinition& ns)
{
  return ns.cpp_context();
}

string ruby_context(const NamespaceDefinition& ns)
{
  const auto parts = Crails::split(cpp_context(ns), ':');
  stringstream stream;

  for (const auto& part : parts)
    stream << "::" << Crails::camelize(part, Crails::UpperCamelcase);
  return stream.str();
}
