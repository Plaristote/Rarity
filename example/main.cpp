#include "rarity.hpp"
#include "myclass.hpp"

using namespace std;

int main(void)
{
  RarityInitialize(); // Must be called before any construction of RarityClass instances
  try
  {
    MyClass my_class("C++ created MyClass");

    Ruby::Lambda lambda = Ruby::Evaluate("Proc.new { |str| puts \"Properly working lambda: #{str.inspect}\" }");
    std::function<void (std::string)> cpp_lambda(lambda);

    cpp_lambda("C++ call from std::function");
    lambda.Call<void, IRarityClass*>(&lambda);
    lambda.Call<void, std::string>("Coucou tu veux voir ma bite ?");
    lambda.Call<void, MyClass*>(&my_class);

    Ruby::PushIncludePath("./scripts");
    Ruby::Require("test.rb");

    Ruby::Constant my_ruby_class("MyRubyClass");
    Ruby::Object   my_ruby_instance = my_ruby_class.Apply("new");

    my_ruby_instance.Apply("run", 1, &my_class); // Method name, argument count, argument list of pointers to Rarity objects
    my_ruby_instance.Apply("run");

    Ruby::Constant("GC").Apply("start"); // Forces Ruby's garbage collector to start
  }
  catch (const std::exception* e)
  {
    std::cerr << "Catched exception: " << e->what() << std::endl;
  }
  RarityFinalize();
  return (0);
}
