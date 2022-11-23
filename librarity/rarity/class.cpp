#include "class.hpp"
#include "context.hpp"

using namespace std;

map<const type_info*, VALUE> RarityClass::ruby_class_map;
  
VALUE RarityClass::ruby_class_for(const std::type_info& type)
{
  auto it = ruby_class_map.find(&type);
  return it != ruby_class_map.end() ? it->second : Qnil;
}

RarityClass::RarityClass(const RarityClass& copy)
{
  if (ruby_class != Qnil)
    initialize_rarity_bindings(copy.ruby_class);
}

RarityClass::~RarityClass()
{
  if (ruby_instance_m != Qnil)
  {
    rb_ivar_set(ruby_instance_m, pointer_symbol, Qnil);
    rb_gc_unregister_address(&ruby_instance_m);
  }
}

void RarityClass::initialize_rarity_bindings(const std::type_info& type)
{
  initialize_rarity_bindings(
    ruby_class_for(type)
  );
}

void RarityClass::initialize_rarity_bindings(VALUE klass)
{
  if (ruby_instance_m == Qnil && klass != Qnil)
  {
    ruby_class = klass;
    rb_gc_register_address(&ruby_instance_m);
    Rarity::protect([this]()
    {
      if (ruby_class != Qnil)
      {
        rb_define_alias(ruby_class, "_real_initialize", "initialize");
        rb_define_alias(ruby_class, "initialize", "_initialize");

        ruby_instance_m = rb_class_new_instance(0, 0, ruby_class);
        pointer_symbol  = rb_intern("@rarity_cpp_pointer");

        rb_define_alias(ruby_class, "initialize", "_real_initialize");

        set_ruby_instance(ruby_instance_m);
      }
    });
  }
}

VALUE RarityClass::ruby_type(void) const
{
  return rb_funcall2(ruby_instance_m, rb_intern("class"), 0, 0);
}

void RarityClass::set_ruby_instance(VALUE val)
{
  ruby_instance_m = val;
  rb_ivar_set(ruby_instance_m, pointer_symbol, INT2FIX((long)this));
}

std::string RarityClass::ruby_symbol(void) const
{
  return Ruby::Object(ruby_class).apply("name");
}
