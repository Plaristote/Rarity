#ifndef  SCRIPTENGINE_HPP
# define SCRIPTENGINE_HPP

# include <angelscript.h>
# include <scriptbuilder/scriptbuilder.h>
# include <scriptstdstring/scriptstdstring.h>
# include <stdarg.h>
# include <string>
# include <functional>
# include <list>

namespace AngelScript
{
  void Call(asIScriptContext*, asIScriptFunction*, const std::string fmt, ...);
  template<typename T> T GetReturn(asIScriptContext* context) { return (reinterpret_cast<T>(context->GetReturnObject())); }
  template<> bool   GetReturn<bool>(asIScriptContext* context);
  template<> int    GetReturn<int>(asIScriptContext* context);
  template<> long   GetReturn<long>(asIScriptContext* context);
  template<> float  GetReturn<float>(asIScriptContext* context);
  template<> double GetReturn<double>(asIScriptContext* context);
  template<> short  GetReturn<short>(asIScriptContext* context);
  
  class Engine
  {
  public:
    static asIScriptEngine* Get(void) { return (_engine);  }
    static void             Initialize(void);
    static void             Finalize(void);
    static asIScriptModule* LoadModule(const std::string& name, const std::string& filepath);

    static std::function<void (const std::string&)> ScriptError;

  private:
    static void             MessageCallback(const asSMessageInfo* msg, void* param);
    
    static asIScriptEngine* _engine;
  };
  
  class ModuleManager
  {
  public:
    static asIScriptModule* Require(const std::string& name, const std::string& filepath);
    static void             Release(asIScriptModule* module);
    static void             OutputFunctionList(asIScriptModule* module);
    
  private:
    struct LoadedModule
    {
      bool operator==(asIScriptModule* module) const { return (ptr == module);   }
      bool operator==(const std::string& str)  const { return (filepath == str); }
      
      std::string       filepath;
      asIScriptModule*  ptr;
      unsigned short    users;
    };
    typedef std::list<LoadedModule> Modules;
    
    static Modules _modules;
  };

  class Exception : public std::exception
  {
  public:
    Exception(asIScriptContext *ctx);

    const char* what(void) const throw() { return (exc_string.c_str()); }

  private:
    std::string exc_string;
  };
}

#endif
