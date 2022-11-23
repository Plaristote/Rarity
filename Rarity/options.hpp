#pragma once
#include <string>

class RarityParser;

struct Options
{
  std::string  output_file;
  std::string  module_name;
  std::string  initializer_name;
  int          clang_argc;
  const char** clang_argv;

  bool operator()(int ac, const char**av, RarityParser& parser);
};
