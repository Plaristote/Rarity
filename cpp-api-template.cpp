#include "rarity.hpp"
<% includes.each do |header| %>#include "<%= header %>"
<% end %>
#include <string>
#include <sstream>

using namespace std;

static VALUE get_class_by_name(const std::string& classname)
{
  VALUE arg_classname = rb_str_new2(classname.c_str());

  return (rb_funcall2(rb_cObject,
                      rb_intern("const_get"),
                      1,
                      &arg_classname));
}

static long get_instance_pointer(VALUE self)
{
  VALUE variable = rb_ivar_get(self, rb_intern("@rarity_cpp_pointer"));

  // throw something if variable is nil
  return (NUM2LONG(variable));
}

static VALUE RarityClassFinalize(VALUE self)
{
  long ptr = NUM2LONG(self);

  delete ((RarityClass*)ptr);
  return (Qnil);
}

template<typename ExpectedType>
static ExpectedType* api_param(VALUE self, const std::string& name)
{
  long         ptr             = get_instance_pointer(self);
  RarityClass* rarity_instance = reinterpret_cast<RarityClass*>(ptr);

  if (get_class_by_name(name) != rarity_instance->GetRubyType())
  {
    // Throw ArgumentError
    return (0);
  }
  return (reinterpret_cast<ExpectedType*>(rarity_instance));
}

namespace RubyToCpp
{
  std::string  String(VALUE value)      { return (std::string(RSTRING_PTR(value))); }
  bool         Bool(VALUE value)        { return (value == Qtrue);  }
  float        Float(VALUE value)       { return (NUM2DBL(value));  }
  int          Int(VALUE value)         { return (NUM2INT(value));  }
  unsigned int UnsignedInt(VALUE value) { return (NUM2UINT(value)); }
  long         Long(VALUE value)        { return (NUM2LONG(value)); }
  double       Double(VALUE value)      { return (NUM2LONG(value)); }
}

namespace CppToRuby
{
  VALUE String(const std::string& value) { return (rb_str_new2(value.c_str())); }
  VALUE Bool(bool value)                 { return (value ? Qtrue : Qfalse); }
  VALUE Float(float value)               { return (rb_float_new(value)); }
  VALUE Int(int value)                   { return (INT2NUM(value)); }
  VALUE UnsignedInt(unsigned int value)  { return (UINT2NUM(value)); }
}

typedef VALUE (*RubyMethod)(...);

<% classes.each do |classname, struct| %>
  <% has_constructor = false %>

  <% struct['methods'].each do |method, desc| %>
    <% if method == 'initialize' %><% has_constructor = true %>
    VALUE binding_<%= classname %>_<%= method %>(VALUE self<%= desc['binding_params'] %>)
    {
      Ruby::Constant    os("ObjectSpace");
      std::stringstream stream;
      RarityClass*      instance = new <%= classname %>(<%= desc['params_apply'] %>);
      Ruby::Object      proc;

      stream << (long)instance;
      std::cout << "Test1" << std::endl;
      proc = Ruby::Evaluate("Proc.new do (<%= classname %>.finalize " + stream.str() + ") end");
      std::cout << "Test2" << std::endl;
      os.Apply("define_finalizer", 2, instance, &proc);
      std::cout << "Test3" << std::endl;
      return (*instance);
    }<% else %>
    VALUE binding_<%= classname %>_<%= method %>(VALUE self<%= desc['binding_params'] %>)
    {
      long                  _ptr  = get_instance_pointer(self);
      <%= classname %>*     _this = reinterpret_cast<<%= classname %>*>(_ptr);
      <% if desc['return'] == 'nil' || desc['return'] == 'void' %>
        _this-><%= method %>(<%= desc['params_apply'] %>);
        return (Qnil);
      <% else %>
        <%= desc['return'] %> ret   = _this-><%= method %>(<%= desc['params_apply'] %>);
        return (<%= CppHelpers._return 'ret', desc['return'] %>);
      <% end %>
    }
    <% end %>
  <% end unless struct['methods'].nil? %>

  VALUE binding_<%= classname %>__initialize(VALUE) { return (Qnil); }

void Initialize_<%= classname %>()
{
  VALUE inherits = <% unless struct['inherits'].nil? %>
    get_class_by_name("<%= struct['inherits'] %>");
  <% else %>
    rb_cObject;
  <% end %>
  VALUE klass = rb_define_class("<%= classname %>", inherits);

  rb_define_method(klass, "_initialize", reinterpret_cast<RubyMethod>(binding_<%= classname %>__initialize), 0);
  rb_define_module_function(klass, "finalize", reinterpret_cast<RubyMethod>(RarityClassFinalize), 1);
  <% struct['methods'].each do |method, desc| %>
  rb_define_method(klass, "<%= method.underscore %>", reinterpret_cast<RubyMethod>(binding_<%= classname %>_<%= method %>), <%= if desc['params'].nil? then 0 else desc['params'].count end %>);
  <% end unless struct['methods'].nil? %>
}
<% end %>

void RarityInitialize(void)
{
  ruby_init();
  <% classes.each do |classname, struct| %>
  Initialize_<%= classname %>();<% end %>
}

void RarityFinalize(void)
{
  ruby_finalize();
}

/*
 * Rarity Ruby Starts here
 */
#include <cstdarg>

Ruby::Object Ruby::Object::Apply(Ruby::Symbol method, unsigned int argc, ...)
{
  this->method = method;
  this->argc   = argc;
  argv         = 0;
  values       = new VALUE[argc];
  result       = Qnil;

  if (argc > 0)
  {
    va_list ar;

    argv = ALLOCA_N(IRarityClass*, argc);
    va_start(ar, argc);
    for (unsigned int i = 0 ; i < argc ; ++i)
    {
      argv[i]     = va_arg(ar, IRarityClass*);
      if (argv[i] != 0)
        values[i] = argv[i]->GetRubyInstance();
      else
        values[i] = Qnil;
    }
    va_end(ar);
  }
  {
    int state = 0;

    result = rb_protect(WrappedApply, reinterpret_cast<VALUE>(this), &state);
    if (state)
      throw new Ruby::Exception();
  }
  delete[] values;
  return (result);
}

VALUE Ruby::Object::WrappedApply(VALUE self)
{
  Ruby::Object& _this = *(reinterpret_cast<Ruby::Object*>(self));

  return (rb_funcall2(_this.instance, _this.method, _this.argc, _this.values));
}

/*
 * Rarity global functions
 */
namespace Ruby
{
  void PushIncludePath(const std::string& path)
  {
    ruby_incpush(path.c_str());
  }

  VALUE RescuableRequire(VALUE str_ptr)
  {
    return (rb_require(reinterpret_cast<const char*>(str_ptr)));
  }

  bool Require(const std::string& path)
  {
    int state;

    rb_protect(RescuableRequire, reinterpret_cast<VALUE>(path.c_str()), &state);
    if (state)
      return (false);
    return (true);
  }

  VALUE Evaluate(const std::string& code)
  {
    return (rb_eval_string(code.c_str()));
  }

  const char* Inspect(VALUE value)
  {
    VALUE ret = rb_funcall2(value, rb_intern("inspect"), 0, 0);

    return (RSTRING_PTR(ret));
  }
}

