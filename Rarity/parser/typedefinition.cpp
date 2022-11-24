#include "definitions.hpp"
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
