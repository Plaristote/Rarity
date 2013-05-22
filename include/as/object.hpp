#ifndef  AS_OBJECT_HPP
# define AS_OBJECT_HPP

# include "scriptengine.hpp"

namespace AngelScript
{
  template<typename TYPE> struct TypeFlags         { enum { value = 'O' }; };
  template<>              struct TypeFlags<int>    { enum { value = 'i' }; };
  template<>              struct TypeFlags<bool>   { enum { value = 'b' }; };
  template<>              struct TypeFlags<float>  { enum { value = 'f' }; };
  template<>              struct TypeFlags<long>   { enum { value = 'l' }; };
  template<>              struct TypeFlags<double> { enum { value = 'd' }; };

  struct IType
  {
    virtual ~IType(void) {}
    virtual unsigned char Flag(void) const { return ('0'); }
    virtual void*         Ptr(void) = 0;
  };

  struct AngelScriptClass : public IType
  {
    AngelScriptClass(const std::string&) {}
    virtual void* Ptr(void) { return (this); }
  };

  template<typename TPL_TYPE>
  class Type : public IType
  {
  public:
    Type(TPL_TYPE value) : value(value) {}

    operator TPL_TYPE() const
    {
      return (value);
    }

    unsigned char Flag(void) const
    {
      return (TypeFlags<TPL_TYPE>::value);
    }

    void* Ptr(void) { return (&value); }

  private:
    TPL_TYPE value;
  };

  template<typename TPL_TYPE>
  class Type<TPL_TYPE*> : public IType
  {
  public:
    Type(TPL_TYPE* instance) : instance(instance) {}
    
    operator void*() const
    {
      return (instance);
    }
    
    void* Ptr(void) { return (instance); }
    
  private:
    TPL_TYPE* instance;
  };

  class Object
  {
    struct Function
    {
      std::string        signature;
      asIScriptFunction* function;
    };

    typedef std::map<std::string, Function> Functions;
  public:
    Object(const std::string& filepath);
    Object(asIScriptContext* context, const std::string& filepath);
    ~Object();
  protected:
    void Initialize(void);    
  public:
    void asDefineMethod(const std::string& name, const std::string& declaration);

    struct ReturnType
    {
      ReturnType(asIScriptContext* context) : context(context) {}

      template<typename TPL_TYPE>
      operator TPL_TYPE() const
      {
        return (reinterpret_cast<TPL_TYPE>(context->GetReturnObject()));
      }

      operator bool()   const { return (context->GetReturnByte());  }
      operator int()    const { return (context->GetReturnWord());  }
      operator long()   const { return (context->GetReturnDWord()); }
      operator float()  const { return (context->GetReturnFloat()); }
      operator double() const { return (context->GetReturnFloat()); }

    protected:
      asIScriptContext* context;
    };

    ReturnType Apply(const std::string& name, unsigned int argc = 0, ...);

  private:
    const std::string filepath;
    asIScriptContext* context;
    asIScriptModule*  module;
    Functions         functions;
  };
}

#endif
