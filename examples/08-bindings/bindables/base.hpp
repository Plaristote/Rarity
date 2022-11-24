#pragma once
#include "rarity.hpp"
#include <string>

namespace Cpp
{
  class Person : public RarityClass
  {
    std::string name;
  public:
    Person(const std::string& name) : RarityClass(typeid(Person)), name(name)
    {
    }

    Person(const std::string& name, const std::type_info& type) : RarityClass(type), name(name)
    {
    }

    std::string applyOnName(std::function<Ruby::Object(std::string)> callback)
    {
      return Ruby::inspect(callback(name));
    }

    const std::string& getName() const
    {
      return name;
    }

    void setName(const std::string& value)
    {
      name = value;
    }

    virtual std::string getRole() const
    {
      return "none";
    }
  };
}
