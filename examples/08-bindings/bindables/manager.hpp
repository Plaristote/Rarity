#pragma once
#include "base.hpp"

namespace Cpp
{
  class Manager : public Person
  {
    std::vector<Person*> persons;
  public:
    Manager(const std::string& name) : Person(name, typeid(Manager))
    {
    }

    std::string getRole() const override
    {
      return "manager";
    }

    const std::vector<Cpp::Person*>& getPersons() const
    {
      return persons;
    }

    void addPerson(Cpp::Person* person)
    {
      persons.push_back(person);
    }
  };
}
