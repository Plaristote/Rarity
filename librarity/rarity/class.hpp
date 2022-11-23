#ifndef  RARITY_CLASS_HPP
# define RARITY_CLASS_HPP

# include "class_interface.hpp"
# include <map>
# include <string>
# include <typeinfo>

class RarityClass : public IRarityClass
{
  static std::map<const std::type_info*, VALUE> ruby_class_map;
public:
  static VALUE ruby_class_for(const std::type_info& type);

  template<typename T>
  static VALUE ruby_class_for()
  {
    return ruby_class_for(typeid(T));
  }

  template<typename T>
  static void register_ruby_class_for(VALUE value)
  {
    ruby_class_map.emplace(&typeid(T), value);
  }

  template<typename T>
  RarityClass(const T*) { initialize_rarity_bindings<T>(); }
  RarityClass(const std::type_info& type) { initialize_rarity_bindings(type); }
  RarityClass(const RarityClass&);
  RarityClass() {}
  ~RarityClass();

  template<typename T>
  void        initialize_rarity_bindings() { initialize_rarity_bindings(typeid(T)); }
  void        initialize_rarity_bindings(const std::type_info&);
  void        initialize_rarity_bindings(VALUE klass);
  VALUE       ruby_type(void) const;
  VALUE       ruby_instance(void) const override { return (ruby_instance_m); }
  void        set_ruby_instance(VALUE val);
  std::string ruby_symbol(void) const;

  operator VALUE() const
  {
    return ruby_instance_m;
  }

//private:
  ID                pointer_symbol;
  ID                name_symbol;
  VALUE             ruby_instance_m = Qnil;
  VALUE             ruby_class = Qnil;
};

#endif
