#pragma once
#include <libtwili/definitions.hpp>
#include <filesystem>
#include <sstream>
#include <fstream>

class Renderer
{
  const std::string module_name;
  const std::filesystem::path target;
  const std::vector<std::filesystem::path>& files;
  const std::vector<ClassDefinition>& classes;
  const std::vector<NamespaceDefinition>& namespaces;
  const std::vector<FunctionDefinition>& functions;
  std::ofstream source;
  std::string initializer_name;
public:
  Renderer(
    const std::string& module_name,
    const std::filesystem::path& target,
    const std::vector<std::filesystem::path>& files,
    const std::vector<ClassDefinition>& a,
    const std::vector<NamespaceDefinition>& b,
    const std::vector<FunctionDefinition>& c
  ) : module_name(module_name), target(target), files(files), classes(a), namespaces(b), functions(c), source(target.string().c_str())
  {
    initializer_name = "initialize_rarity_bindings";
  }

  void set_initializer_name(const std::string& value) { initializer_name = value; }
  void render();
private:
  void generate_include_paths();
  void generate_instance_getter();
  void generate_instance_finalizer();
  void generate_type_converters();
  void generate_cpp_to_ruby_template();
  void generate_classes_bindings();
  void generate_class_bindings(const ClassDefinition&);
  void generate_constructor_binding(const ClassDefinition&, const MethodDefinition&);
  void generate_static_method_binding(const ClassDefinition&, const MethodDefinition&);
  void generate_method_binding(const ClassDefinition&, const MethodDefinition&);
  void generate_function_binding(const FunctionDefinition&);

  std::string ruby_class_getter(const ClassDefinition&);
  bool is_interface(const ClassDefinition&, std::vector<ClassDefinition> parents = {}) const;
};
