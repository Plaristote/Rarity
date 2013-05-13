#ifndef  MY_CLASS_HPP
# define MY_CLASS_HPP

//# define  RARITY_RUBY
#undef RARITY_RUBY
# include "rarity_scripting.hpp"
# include <string>
# include <algorithm>
# include <iostream>

 class MyClass : ScriptBindings
 {
 public:
   struct Some
   {
     static MyClass Thing()
     {
       std::cout << "Called Some::Thing" << std::endl;
       return (MyClass("bite"));
     }

/*     static Ruby::Object RubyAwareMethod(Ruby::Object to_split, Ruby::Object to_split_with)
     {
       return (to_split.Apply("split", 1, &to_split_with));
     }*/
   };

   MyClass(const std::string& name) : script_bindings("MyClass"), name(name)
   {
     std::cout << "Initializing the class in C++" << std::endl;
     //Ruby::Object self(*this);
     //self.Apply("get_name");
     std::cout << "Successfully applied" << std::endl;
   }

   const std::string& GetName() const
   {
     return (name);
   }

   int operator[](int i)
   {
     return (i + name.size());
   }

  void SetName(const std::string& str)
  {
    name = str;
  }

  static void PrintArray(const std::vector<std::string>& array)
  {
    std::cout << "MyClass Static Method" << std::endl;
    std::for_each(array.begin(), array.end(), [](std::string value)
    {
      std::cout << "- " << value << std::endl;
    });
  }

 private:
   std::string name;
 };

 class MyOtherClass : ScriptBindings
 {
 public:
   MyOtherClass(void) : script_bindings("MyOtherClass")
   {
     my_class = 0;
   }

   void         SetMyClass(MyClass* self)
   {
     my_class = self;
   }

   unsigned int PrintName(void) const
   {
     if (my_class != 0)
       std::cout << "MyOtherClass PrintName -> " << my_class->GetName() << std::endl;
     else
       std::cout << "MyOtherClass PrintName -> nil pointer" << std::endl;
     return (42);
   }

 private:
   MyClass* my_class;
 };

#endif
