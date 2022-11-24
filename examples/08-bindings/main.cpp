#include "bindables/base.hpp"
#include "bindables/manager.hpp"
#include <filesystem>
#include <iostream>

#include "rarity/hash.hpp"

void initialize_rarity_bindings(void);

int main(int ac, char** av)
{
  Rarity rarity;

  if (ac != 2)
  {
    std::cerr << "usage: " << av[0] << " [script.rb]" << std::endl;
    return -1;
  }
  initialize_rarity_bindings();
  try
  {
    auto path = std::filesystem::canonical(av[1]);

    Rarity::protect([&]()
    {
      Ruby::push_include_path(path.parent_path().string());
      Ruby::require(path.filename().string());

      Ruby::Constant my_ruby_class("MyRubyClass");
      Ruby::Object my_ruby_instance = my_ruby_class.apply("new");
      std::function<void(std::string)> done = [](std::string value)
      {
        std::cout << "MyRubyClass calling `done` callback with message: " << value << std::endl;
      };

      std::cout << "Step #1" << std::endl;
      my_ruby_instance.apply("run");
      std::cout << "Step #2" << std::endl;
      my_ruby_instance.apply("run", 1, Ruby::to_ruby_type(done).get());
    });
  }
  catch (const Ruby::Exception& e)
  {
    std::cerr << "Ruby Error: " << e << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
