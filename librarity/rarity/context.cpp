#include "context.hpp"
#include <string>
#include <stdexcept>

bool Rarity::is_initialized = false;

static const std::string program_name("ruby");
static const std::string dummy_command("-eprint ''");

static void set_ruby_arguments(const char** argv, const std::vector<std::string>& options)
{
  argv[0] = program_name.c_str();
  argv[options.size() + 1] = dummy_command.c_str();
  for (int i = 0 ; i < options.size() ; ++i)
    argv[i + 1] = options[i].c_str();
}

Rarity::Rarity(const std::vector<std::string>& options)
{
  const char* argv[options.size() + 2];
  void*       node;
  int         state;

  if (is_initialized)
    throw std::runtime_error("Rarity can only be instanciated once per process.");
  is_initialized = true;
  ruby_init();
  set_ruby_arguments(argv, options);
  node = ruby_options(options.size() + 2, const_cast<char**>(argv));
  if (ruby_executable_node(node, &state))
    state = ruby_exec_node(node);
  if (state)
    throw std::runtime_error("Rarity failed to initialize a Ruby VM");
}

Rarity::~Rarity()
{
  ruby_finalize();
}

void Rarity::protect(std::function<void (void)> block)
{
  int state = 0;

  rb_protect(Rarity::wrapped_protect, reinterpret_cast<VALUE>(&block), &state);
  if (state != 0)
    throw Ruby::Exception();
}

VALUE Rarity::wrapped_protect(VALUE value)
{
  std::function<void (void)>* functor = reinterpret_cast<std::function<void (void)>*>(value);

  (*functor)();
  return (Qnil);
}
