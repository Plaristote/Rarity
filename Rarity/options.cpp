#include "options.hpp"
#include <libtwili/parser.hpp>
#include <iostream>

using namespace std;

enum OptionState
{
  InputsOption,
  OutputOption,
  ModuleOption,
  InitializerOption,
  NoOption
};

static void print_help()
{
  cerr << "Options:" << endl
    << "  -i [ --input ] args\tlist of input files or directories" << endl
    << "  -o [ --output ] arg\toutput c++ file" << endl
    << "  -m [ --module ] arg\tname of a module in which the Ruby bindings will be wrapped" << endl
    << "  -f [ --function ] arg\tname of the function initializing the bindings" << endl
    << "  -c [ --clang ] args\tuse this option to start specifying clang's options (include paths, etc)" << endl;
}

bool Options::operator()(int ac, const char** av, RarityParser& parser)
{
  OptionState state = NoOption;
  
  for (int i = 1 ; i < ac ; ++i)
  {
    if (av[i] == string("-h") || av[i] == string("--help"))
    {
      print_help();
      return false;
    }
    else if (av[i] == string("-i") || av[i] == string("--input"))
      state = InputsOption;
    else if (av[i] == string("-o") || av[i] == string("--output"))
      state = OutputOption;
    else if (av[i] == string("-m") || av[i] == string("--module"))
      state = ModuleOption;
    else if (av[i] == string("-f") || av[i] == string("--function"))
      state = InitializerOption;
    else if (av[i] == string("-c") || av[i] == string("--clang"))
    {
      clang_argc = ac - i - 1;
      clang_argv = &av[i + 1];
      break ;
    }
    else 
    {
      switch (state)
      {
      case InputsOption:
        parser.add_directory(string(av[i]));
        break ;
      case OutputOption:
        output_file = av[i];
        state = NoOption;
        break ;
      case ModuleOption:
        module_name = av[i];
        state = NoOption;
        break ;
      case InitializerOption:
	initializer_name = av[i];
	state = NoOption;
      default:
        cerr << "/!\\ Cannot read option \"" << av[i] << '"' << endl;
	print_help();
        return false;
      }
    }
  }
  return true;
}
