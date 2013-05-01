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

  void SetName(const std::string& str)
  {
    name = str;
  }

 private:
   std::string name;
 }; 

#endif
