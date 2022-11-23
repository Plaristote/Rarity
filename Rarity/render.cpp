#include "render.hpp"
#include "parser.hpp"
#include <fstream>
#include <map>
#include <algorithm>
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>
#include <crails/utils/semantics.hpp>

using namespace std;

string Renderer::ruby_class_getter(const ClassDefinition& klass)
{
  string result;

  if (module_name.length())
    return module_name + klass.ruby_name();
  return klass.ruby_name();
}

void Renderer::render()
{
  generate_include_paths();
  source << endl;
  source << "static VALUE wrapping_module;" << endl;
  source << endl;
  generate_cpp_to_ruby_template();
  source << endl;
  generate_classes_bindings();
  source.close();
}

void Renderer::generate_include_paths()
{
  source << "#include <rarity.hpp>" << endl
         << "#include <string>" << endl
	 << "#include <sstream>" << endl;
  for (const auto& file : files)
  {
    source << "#include \""
           << filesystem::relative(filesystem::canonical(file), filesystem::canonical(target).parent_path()).string()
	   << '"' << endl;
  }
}

void Renderer::generate_cpp_to_ruby_template()
{
  source
    << "template<typename T>" << endl
    << "static VALUE cpp_to_ruby(T var)" << endl
    << '{' << endl
    << "  auto rarity_class = Ruby::to_ruby_type<T>(var);" << endl
    << "  VALUE value = rarity_class->ruby_instance();" << endl
    << endl
    << "  return value;" << endl
    << '}' << endl;
}

static std::string wrapper_getter(const NamespaceDefinition& klass, bool include_name = false)
{
  auto parts = Crails::split(klass.ruby_context(), ':');

  if (parts.size() > 0)
  {
    stringstream stream;

    if (include_name)
    {
      auto last = parts.begin();
      std::advance(last, parts.size() - 1);
      parts.erase(last);
    }
    for (auto it = parts.rbegin() ; it != parts.rend() ; ++it)
      stream << "Ruby::Constant(\"" << *it << "\", ";
    stream << "wrapping_module";
    for (auto it = parts.rbegin() ; it != parts.rend() ; ++it)
      stream << ')';
    return stream.str();
  }
  return "wrapping_module";
}

static std::string inherits_getter(const ClassDefinition& klass)
{
  if (klass.cpp_base.length() > 0)
    return "RarityClass::ruby_class_for<" + klass.cpp_base + ">()";
  return "rb_cObject";
}

void Renderer::generate_classes_bindings()
{
  source << "typedef VALUE (*RubyMethod)(...);" << endl << endl;
  for (const auto& klass : classes)
    generate_class_bindings(klass);
  source
    << "void " << initializer_name << "(void)" << endl
    << '{' << endl;
  if (module_name.length() > 0)
    source << "  wrapping_module = rb_define_module(\"" << module_name << "\");" << endl;
  else
    source << "  wrapping_module = rb_cObject;" << endl;
  for (const auto& ns : namespaces)
    source << "  rb_define_module_under(" << wrapper_getter(ns, true) << ", \"" << Crails::camelize(ns.name, Crails::UpperCamelcase) << "\");" << endl;
  for (const auto& klass : classes)
    source << "  initialize_" << klass.binding_name() << "();" << endl;
  source << '}' << endl;
}

void Renderer::generate_class_bindings(const ClassDefinition& klass)
{
  if (klass.constructor)
    generate_constructor_binding(klass, *klass.constructor);
  for (const auto& method : klass.methods)
  {
    if (method.is_static)
      generate_static_method_binding(klass, method);
    else
      generate_method_binding(klass, method);
  }
  source
    << "static VALUE binding_" << klass.binding_name() << "__initialize(VALUE self) { return Qnil; }" << endl;
  source
    << endl
    << "static void initialize_" << klass.binding_name() << "()" << endl
    << '{' << endl
    << "  VALUE inherits = " << inherits_getter(klass) << ";" << endl
    << "  // ruby context = " << klass.full_name << " -> " << klass.ruby_context() << endl
    << "  VALUE wrapped = " << wrapper_getter(klass) << ';' << endl
    << "  VALUE klass = rb_define_class_under(wrapped, \"" << klass.name << "\", inherits);" << endl
    << endl
    << "  RarityClass::register_ruby_class_for<" << klass.full_name << ">(klass);" << endl
    << "  rb_define_method(klass, \"_initialize\", reinterpret_cast<RubyMethod>(binding_" << klass.binding_name() << "__initialize), 0);" << endl
    << "  rb_define_module_function(klass, \"finalize\", reinterpret_cast<RubyMethod>(Ruby::finalize_rarity_class), 1);" << endl;
  if (klass.constructor)
    source << "  rb_define_method(klass, \"initialize\", reinterpret_cast<RubyMethod>(binding_" << klass.binding_name() << '_' << klass.constructor->binding_name() << "), " << klass.constructor->params.size() << ");" << endl;
  for (const auto& method : klass.methods)
  {
    source
      << "  " << (method.is_static ? "rb_define_module_function" : "rb_define_method")
      << "(klass, \"" << method.ruby_name() << "\", "
      << "reinterpret_cast<RubyMethod>(binding_" << klass.binding_name() << '_' << method.binding_name() << "), "
      << method.params.size() << ");" << endl;
  }
  // TODO attributes ?
  source << '}' << endl;
}

static std::string binding_params_for(const MethodDefinition& method)
{
  stringstream stream;

  for (int i = 0 ; i != method.params.size() ; ++i)
    stream << ", VALUE param_" << i;
  return stream.str();
}

static std::string binding_params_apply(const MethodDefinition& method)
{
  stringstream stream;

  for (int i = 0 ; i != method.params.size() ; ++i)
  {
    const auto& param = method.params[i];

    if (i > 0) stream << ", ";
    if (param == "char" && param.is_pointer && param.is_const)
      stream << "(Ruby::to_cpp_type<std::string>(param_" << i << ")).c_str()";
    else if (param.is_pointer)
      stream << "(Ruby::to_cpp_type<" << param << "*>(param_" << i << "))";
    else
      stream << "(Ruby::to_cpp_type<" << param << ">(param_" << i << "))";
  }
  return stream.str();
}

std::map<std::string, std::vector<std::string>> cpp_to_ruby_types{
  {"String",  {"std::string","string"}},
  {"Bignum",  {"long", "long long", "unsigned long", "unsigned long long"}},
  {"Integer", {"short", "int", "unsigned short", "unsigned int"}},
  {"Float",   {"float", "double", "long double"}}
};

static std::string binding_params_check(const ClassDefinition& klass, const MethodDefinition& method)
{
  stringstream stream;

  for (int i = 0 ; i < method.params.size() ; ++i)
  {
    const auto& param = method.params[i];
    string expected_ruby_type = param;
    stringstream message;

    message << "Mismatched type in " << klass.name << "::" << method.name << ", argument " << (i + 1) << ". "; 
    if (param == "char" && param.is_pointer && param.is_const)
      expected_ruby_type = "String";
    else if (param.find("function<") != string::npos)
      expected_ruby_type = "Proc";
    else if (param.find("vector<") != string::npos)
      expected_ruby_type = "Array";
    else if (!param.is_pointer)
    {
      for (auto it = cpp_to_ruby_types.begin() ; it != cpp_to_ruby_types.end() ; ++it)
      {
        auto found = std::find(it->second.begin(), it->second.end(), expected_ruby_type);

        if (found != it->second.end())
        {
          expected_ruby_type = it->first;
          break ;
        }
      }
    }
    if (param == "bool")
    {
      message << "Expecting Boolean.";
      stream
	<< "  {" << endl
        << "    if (param_" << i << " != Qtrue && param_" << i << " != Qfalse)" << endl
	<< "    {" << endl
	<< "      rb_raise((Ruby::Constant(\"ArgumentError\").ruby_instance()), \"" << message.str() << "\");" << endl
	<< "    }" << endl
	<< "  }";
    }
    else
    {
      message << "Expecting " << expected_ruby_type << ", got \" + tmp_typename + \".";
      stream
	<< "  {" << endl
        << "    Ruby::Object tmp(param_" << i << ");" << endl
	<< "    const std::string tmp_typename = (Ruby::to_cpp_type<std::string>(tmp.apply(\"class\").apply(\"name\")));" << endl
	<< "    if (tmp_typename != \"" << expected_ruby_type << "\")" << endl
	<< "      rb_raise((Ruby::Constant(\"ArgumentError\").ruby_instance()), (\"" << message.str() << "\").c_str());" << endl
	<< "  }" << endl;
    }
  }
  return stream.str();
}

void Renderer::generate_constructor_binding(const ClassDefinition& klass, const MethodDefinition& method)
{
  source
    << "static VALUE binding_"
    << klass.binding_name() << '_' << method.binding_name() << '('
    << "VALUE self" << binding_params_for(method) << ')' << endl
    << '{' << endl
    << "  Ruby::Constant os(\"ObjectSpace\");" << endl
    << "  std::stringstream stream;" << endl
    << "  RarityClass* instance = new " << klass.full_name << '('
    << binding_params_apply(method) << ");" << endl
    << "  Ruby::Object proc;" << endl
    << endl
    << "  instance->set_ruby_instance(self);" << endl
    << "  stream << reinterpret_cast<long>(instance);" << endl
    << "  proc = Ruby::evaluate(\"Proc.new do (" << ruby_class_getter(klass) << ".finalize \" + stream.str() + \") end\");" << endl
    << "  os.apply(\"define_finalizer\", 2, instance, &proc);" << endl
    << "  return Qnil;" << endl
    << '}' << endl;
}

void Renderer::generate_static_method_binding(const ClassDefinition& klass, const MethodDefinition& method)
{
  source
    << "static VALUE binding_"
    << klass.binding_name() << '_' << method.binding_name() << '('
    << "VALUE self" << binding_params_for(method) << ')' << endl
    << '{' << endl;
  source << "  ";
  if (method.return_type)
    source << (*method.return_type) << " ret = ";
  source
    << klass.full_name << "::" << method.name
    << '(' << binding_params_apply(method) << ");" << endl
    << "  return " << (method.return_type ? "::cpp_to_ruby(ret);" : "Qnil;") << endl
    << '}' << endl;
}

void Renderer::generate_method_binding(const ClassDefinition& klass, const MethodDefinition& method)
{
  source
    << "static VALUE binding_"
    << klass.binding_name() << '_' << method.binding_name() << '('
    << "VALUE self" << binding_params_for(method) << ')' << endl
    << '{' << endl
    << "  long _ptr = Ruby::get_instance_pointer(self);" << endl
    << "  auto* _this = reinterpret_cast<" << klass.full_name << "*>(_ptr);" << endl
    << endl
    << binding_params_check(klass, method);
  source << "  ";
  if (method.return_type)
    source << (*method.return_type) << " ret = ";
  source
    << "_this->" << method.name << '(' << binding_params_apply(method) << ");" << endl
    << "  return " << (method.return_type ? "::cpp_to_ruby(ret);" : "Qnil;") << endl
    << '}' << endl;
}
