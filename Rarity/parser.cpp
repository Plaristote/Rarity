#include "parser.hpp"
#include <iostream>
#include <functional>
#include <sstream>
#include <crails/utils/split.hpp>

using namespace std;

ostream& operator<<(ostream& stream, const CXString& str)
{
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

string cxStringToStdString(const CXString& source)
{
  string result(clang_getCString(source));
  clang_disposeString(source);
  return result;
}

// Edited from Khuck's response in
// https://stackoverflow.com/questions/62005698/libclang-clang-getargtype-returns-wrong-type
static bool find_parsing_errors(CXTranslationUnit translationUnit)
{
  int nbDiag = clang_getNumDiagnostics(translationUnit);
  bool foundError = false;

  if (nbDiag)
    cerr << "There are " << nbDiag << " diagnostics:" << endl;
  for (unsigned int currentDiag = 0 ; currentDiag < nbDiag ; ++currentDiag)
  {
    CXDiagnostic diagnotic = clang_getDiagnostic(translationUnit, currentDiag);
    string tmp = cxStringToStdString(
      clang_formatDiagnostic(diagnotic, clang_defaultDiagnosticDisplayOptions())
    );

    if (tmp.find("error:") != string::npos)
      foundError = true;
    cerr << tmp << endl;
  }
  return foundError;
}

class RarityParser;

RarityParser* parser = nullptr;

RarityParser::RarityParser()
{
  parser = this;
}

RarityParser::~RarityParser()
{
  parser = nullptr;
}

void RarityParser::add_directory(const string& path)
{
  add_directory(filesystem::path(path));
}

void RarityParser::add_directory(const filesystem::path& path)
{
  directories.push_back(filesystem::canonical(path).string());
}

filesystem::path RarityParser::get_current_path() const
{
  CXSourceLocation location = clang_getCursorLocation(cursor);
  CXFile cursorFile;

  clang_getExpansionLocation(location, &cursorFile, nullptr, nullptr, nullptr);
  return filesystem::path(cxStringToStdString(clang_File_tryGetRealPathName(cursorFile)));
}

bool RarityParser::is_included(const std::filesystem::path& path) const
{
  for (const std::string& directory : directories)
  {
    if (path.string().find(directory) != std::string::npos)
      return true;
  }
  return false;
}

bool RarityParser::has_class(const std::string& class_name) const
{
  return std::find(classes.begin(), classes.end(), class_name) != classes.end();
}

std::vector<ClassDefinition> RarityParser::get_classes() const
{
  std::vector<ClassDefinition> result;

  for (const auto& entry : classes) result.push_back(entry.klass);
  return result;
}

std::vector<NamespaceDefinition> RarityParser::get_namespaces() const
{
  std::vector<NamespaceDefinition> result;

  for (const auto& entry : namespaces) result.push_back(entry.ns);
  return result;
}

optional<string> RarityParser::fullname_for(CXCursor cursor) const
{
  auto ns_it = std::find(namespaces.begin(), namespaces.end(), cursor);
  auto class_it = std::find(classes.begin(), classes.end(), cursor);

  if (ns_it != namespaces.end())
    return ns_it->ns.full_name;
  else if (class_it != classes.end())
    return class_it->klass.full_name;
  return optional<string>();
}

RarityParser::ClassContext* RarityParser::find_class_for(CXCursor cursor)
{
  auto it = std::find(classes.begin(), classes.end(), cursor);

  return it != classes.end() ? &(*it) : nullptr;
}

RarityParser::ClassContext* RarityParser::find_class_by_name(const std::string& full_name)
{
  auto it = std::find(classes.begin(), classes.end(), full_name);

  return it != classes.end() ? &(*it) : nullptr;
}

RarityParser::ClassContext* RarityParser::find_class_like(const std::string& symbol_name, const std::string& cpp_context)
{
  auto match = std::find(classes.begin(), classes.end(), cpp_context + "::" + symbol_name);

  if (match == classes.end()) // not an exact match
  {
    auto parts = Crails::split(cpp_context, ':');

    do
    {
      string parent_context;
      parts.remove(*parts.rbegin());
      for (const auto& part : parts) parent_context += "::" + part;
      match = std::find(classes.begin(), classes.end(), parent_context + "::" + symbol_name);
    }
    while (match == classes.end() && parts.size() > 0);
  }
  return match != classes.end() ? &(*match) : nullptr;
}

bool RarityParser::operator()(CXTranslationUnit& unit)
{
  clang_visitChildren(
    clang_getTranslationUnitCursor(unit),
    &RarityParser::visitor_callback,
    nullptr
  );
  if (!find_parsing_errors(unit))
  {
    cout << "Bindings will be generated for the following classes:" << endl;
    for (const auto& klass : classes)
      cout << " - " << klass.klass.full_name << endl;
    return true;
  }
  return false;
}

CXChildVisitResult RarityParser::visit_namespace(const std::string& symbol_name, CXCursor parent)
{
  auto base_name = fullname_for(parent);
  auto full_name = (base_name ? *base_name : string()) + "::" + symbol_name;
  auto it = std::find(namespaces.begin(), namespaces.end(), full_name);

  if (it == namespaces.end())
  {
    NamespaceContext ns_context;

    ns_context.cursors.push_back(cursor);
    ns_context.ns.name = symbol_name;
    ns_context.ns.full_name = full_name;
    namespaces.push_back(ns_context);
  }
  else
    it->cursors.push_back(cursor);
  return CXChildVisit_Recurse;
}

CXChildVisitResult RarityParser::visit_class(const std::string& symbol_name, CXCursor parent)
{
  auto kind = clang_getCursorKind(cursor);
  ClassContext new_class;
  ClassContext* parent_class;
  ClassContext* existing_class;

  new_class.klass.name = symbol_name;
  new_class.cursors.push_back(cursor);
  new_class.current_access = kind == CXCursor_StructDecl ? CX_CXXPublic : CX_CXXPrivate;
  if (parent.kind == CXCursor_TranslationUnit)
    new_class.klass.full_name = "::" + symbol_name;
  else if ((parent_class = find_class_for(parent)))
  {
    if (parent_class->current_access != CX_CXXPublic)
      return CXChildVisit_Continue;
    new_class.klass.full_name = parent_class->klass.full_name + "::" + symbol_name;
  }
  else
  {
    auto context_fullname = fullname_for(parent);

    if (context_fullname)
      new_class.klass.full_name = *context_fullname + "::" + symbol_name;
    else
    {
      cerr << "(!) Couldn't find context for class " << symbol_name << endl;
      return CXChildVisit_Continue;
    }
  }
  existing_class = find_class_by_name(new_class.klass.full_name);
  if (existing_class != nullptr)
  {
    existing_class->cursors.push_back(cursor);
    return CXChildVisit_Continue;
  }
  classes.push_back(new_class);
  return CXChildVisit_Recurse;
}

void RarityParser::visit_base_class(ClassContext& current_class, const std::string& cursor_text)
{
  std::string symbol_name = cursor_text.find("class ")
    ? cursor_text.substr(5)
    : (cursor_text.find("struct ") ? cursor_text.substr(6) : cursor_text);
  ClassContext* base_class = find_class_like(symbol_name, current_class.klass.full_name);

  if (base_class)
    current_class.klass.cpp_base = base_class->klass.full_name;
  else
    cout << "(i) " << current_class.klass.full_name << " base class " << symbol_name << " cannot be solved" << endl;
}

MethodDefinition RarityParser::visit_method(const std::string& symbol_name, CXCursor)
{
  MethodDefinition new_method;
  CXType method_type = clang_getCursorType(cursor);
  CXType return_type = clang_getResultType(method_type);
  CXType arg_type;

  new_method.name = symbol_name;
  new_method.is_static = clang_CXXMethod_isStatic(cursor);
  if (return_type.kind != 0 && return_type.kind != CXType_Void)
    new_method.return_type = ParamDefinition(return_type);
  for (int i = 0 ; (arg_type = clang_getArgType(method_type, i)).kind != 0 ; ++i)
    new_method.params.push_back(ParamDefinition(arg_type));
  //cout << "  -> with method `" << new_method.name << "`\n";
  //for (const auto& param : new_method.params)
  //  cout << "    -> with param " << param << endl;
  return new_method;
}

CXChildVisitResult RarityParser::visitor(CXCursor parent, CXClientData)
{
  if (is_included(get_current_path()))
  {
    auto kind = clang_getCursorKind(cursor);
    string symbol_name = cxStringToStdString(clang_getCursorSpelling(cursor));

    if (kind == CXCursor_Namespace)
      return visit_namespace(symbol_name, parent);
    if (kind == CXCursor_StructDecl || kind == CXCursor_ClassDecl)
      return visit_class(symbol_name, parent);
    else
    {
      ClassContext* current_class = find_class_for(parent);

      if (current_class)
      {
        switch (kind)
        {
        case CXCursor_CXXBaseSpecifier:
          visit_base_class(*current_class, symbol_name);
          break ;
        case CXCursor_CXXAccessSpecifier:
          current_class->current_access = clang_getCXXAccessSpecifier(cursor);
          break ;
        case CXCursor_CXXMethod:
          if (current_class->current_access == CX_CXXPublic)
            current_class->klass.methods.push_back(visit_method(symbol_name, parent));
          return CXChildVisit_Continue;
        case CXCursor_Constructor:
          if (!current_class->klass.constructor && current_class->current_access == CX_CXXPublic)
            current_class->klass.constructor = visit_method(symbol_name, parent);
          return CXChildVisit_Continue;
        default:
          //cout << "  Unhandled decl: " << clang_getCursorKindSpelling(clang_getCursorKind(cursor)) << " -> " <<  symbol_name << endl;
          break ;
        }
        return CXChildVisit_Recurse;
      }
      //else
      //  cout << "Unhandled decl: " << clang_getCursorKindSpelling(clang_getCursorKind(cursor)) << " -> " <<  symbol_name << endl;
    }
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult RarityParser::visitor_callback(CXCursor c, CXCursor parent, CXClientData clientData)
{
  parser->cursor = c;
  return parser->visitor(parent, clientData);
}