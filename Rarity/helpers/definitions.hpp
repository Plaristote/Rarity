#pragma once
#include <libtwili/definitions.hpp>

std::string binding_name(const NamespaceDefinition& ns);
std::string ruby_name(const NamespaceDefinition& ns);
std::string cpp_context(const NamespaceDefinition& ns);
std::string ruby_context(const NamespaceDefinition& ns);
std::string binding_name(const MethodDefinition& method);
std::string ruby_name(const MethodDefinition& method);
std::string binding_name(const FunctionDefinition& function);
std::string ruby_name(const FunctionDefinition& function);
std::string cpp_context(const FunctionDefinition& function);
std::string ruby_context(const FunctionDefinition& function);
