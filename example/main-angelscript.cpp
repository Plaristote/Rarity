#include "as/scriptengine.hpp"
#include "as/object.hpp"
#include "myclass.hpp"

using namespace std;

int main(void)
{
  AngelScript::Engine::Initialize();

  try
  {
    asIScriptEngine*    engine  = AngelScript::Engine::Get();
    AngelScript::Object object("scripts/test.as");

    object.asDefineMethod("main", "int main(MyClass@)");

    {
      MyClass my_myclass("Coucou tu veux voir ma bite ?");
      int                retval;

      cout << "MyClass name: " << my_myclass.GetName() << endl;
      retval = object.Apply("main", 1, &my_myclass);
      cout << "MyClass name: " << my_myclass.GetName() << endl;
      cout << "Returned value: " << retval << endl;
    }

    //AngelScript::ModuleManager::Release(module);
  }
  catch (const std::exception* e)
  {
    std::cerr << "Catched exception: " << e->what() << std::endl;
  }
  AngelScript::Engine::Finalize();
  return (0);
}
