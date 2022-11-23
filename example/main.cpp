#include "rarity.hpp"
#include "myclass.hpp"
#include <iostream>
using namespace std;

void initialize_rarity_bindings(void);

int main(void)
{
  Rarity rarity; // Must be instantiated before any construction of RarityClass instances

  initialize_rarity_bindings();
  try
  {
    MyClass my_class("C++ created MyClass");

    Ruby::Lambda                      lambda = Ruby::evaluate("Proc.new { |str| puts \"Properly working lambda: #{str.inspect}\" }");

    lambda.as_function<void, std::string>()("C++ call from std::function");
    lambda.call<void, IRarityClass*>(&lambda);
    lambda.call<void, std::string>("Coucou tu veux voir ma bite ?");
    lambda.call<void, MyClass*>(&my_class);

    Ruby::push_include_path("./scripts");
    if (!(Ruby::require("test.rb")))
    {
      std::cout << "Could not require file" << std::endl;
      return (-1);
    }

    Ruby::Constant my_ruby_class("MyRubyClass");
    Ruby::Object   my_ruby_instance = my_ruby_class.apply("new");

    my_ruby_instance.apply("run", 1, &my_class); // Method name, argument count, argument list of pointers to Rarity objects
    int ret_val = my_ruby_instance.apply("run");

    std::cout << "Properly casted ret_val in int (" << ret_val << ')' << std::endl;

    Ruby::Constant("GC").apply("start"); // Forces Ruby's garbage collector to start
  }
  catch (const std::exception* e)
  {
    std::cerr << "Catched exception: " << e->what() << std::endl;
  }
  return (0);
}
