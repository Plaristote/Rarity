#ifndef  RARITY_HPP
# define RARITY_HPP

# include "ruby.h"
# include <string>
# include <iostream>

void RarityInitialize(void);
void RarityFinalize(void);

namespace Ruby
{
  void        PushIncludePath(const std::string& path);
  bool        Require(const std::string& path);
  VALUE       Evaluate(const std::string& code);
  const char* Inspect(VALUE value);
}

/*
 * RarityClass
 */
struct IRarityClass
{
  virtual VALUE GetRubyInstance(void) const = 0;
};

class RarityType : public IRarityClass
{
  union Content
  {
    const char* string;
    int         number;
    float       floating_number;
    bool        boolean;
  };

  enum ContentType
  {
    String, Number, FloatingNumber, Boolean
  };

public:
  RarityType(const std::string& str)
  {
    content.string = str.c_str();
    content_type   = String;
  }

  RarityType(const char* str)
  {
    content.string = str;
    content_type   = String;
  }

  RarityType(int number)
  {
    content.number = number;
    content_type   = Number;
  }

  RarityType(float floating_number)
  {
    content.floating_number = floating_number;
    content_type            = FloatingNumber;
  }

  RarityType(bool boolean)
  {
    content.boolean = boolean;
    content_type    = Boolean;
  }

  VALUE GetRubyInstance(void) const
  {
    switch (content_type)
    {
    case String:
      return (rb_str_new2(content.string));
    case Number:
      return (INT2NUM(content.number));
    case FloatingNumber:
      return (NUM2DBL(content.floating_number));
    case Boolean:
      return (content.boolean ? Qtrue : Qfalse);
    }
    return (Qnil);
  }

private:
  Content     content;
  ContentType content_type;
};

class RarityClass : public IRarityClass
{
public:
  RarityClass(const std::string& classname)
  {
    name_symbol    = rb_intern(classname.c_str());
    ruby_class     = rb_const_get(rb_cObject, name_symbol);
    {
      rb_define_alias(ruby_class, "_real_initialize", "initialize");
      rb_define_alias(ruby_class, "initialize", "_initialize");

      ruby_instance  = rb_class_new_instance(0, 0, ruby_class);
      pointer_symbol = rb_intern("@rarity_cpp_pointer");

      rb_define_alias(ruby_class, "initialize", "_real_initialize");

      rb_ivar_set(ruby_instance, pointer_symbol, INT2FIX((long)this));
      VALUE ruby_ptr = rb_ivar_get(ruby_instance, pointer_symbol);
      if (NUM2LONG(ruby_ptr) != (long)this)
        std::cout << "MY PRIMARY FUNCTION IS FAILURE" << std::endl;
    }
  }

  ~RarityClass(void)
  {
    rb_ivar_set(ruby_instance, pointer_symbol, Qnil);
  }

  VALUE GetRubyType(void) const
  {
    return (rb_funcall2(ruby_instance, rb_intern("class"), 0, 0));
  }

  VALUE GetRubyInstance(void) const { return (ruby_instance); }

  operator VALUE() const
  {
    return (ruby_instance);
  }

private:
  ID        pointer_symbol;
  ID        name_symbol;
  VALUE     ruby_instance;
  VALUE     ruby_class;
};

# include "rarity_object.hpp"

#endif
