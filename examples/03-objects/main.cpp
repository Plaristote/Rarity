#include <rarity.hpp>
#include <iostream>

int main(int, char**)
{
  Rarity rarity; // Must be instantiated once and only once

  try
  {
    Ruby::Constant string_class("String");
    Ruby::Object string_instance = string_class.apply("new");
    std::string part1("Hello"), part2(", world!");

    string_instance = string_instance.apply("+", 1, Ruby::to_ruby_type(part1).get());
    string_instance = string_instance.apply("+", 1, Ruby::to_ruby_type(part2).get());
    std::cout << "String: " << Ruby::inspect(string_instance) << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Catched exception: " << e.what() << std::endl;
  }
  return 0;
}
