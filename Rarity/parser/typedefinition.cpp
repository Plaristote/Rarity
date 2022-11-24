#include "definitions.hpp"
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>

using namespace std;

string cxStringToStdString(const CXString&);

TypeDefinition& TypeDefinition::load_from(CXType type, const std::vector<TypeDefinition>& known_types)
{
  string spelt = cxStringToStdString(clang_getTypeSpelling(type));

  return load_from(spelt, known_types);
}

std::string parse_template_parameter(int& i, const std::string& src, const std::vector<TypeDefinition>& known_types)
{
  int template_depth = 1;
  int start = ++i;
  string         param_name;
  TypeDefinition param_type;
  optional<TypeDefinition> parent_type;

  while (template_depth > 0 && i < src.length())
  {
    if (src[i] == '<')
      template_depth++;
    else if (src[i] == '>')
      template_depth--;
    i++;
  }
  param_name = src.substr(start, i - start - 1);
  param_type.load_from(param_name, known_types);
  parent_type = param_type.find_parent_type(known_types);
  if (parent_type)
  {
    param_type.type_full_name = parent_type->type_full_name;
    param_type.is_const = param_type.is_const || parent_type->is_const;
    param_type.is_pointer = param_type.is_pointer || parent_type->is_pointer;
    param_type.is_reference = param_type.is_reference || parent_type->is_reference;
    return param_type.to_full_name();
  }
  return src.substr(start, i - start - 1);
}

TypeDefinition& TypeDefinition::load_from(const string& type, const vector<TypeDefinition>& known_types)
{
  string spelt = type;
  list<string> tokens;

  raw_name = type;

  for (int i = spelt.size() - 1 ; i >= 0 ; --i)
  {
    if (spelt[i] == '*')
      is_pointer++;
    else if (spelt[i] == '&')
      is_reference++;
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

  string part;

  for (int i = 0 ; i < spelt.length() ; ++i)
  {
    if (spelt[i] == '<')
    {
      part += '<' + parse_template_parameter(i, spelt, known_types) + '>';
      continue ;
    }
    if (spelt[i] == ':' && spelt[i + 1] == ':')
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

std::string TypeDefinition::to_string() const
{
  std::string result;

  if (is_const)
    result += "const ";
  result += Crails::join(scopes, "::") + "::" + name;
  for (int i = 0 ; i < is_pointer ; ++i)
    result += '*';
  for (int i = 0 ; i < is_reference ; ++i)
    result += '&';
  return result;
}

std::string TypeDefinition::to_full_name() const
{
  std::string result;

  if (is_const)
    result += "const ";
  result += type_full_name;
  for (int i = 0 ; i < is_pointer ; ++i)
    result += '*';
  for (int i = 0 ; i < is_reference ; ++i)
    result += '&';
  return result;
}
