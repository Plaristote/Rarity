#include <rarity.hpp>
#include <iostream>

int main(int, char**)
{
  Rarity rarity; // Must be instantiated once and only once

  try
  {
    {
      Ruby::Lambda lambda = Ruby::evaluate(
        "Proc.new { |param| puts \"Ruby lambda says: #{param.inspect}\" }"
      );
      lambda.call<void, std::string>("Hello, world !");
      lambda.call<void, unsigned int>(42);
    }

    {
      Ruby::Lambda lambda = Ruby::evaluate("Proc.new {|param| param ** param}");
      auto cpp_lambda = Ruby::to_cpp_type<std::function<unsigned int (unsigned int)>>(lambda);
      std::cout << cpp_lambda(3) << std::endl;
    }

    {
      Ruby::Lambda lambda = Ruby::evaluate("Proc.new {|param| puts param.inspect}");
      lambda.call<void, std::vector<int>>({1,2,3});
    }

    {
      Ruby::Lambda lambda = Ruby::evaluate("Proc.new {|param| param.call \"Coucou, petite perruche !\"}");
      std::function<int(std::string)> cpp_lambda = [](std::string str) -> int { return str.length(); };
      auto rubified_lambda = Ruby::Lambda::Factory<int, std::string>::create(cpp_lambda);
      auto length = lambda.call<int, Ruby::Object>(rubified_lambda);
      std::cout << "Length = " << length << " should be " << std::string("Coucou petite perruche").length() << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Catched exception: " << e.what() << std::endl;
  }
  return 0;
}
