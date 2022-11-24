#ifndef  MY_CLASS_HPP
# define MY_CLASS_HPP

# include "rarity.hpp"
# include <string>
# include <iostream>

 class MyClass : public RarityClass
 {
 public:
   struct Some
   {
     static void Thing()
     {
       std::cout << "Called Some::Thing" << std::endl;
     }
   };

   MyClass(const std::string& name, const std::type_info& type = typeid(MyClass)) : RarityClass(type), name(name)
   {
     std::cout << "Initializing the class in C++" << std::endl;
     Ruby::Object self(*this);
     self.apply("get_name");
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

 namespace Bleh
{
 class MyOtherClass : public MyClass
 {
 public:
   MyOtherClass(void) : MyClass("parrot", typeid(MyOtherClass))
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
}

#endif
