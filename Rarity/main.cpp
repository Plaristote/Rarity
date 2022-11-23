#include "parser.hpp"
#include "render.hpp"
#include "options.hpp"
#include <regex>
#include <iostream>

using namespace std;

static void collect_files(const filesystem::path& path, vector<filesystem::path>& files)
{
  static const regex pattern("\\.(h|hpp|hxx)$");
  auto filename = path.filename().string();
  auto match = sregex_iterator(filename.begin(), filename.end(), pattern);

  if (filesystem::is_directory(path))
  {
    for (const auto& entry : filesystem::recursive_directory_iterator(path))
      collect_files(entry.path(), files);
  }
  else if (match != sregex_iterator())
    files.push_back(path);
}

int main(int ac, const char** av)
{
  RarityParser parser;
  Options options;
  vector<filesystem::path> files;

  options.output_file = "rarity-bindings.cpp";
  if (!options(ac, av, parser))
    return -1;
  for (const string& dirpath : parser.get_directories())
    collect_files(filesystem::path(dirpath), files);
  for (const auto& filepath : files)
  {
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
      index,
      filepath.string().c_str(),
      options.clang_argv, options.clang_argc,
      nullptr, 0,
      CXTranslationUnit_None
    );

    cout << "- Importing " << filepath.string() << endl;
    if (unit == nullptr || !parser(unit))
    {
      cerr << "/!\\ Failed to parse file " << filepath.string() << endl;
      return -1;
    }
    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
  }
  {
    auto classes = parser.get_classes();
    auto nses    = parser.get_namespaces();
    Renderer renderer(
      options.module_name,
      options.output_file,
      files,
      classes,
      nses
    );

    renderer.render();
  }
  return 0;
}
