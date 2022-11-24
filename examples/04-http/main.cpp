#include <rarity.hpp>
#include <iostream>

int main(int, char**)
{
  Rarity rarity; // Must be instantiated once and only once

  try
  {
    Ruby::require("net/http");
    Ruby::Object net = Ruby::Constant("Net");
    Ruby::Object http = Ruby::Constant("HTTP", net);
    Ruby::Object response = http.apply(
      "get", 2,
      Ruby::to_ruby_type<const std::string>("example.com").get(),
      Ruby::to_ruby_type<const std::string>("/index.html").get()
    );
    std::cout << (std::string)(response) << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Catched exception: " << e.what() << std::endl;
  }
  return 0;
}

