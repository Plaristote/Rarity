#pragma once
#include "definitions.hpp"
#include <filesystem>
#include <optional>

class RarityParser
{
  struct ClassContext
  {
    ClassDefinition       klass;
    CX_CXXAccessSpecifier current_access;
    CXCursor              cursor;
    bool operator==(const std::string& value) const { return klass.full_name == value; }
    bool operator==(const CXCursor value) const { return clang_equalCursors(cursor, value) != 0; }
  };

  struct NamespaceContext
  {
    NamespaceDefinition ns;
    CXCursor            cursor;
    ClassContext        current_class;
    bool operator==(const std::string& value) const { return ns.full_name == value; }
    bool operator==(const CXCursor value) const { return clang_equalCursors(cursor, value) != 0; }
  };

  std::vector<std::string>      directories;
  std::vector<ClassContext>     classes;
  std::vector<NamespaceContext> namespaces;
  NamespaceContext              current_ns;
  NamespaceDefinition           root_ns;
  CXCursor                      cursor;
public:
  RarityParser();
  RarityParser(const RarityParser&) = delete;
  ~RarityParser();

  void add_directory(const std::string& path);
  void add_directory(const std::filesystem::path& path);
  const std::vector<std::string>& get_directories() const { return directories; }
  std::vector<ClassDefinition> get_classes() const;
  std::vector<NamespaceDefinition> get_namespaces() const;

  std::filesystem::path get_current_path() const;
  bool                  is_included(const std::filesystem::path& path) const;
  bool                  has_class(const std::string& class_name) const;
  bool                  operator()(CXTranslationUnit& unit);

private:
  static CXChildVisitResult visitor_callback(CXCursor c, CXCursor parent, CXClientData clientData);

  CXChildVisitResult visitor(CXCursor parent, CXClientData clientData);
  CXChildVisitResult visit_class(const std::string& symbol_name, CXCursor parent);
  MethodDefinition   visit_method(const std::string& symbol_name, CXCursor parent);
  void               visit_base_class(ClassContext& class_context, const std::string& symbol_name);
  CXChildVisitResult visit_namespace(const std::string& symbol_name, CXCursor parent);

  std::optional<std::string> fullname_for(CXCursor) const;
  ClassContext* find_class_for(CXCursor);
  ClassContext* find_class_like(const std::string& symbol_name, const std::string& cpp_context);
};
