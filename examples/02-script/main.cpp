#include <rarity.hpp>
#include <iostream>
#include <filesystem>

int main(int argc, char** argv)
{
  Rarity rarity; // Must be instantiated once and only once

  if (argc != 2)
  {
    std::cerr << "usage: " << argv[0] << " [file.rb]" << std::endl;
    return -1;
  }
  try
  {
    auto path = std::filesystem::canonical(argv[1]);

    Ruby::push_include_path(path.parent_path().string());
    if (Ruby::require(path.filename().string()))
      std::cout << "The deed is done" << std::endl;
  }
  catch (const Ruby::Exception& e)
  {
    std::cerr << "Script " << argv[1] << " threw an exception: " << e << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Catched exception: " << e.what() << std::endl;
  }
  return 0;
}
