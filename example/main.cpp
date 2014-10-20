#include "rarity.hpp"
#include "myclass.hpp"

using namespace std;

int main(void)
{
  RarityInitialize(); // Must be called before any construction of RarityClass instances

  try
  {
    MyClass my_class("C++ created MyClass");

    Ruby::Lambda                      lambda = Ruby::Evaluate("Proc.new { |str| puts \"Properly working lambda: #{str.inspect}\" }");

    lambda.as_function<void, std::string>()("C++ call from std::function");
    lambda.Call<void, IRarityClass*>(&lambda);
    lambda.Call<void, std::string>("Coucou tu veux voir ma bite ?");
    lambda.Call<void, MyClass*>(&my_class);

    Ruby::PushIncludePath("./scripts");
    if (!(Ruby::Require("test.rb")))
    {
      std::cout << "Could not require file" << std::endl;
      return (-1);
    }

    Ruby::Constant my_ruby_class("MyRubyClass");
    Ruby::Object   my_ruby_instance = my_ruby_class.Apply("new");

    my_ruby_instance.Apply("run", 1, &my_class); // Method name, argument count, argument list of pointers to Rarity objects
    int ret_val = my_ruby_instance.Apply("run");

    std::cout << "Properly casted ret_val in int (" << ret_val << ')' << std::endl;

    Ruby::Constant("GC").Apply("start"); // Forces Ruby's garbage collector to start
  }
  catch (const std::exception* e)
  {
    std::cerr << "Catched exception: " << e->what() << std::endl;
  }
  RarityFinalize();
  return (0);
}
