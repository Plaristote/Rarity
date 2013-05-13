#include "scriptengine.hpp"
#include "myclass.hpp"

using namespace std;

int main(void)
{
  AngelScript::Engine::Initialize();

  try
  {
    asIScriptEngine*  engine  = AngelScript::Engine::Get();
    asIScriptContext* context = engine->CreateContext();
    asIScriptModule*  module  = AngelScript::ModuleManager::Require("test-module", "scripts/test.as");

    {
      MyClass my_myclass("Coucou tu veux voir ma bite ?");
      asIScriptFunction* main = module->GetFunctionByDecl("int main(MyClass@)");
      int                retval;

      cout << "MyClass name: " << my_myclass.GetName() << endl;
      AngelScript::Call(context, main, "O", &my_myclass);
      retval = AngelScript::GetReturn<int>(context);
      cout << "MyClass name: " << my_myclass.GetName() << endl;
      cout << "Returned value: " << retval << endl;
    }

    AngelScript::ModuleManager::Release(module);
  }
  catch (const std::exception* e)
  {
    std::cerr << "Catched exception: " << e->what() << std::endl;
  }
  AngelScript::Engine::Finalize();
  return (0);
}
