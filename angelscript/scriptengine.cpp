#include "scriptengine.hpp"
#include <iostream>
#include <sstream>
<% includes.each do |header| %>#include "<%= header %>"
<% end %>
using namespace std;
using namespace AngelScript;

/*
 * Generated bindings
 */
<% classes.each do |classname, klass| %>
static void initialize_<%= klass['binding-symbol'] %>(asIScriptEngine* engine)
{
  const char* classname = "<%= klass['alias'] %>";
  <% klass['methods'].each do |methodname, method| %><% unless method['static'] == true %>
  engine->RegisterObjectMethod(classname, "<%= method['decl'] %>", asMETHODPR(<%= classname %>,<%= methodname %>, (<%= unless method['params'].nil? then method['params'].join ', ' else 'void' end %>) <%= 'const' if method['const'] == true %>, <%= method['return'] %>), asCALL_THISCALL);<% else %>
  engine->RegisterGlobalFunction("<%= method['decl'] %>", asFUNCTION(<%= classname %>::<%= methodname %>), asCALL_CDECL);<% end %><% end %>
}
<% end %>

static void AngelScriptInitialize(asIScriptEngine* engine)
{
<% classes.each do |name, klass| %>
  engine->RegisterObjectType("<%= klass['alias'] %>", 0, asOBJ_REF | asOBJ_NOCOUNT);<% end %>
<% classes.each do |name, klass| %>
  initialize_<%= klass['binding-symbol'] %>(engine);<% end %>
}

/*
 * AngelScript global code
 */
asIScriptEngine*                    Engine::_engine;
function<void (const std::string&)> Engine::ScriptError;
ModuleManager::Modules              ModuleManager::_modules;

AngelScript::Exception::Exception(asIScriptContext* context)
{
  const asIScriptFunction* function = context->GetExceptionFunction();
  std::stringstream        stream;

  stream << "[Rarity][AngelScript] " << context->GetExceptionString() << ": from function '" << function->GetDeclaration() << "' at line '" << context->GetExceptionLineNumber() << '\'';
  exc_string = stream.str();
}

void AngelScript::Call(asIScriptContext* context, asIScriptFunction* function, const std::string fmt, ...)
{
  int     ret;
  va_list ap;

  context->Prepare(function);
  va_start(ap, fmt);
  for (unsigned short i = 0 ; fmt[i] ; ++i)
  {
    if (fmt[i] == 'O')
      context->SetArgObject(i, va_arg(ap, void*));
    else if (fmt[i] == 'b')
      context->SetArgByte(i, va_arg(ap, int));
    else if (fmt[i] == 'i')
      context->SetArgDWord(i, va_arg(ap, int));
    else if (fmt[i] == 'l')
      context->SetArgDWord(i, va_arg(ap, long));
    else if (fmt[i] == 'f')
      context->SetArgFloat(i, va_arg(ap, double));
    else if (fmt[i] == 'd')
      context->SetArgDouble(i, va_arg(ap, double));
    else if (fmt[i] == 's')
      context->SetArgWord(i, va_arg(ap, int));
  }
  va_end(ap);
  ret = context->Execute();
  switch (ret)
  {
  case asCONTEXT_NOT_PREPARED:
  case asEXECUTION_ABORTED:
  case asEXECUTION_SUSPENDED:
  case asEXECUTION_FINISHED:
    break ;
  case asEXECUTION_EXCEPTION:
    throw new AngelScript::Exception(context);
    break ;
  }
}

namespace AngelScript
{
  template<> bool GetReturn<bool>(asIScriptContext* context)
  { return (context->GetReturnByte()); }
  template<> int GetReturn<int>(asIScriptContext* context)
  { return (context->GetReturnDWord()); }
  template<> long GetReturn<long>(asIScriptContext* context)
  { return (context->GetReturnDWord()); }
  template<> float GetReturn<float>(asIScriptContext* context)
  { return (context->GetReturnFloat()); }
  template<> double GetReturn<double>(asIScriptContext* context)
  { return (context->GetReturnDouble()); }
  template<> short GetReturn<short>(asIScriptContext* context)
  { return (context->GetReturnWord()); }
}

void             Engine::Initialize(void)
{
  _engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
  if (_engine)
  {
    _engine->SetMessageCallback(asFUNCTION(Engine::MessageCallback), 0, asCALL_CDECL);    
    RegisterStdString(_engine);
    //RegisterStdString_Generic(_engine);

    AngelScriptInitialize(_engine);
  }
}

void             Engine::Finalize(void)
{
  if (_engine)
    _engine->Release();
}

asIScriptModule* Engine::LoadModule(const std::string& name, const std::string& filepath)
{
  CScriptBuilder builder;

  if ((builder.StartNewModule(_engine, name.c_str())) >= 0)
  {
    if ((builder.AddSectionFromFile(filepath.c_str())) >= 0)
    {
      if ((builder.BuildModule()) >= 0)
        return (_engine->GetModule(name.c_str()));
      else
        cerr << "[ScriptEngine] Can't compile module '" << name << "'" << endl;
    }
    else
      cerr << "[ScriptEngine] Can't load file '" << filepath << "'" << endl;
  }
  else
    cerr << "[ScriptEngine] Can't start Module Loader" << endl;
  return (0);
}

void Engine::MessageCallback(const asSMessageInfo* msg, void* param)
{
  stringstream stream;
  string       errorType("Error");

  if (msg->type == asMSGTYPE_WARNING)
    errorType = "Warning";
  else if (msg->type == asMSGTYPE_INFORMATION)
    errorType = "Information";

  stream << "[AngelScript][" << errorType << "] " << msg->section << " (" << msg->row << ", " << msg->col << ')';
  stream << ' ' << msg->message;

  if (ScriptError)
    ScriptError(stream.str());
  else
    cerr << stream.str() << endl;
}

asIScriptModule* ModuleManager::Require(const std::string& name, const std::string& filepath)
{
  Modules::iterator existing = std::find(_modules.begin(), _modules.end(), filepath);
  
  if (existing == _modules.end())
  {
    LoadedModule module;
    
    module.users    = 1;
    module.filepath = filepath;
    module.ptr      = Engine::LoadModule(name, filepath);
    if (module.ptr == 0)
      return (0);
    _modules.push_back(module);
    return (module.ptr);
  }
  existing->users += 1;
  return (existing->ptr);
}

void             ModuleManager::Release(asIScriptModule* ptr)
{
  Modules::iterator existing = std::find(_modules.begin(), _modules.end(), ptr);
  
  if (existing != _modules.end())
  {
    existing->users -= 1;
    if (existing->users == 0)
    {
      Engine::Get()->DiscardModule(existing->ptr->GetName());
      _modules.erase(existing);
    }
  }
}

void ModuleManager::OutputFunctionList(asIScriptModule* ptr)
{
  unsigned int count = ptr->GetFunctionCount();

  cout << "Function for module " << ptr->GetName() << endl;
  for (unsigned int it = 0 ; it < count ; ++it)
  {
    const string declaration(ptr->GetFunctionByIndex(it)->GetDeclaration());

    cout << "  " << declaration << endl;
  }
  cout << endl;
}