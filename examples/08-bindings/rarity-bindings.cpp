#include <rarity.hpp>
#include <string>
#include <sstream>
#include "bindables/manager.hpp"
#include "bindables/base.hpp"

static VALUE wrapping_module;

template<typename T>
static VALUE cpp_to_ruby(T var)
{
  auto rarity_class = Ruby::to_ruby_type<T>(var);
  VALUE value = rarity_class->ruby_instance();

  return value;
}

typedef VALUE (*RubyMethod)(...);

static VALUE binding_____Cpp____Person_Person(VALUE self, VALUE param_0)
{
  Ruby::Constant os("ObjectSpace");
  std::stringstream stream;
  RarityClass* instance = new ::Cpp::Person((Ruby::to_cpp_type<std::string>(param_0)));
  Ruby::Object proc;

  instance->set_ruby_instance(self);
  stream << reinterpret_cast<long>(instance);
  proc = Ruby::evaluate("Proc.new do (::Cpp::Person.finalize " + stream.str() + ") end");
  os.apply("define_finalizer", 2, instance, &proc);
  return Qnil;
}
static VALUE binding_____Cpp____Person_applyOnName(VALUE self, VALUE param_0)
{
  long _ptr = Ruby::get_instance_pointer(self);
  auto* _this = reinterpret_cast<::Cpp::Person*>(_ptr);

  {
    Ruby::Object tmp(param_0);
    const std::string tmp_typename = (Ruby::to_cpp_type<std::string>(tmp.apply("class").apply("name")));
    if (tmp_typename != "Proc")
      rb_raise((Ruby::Constant("ArgumentError").ruby_instance()), ("Mismatched type in Person::applyOnName, argument 1. Expecting Proc, got " + tmp_typename + ".").c_str());
  }
  std::string ret = _this->applyOnName((Ruby::to_cpp_type<std::function<Ruby::Object (std::string)>>(param_0)));
  return ::cpp_to_ruby(ret);
}
static VALUE binding_____Cpp____Person_getName(VALUE self)
{
  long _ptr = Ruby::get_instance_pointer(self);
  auto* _this = reinterpret_cast<::Cpp::Person*>(_ptr);

  std::string ret = _this->getName();
  return ::cpp_to_ruby(ret);
}
static VALUE binding_____Cpp____Person_setName(VALUE self, VALUE param_0)
{
  long _ptr = Ruby::get_instance_pointer(self);
  auto* _this = reinterpret_cast<::Cpp::Person*>(_ptr);

  {
    Ruby::Object tmp(param_0);
    const std::string tmp_typename = (Ruby::to_cpp_type<std::string>(tmp.apply("class").apply("name")));
    if (tmp_typename != "String")
      rb_raise((Ruby::Constant("ArgumentError").ruby_instance()), ("Mismatched type in Person::setName, argument 1. Expecting String, got " + tmp_typename + ".").c_str());
  }
  _this->setName((Ruby::to_cpp_type<std::string>(param_0)));
  return Qnil;
}
static VALUE binding_____Cpp____Person_getRole(VALUE self)
{
  long _ptr = Ruby::get_instance_pointer(self);
  auto* _this = reinterpret_cast<::Cpp::Person*>(_ptr);

  std::string ret = _this->getRole();
  return ::cpp_to_ruby(ret);
}
static VALUE binding_____Cpp____Person__initialize(VALUE self) { return Qnil; }

static void initialize_____Cpp____Person()
{
  VALUE inherits = rb_cObject;
  // ruby context = ::Cpp::Person -> ::Cpp
  VALUE wrapped = Ruby::Constant("Cpp", wrapping_module);
  VALUE klass = rb_define_class_under(wrapped, "Person", inherits);

  RarityClass::register_ruby_class_for<::Cpp::Person>(klass);
  rb_define_method(klass, "_initialize", reinterpret_cast<RubyMethod>(binding_____Cpp____Person__initialize), 0);
  rb_define_module_function(klass, "finalize", reinterpret_cast<RubyMethod>(Ruby::finalize_rarity_class), 1);
  rb_define_method(klass, "initialize", reinterpret_cast<RubyMethod>(binding_____Cpp____Person_Person), 1);
  rb_define_method(klass, "apply_on_name", reinterpret_cast<RubyMethod>(binding_____Cpp____Person_applyOnName), 1);
  rb_define_method(klass, "get_name", reinterpret_cast<RubyMethod>(binding_____Cpp____Person_getName), 0);
  rb_define_method(klass, "set_name", reinterpret_cast<RubyMethod>(binding_____Cpp____Person_setName), 1);
  rb_define_method(klass, "get_role", reinterpret_cast<RubyMethod>(binding_____Cpp____Person_getRole), 0);
}
static VALUE binding_____Cpp____Manager_Manager(VALUE self, VALUE param_0)
{
  Ruby::Constant os("ObjectSpace");
  std::stringstream stream;
  RarityClass* instance = new ::Cpp::Manager((Ruby::to_cpp_type<std::string>(param_0)));
  Ruby::Object proc;

  instance->set_ruby_instance(self);
  stream << reinterpret_cast<long>(instance);
  proc = Ruby::evaluate("Proc.new do (::Cpp::Manager.finalize " + stream.str() + ") end");
  os.apply("define_finalizer", 2, instance, &proc);
  return Qnil;
}
static VALUE binding_____Cpp____Manager_getRole(VALUE self)
{
  long _ptr = Ruby::get_instance_pointer(self);
  auto* _this = reinterpret_cast<::Cpp::Manager*>(_ptr);

  std::string ret = _this->getRole();
  return ::cpp_to_ruby(ret);
}
static VALUE binding_____Cpp____Manager_getPersons(VALUE self)
{
  long _ptr = Ruby::get_instance_pointer(self);
  auto* _this = reinterpret_cast<::Cpp::Manager*>(_ptr);

  std::vector<Cpp::Person *> ret = _this->getPersons();
  return ::cpp_to_ruby(ret);
}
static VALUE binding_____Cpp____Manager_addPerson(VALUE self, VALUE param_0)
{
  long _ptr = Ruby::get_instance_pointer(self);
  auto* _this = reinterpret_cast<::Cpp::Manager*>(_ptr);

  {
    Ruby::Object tmp(param_0);
    const std::string tmp_typename = (Ruby::to_cpp_type<std::string>(tmp.apply("class").apply("name")));
    if (tmp_typename != "Cpp::Person")
      rb_raise((Ruby::Constant("ArgumentError").ruby_instance()), ("Mismatched type in Manager::addPerson, argument 1. Expecting Cpp::Person, got " + tmp_typename + ".").c_str());
  }
  _this->addPerson((Ruby::to_cpp_type<Cpp::Person*>(param_0)));
  return Qnil;
}
static VALUE binding_____Cpp____Manager__initialize(VALUE self) { return Qnil; }

static void initialize_____Cpp____Manager()
{
  VALUE inherits = RarityClass::ruby_class_for<::Cpp::Person>();
  // ruby context = ::Cpp::Manager -> ::Cpp
  VALUE wrapped = Ruby::Constant("Cpp", wrapping_module);
  VALUE klass = rb_define_class_under(wrapped, "Manager", inherits);

  RarityClass::register_ruby_class_for<::Cpp::Manager>(klass);
  rb_define_method(klass, "_initialize", reinterpret_cast<RubyMethod>(binding_____Cpp____Manager__initialize), 0);
  rb_define_module_function(klass, "finalize", reinterpret_cast<RubyMethod>(Ruby::finalize_rarity_class), 1);
  rb_define_method(klass, "initialize", reinterpret_cast<RubyMethod>(binding_____Cpp____Manager_Manager), 1);
  rb_define_method(klass, "get_role", reinterpret_cast<RubyMethod>(binding_____Cpp____Manager_getRole), 0);
  rb_define_method(klass, "get_persons", reinterpret_cast<RubyMethod>(binding_____Cpp____Manager_getPersons), 0);
  rb_define_method(klass, "add_person", reinterpret_cast<RubyMethod>(binding_____Cpp____Manager_addPerson), 1);
}
void initialize_rarity_bindings(void)
{
  wrapping_module = rb_cObject;
  rb_define_module_under(wrapping_module, "Cpp");
  initialize_____Cpp____Person();
  initialize_____Cpp____Manager();
}
