#ifndef  MY_CLASS_HPP
# define MY_CLASS_HPP

# include "rarity.hpp"
# include <string>

 class MyClass : public RarityClass
 {
 public:
   MyClass(const std::string& name) : RarityClass("MyClass"), name(name)
   {
     std::cout << "Initializing the class in C++" << std::endl;
     Ruby::Object self(*this);
     self.Apply("get_name");
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

  void PrintArray(const std::vector<std::string>& array)
  {
    std::cout << name << " printing array" << std::endl;
    std::for_each(array.begin(), array.end(), [](std::string value)
    {
      std::cout << "- " << value << std::endl;
    });
  }

 private:
   std::string name;
 };

 class MyOtherClass : public RarityClass
 {
 public:
   MyOtherClass(void) : RarityClass("MyOtherClass")
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
