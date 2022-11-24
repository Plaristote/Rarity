#pragma once
#include <string>
#include <optional>
#include <vector>
#include <clang-c/Index.h>

struct TypeDefinition
{
  std::string              name;
  std::vector<std::string> scopes;
  std::string              type_full_name;
  bool                     is_const = false;
  bool                     is_reference = false;
  bool                     is_pointer = false;

  TypeDefinition& load_from(CXType);
  unsigned char   type_match(const TypeDefinition&);
  std::string     solve_type(const std::vector<TypeDefinition>& known_types);
  std::optional<TypeDefinition> find_parent_type(const std::vector<TypeDefinition>& known_types);
};

struct ParamDefinition : public std::string
{
  ParamDefinition() {}
  ParamDefinition(CXType type, const std::vector<TypeDefinition>& known_types);
  ParamDefinition(const std::string& name) : std::string(name) {}

  bool is_const     = false;
  bool is_reference = false;
  bool is_pointer   = false;
};

struct MethodDefinition
{
  bool                           is_static;
  std::string                    name;
  std::optional<ParamDefinition> return_type;
  std::vector<ParamDefinition>   params;

  std::string binding_name() const;
  std::string ruby_name() const;
};

struct NamespaceDefinition
{
  std::string name;
  std::string full_name;

  bool operator==(const std::string& value) const { return name == value; }
  std::string binding_name() const;
  std::string ruby_name() const;
  std::string cpp_context() const;
  std::string ruby_context() const;
};

struct ClassDefinition : public NamespaceDefinition
{
  std::string                     cpp_base;
  std::optional<MethodDefinition> constructor;
  std::vector<MethodDefinition>   methods;
};
