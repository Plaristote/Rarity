#pragma once
#include <string>
#include <optional>
#include <vector>
#include <clang-c/Index.h>

struct TemplateParameter
{
  std::string type;
  std::string name;
  std::string default_value;
};

typedef std::vector<TemplateParameter> TemplateParameters;

struct TypeDefinition
{
  std::string              raw_name;
  std::string              name;
  std::vector<std::string> scopes;
  std::string              type_full_name;
  bool                     is_const = false;
  int                      is_reference = 0;
  int                      is_pointer = 0;

  TypeDefinition& load_from(CXType, const std::vector<TypeDefinition>& known_types);
  TypeDefinition& load_from(const std::string& name, const std::vector<TypeDefinition>& known_types);
  unsigned char   type_match(const TypeDefinition&);
  std::string     solve_type(const std::vector<TypeDefinition>& known_types);
  std::optional<TypeDefinition> find_parent_type(const std::vector<TypeDefinition>& known_types);
  std::string     to_string() const;
  std::string     to_full_name() const;
};

struct ParamDefinition : public std::string
{
  ParamDefinition() {}
  ParamDefinition(CXType type, const std::vector<TypeDefinition>& known_types);
  ParamDefinition(const std::string& name) : std::string(name) {}

  bool is_const     = false;
  int  is_reference = 0;
  int  is_pointer   = 0;

  std::string to_string() const;
};

struct InvokableDefinition
{
  std::optional<ParamDefinition> return_type;
  std::vector<ParamDefinition>   params;
  TemplateParameters template_parameters;
  bool is_template() const { return template_parameters.size() > 0; }
};

struct MethodDefinition : public InvokableDefinition
{
  bool               is_static;
  std::string        name;
  std::string        visibility;

  std::string binding_name() const;
  std::string ruby_name() const;
};

struct FunctionDefinition : public InvokableDefinition
{
  std::string        name;
  std::string        full_name;

  std::string binding_name() const;
  std::string ruby_name() const;
  std::string cpp_context() const;
  std::string ruby_context() const;
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
  std::vector<std::string>        bases;
  std::vector<std::string>        known_bases;
  std::vector<MethodDefinition>   constructors;
  std::vector<MethodDefinition>   methods;
  TemplateParameters              template_parameters;
  bool is_empty() const { return constructors.size() + methods.size() == 0; }
  bool is_template() const { return template_parameters.size() > 0; }
};
