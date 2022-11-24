#include <rarity.hpp>
#include <iostream>

int main(int, char**)
{
  Rarity rarity; // Must be instantiated once and only once

  try
  {
    std::vector<std::string> tokens;

    tokens.push_back("Hello,");
    tokens.push_back("world!");
    Ruby::Array array(tokens);

    array.insert(array.end(), Ruby::evaluate("\"How do you do ?\""));
    array.push_back(*Ruby::to_ruby_type<const std::string>("Fine thank you"));

    for (auto it = array.begin() ; it != array.end() ; ++it)
      std::cout << "- " << Ruby::inspect(*it) << std::endl;

    for (int i = 0 ; i < array.size() ; ++i)
      std::cout << "index[" << i << "] = " << Ruby::inspect(array[i]) << std::endl;

    tokens = array.as_vector<std::string>();

    for (const std::string& entry : tokens)
      std::cout << "std::vector -> " << entry << std::endl;

    const auto list = Ruby::to_cpp_type<std::list<std::string>>(array);
    for (const std::string& entry : list)
      std::cout << "std::list -> " << entry << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Catched exception: " << e.what() << std::endl;
  }
  return 0;
}
