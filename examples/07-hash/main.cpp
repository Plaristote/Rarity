#include "rarity.hpp"
#include "rarity/hash.hpp"
#include <iostream>

int main(int ac, char** av)
{
  Rarity rarity;

  try
  {
    Rarity::protect([&]()
    {
      Ruby::Hash hash;
      const std::string key("khey");

      hash.emplace(key, "oeil");
      hash.emplace(42, "raiponce");
      std::cout << "Inspected hash: " << Ruby::inspect(hash) << std::endl;
      std::cout << "Hash size: " << hash.size() << std::endl;
      std::cout << "Includes 42 key ? " << hash.contains(42) << std::endl;
      std::cout << "Hash contents as Ruby::Hash:" << std::endl;
      for (auto it = hash.begin() ; it != hash.end() ; ++it)
        std::cout << '\t' << Ruby::inspect(it.key()) << " -> " << Ruby::inspect(it.value()) << std::endl;
      std::cout << "Hash contents as std::map:" << std::endl;
      for (const auto& it : hash.to_map<std::string, std::string>())
        std::cout << '\t' << it.first << " -> " << it.second << std::endl;
      std::cout << "Hash::at<std::string>(): " << hash.at<std::string>(key) << std::endl;
    });
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}

