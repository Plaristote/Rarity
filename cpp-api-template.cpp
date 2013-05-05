#include "rarity.hpp"
<% includes.each do |header| %>#include "<%= header %>"
<% end %>
#include <string>
#include <sstream>

using namespace std;

VALUE Rarity::wrapping_module;

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

static VALUE RarityClassFinalize(VALUE, VALUE param)
{
  long ptr = NUM2LONG(param);

  delete ((RarityClass*)ptr);
  return (Qnil);
}

namespace Ruby
{
  template<> Ruby::Object ToCppType<Ruby::Object>(VALUE value) { return (value); }
  template<> std::string  ToCppType<std::string>(VALUE value)  { return (std::string(RSTRING_PTR(value))); }
  template<> bool         ToCppType<bool>(VALUE value)         { return (value == Qtrue ? true : false); }
  template<> float        ToCppType<float>(VALUE value)        { return (NUM2DBL(value)); }
  template<> int          ToCppType<int>(VALUE value)          { return (NUM2INT(value));  }
  template<> unsigned int ToCppType<unsigned int>(VALUE value) { return (NUM2UINT(value)); }
  template<> long         ToCppType<long>(VALUE value)         { return (NUM2LONG(value)); }
  template<> double       ToCppType<double>(VALUE value)       { return (NUM2LONG(value)); }

  template<> IRarityClass* ToRubyType<const char*>(const char*& str)
  { return (new Ruby::Object(rb_str_new2(str))); }
  template<> IRarityClass* ToRubyType<std::string>(std::string& str)
  { return (new Ruby::Object(rb_str_new2(str.c_str()))); }
  template<> IRarityClass* ToRubyType<unsigned int>(unsigned int& nbr)
  { return (new Ruby::Object(UINT2NUM(nbr))); }
  template<> IRarityClass* ToRubyType<int>(int& nbr)
  { return (new Ruby::Object(INT2NUM(nbr))); }
  template<> IRarityClass* ToRubyType<double>(double& dbl)
  { return (new Ruby::Object(NUM2DBL(dbl))); }
  template<> IRarityClass* ToRubyType<float>(float& nbr)
  { return (new Ruby::Object(rb_float_new(nbr))); }
  template<> IRarityClass* ToRubyType<bool>(bool& boolean)
  { return (new Ruby::Object(boolean ? Qtrue : Qfalse)); }
}

template<typename T>
VALUE CppToRuby(T var)
{
  IRarityClass* rarity_class = Ruby::ToRubyType<T>(var);
  VALUE         value        = rarity_class->GetRubyInstance();

  delete rarity_class;
  return (value);
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

      instance->SetRubyInstance(self);
      stream << (long)instance;
      proc = Ruby::Evaluate("Proc.new do (<%= classname %>.finalize " + stream.str() + ") end");
      os.Apply("define_finalizer", 2, instance, &proc);

      return (Qnil);
    }<% else %>
    VALUE binding_<%= classname %>_<%= desc['name'] %>(VALUE self<%= desc['binding_params'] %>)
    {
      long                  _ptr  = get_instance_pointer(self);
      <%= classname %>*     _this = reinterpret_cast<<%= classname %>*>(_ptr);

      <%= desc['params_check'] %>
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
  <% if options[:mod].nil? %>
  VALUE klass = rb_define_class("<%= classname %>", inherits);
  <% else %>
  VALUE klass = rb_define_class_under(Rarity::wrapping_module, "<%= classname %>", inherits);
  <% end %>

  rb_define_method(klass, "_initialize", reinterpret_cast<RubyMethod>(binding_<%= classname %>__initialize), 0);
  rb_define_module_function(klass, "finalize", reinterpret_cast<RubyMethod>(RarityClassFinalize), 1);
  <% struct['methods'].each do |method, desc| %>
  rb_define_method(klass, "<%= desc['ruby_name'] %>", reinterpret_cast<RubyMethod>(binding_<%= classname %>_<%= desc['name'] %>), <%= if desc['params'].nil? then 0 else desc['params'].count end %>);
  <% end unless struct['methods'].nil? %>
}
<% end %>

void RarityInitialize(void)
{
  ruby_init();
  <% unless options[:mod].nil? %>
  Rarity::wrapping_module = rb_define_module("<%= options[:module] %>");
  <% else %>
  Rarity::wrapping_module = rb_cObject;
  <% end %>
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

  template<>
  IRarityClass* ToRubyType<IRarityClass*>(IRarityClass*& type)
  {
    return (new Object(type->GetRubyInstance()));
  }
}

