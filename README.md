Rarity
======

Code generator for binding C++ APIs to Ruby with no sweat.
Rarity consists of a few header files you will need to include in your project and a script that will generate
your bindings by checking out a YAML file containing your bindings description.

Usage
===
By default, you need to go into your project's directory and execute the following (replacing $RARITY_PATH with the path to the rarity.rb script)

      ruby $RARITY_PATH/rarity.rb
      
This will create a rarity-bindings.cpp file in the current directory that you will need to link to your project.
You will also have to add the include directory $RARITY_PATH/include and link with a Ruby library.

You can also specify your own input directory and output file like this:

      ruby $RARITY_PATH/rarity.rb --input directory --output file.cpp

How to generate bindings for a class ?
===
The first thing to do is to add a component called 'RarityClass' to the class using inheritence. Any class that needs to be used from both
C++ and Ruby must include this component. After including the component, our code will look like this:
    
./my_class.hpp
    
     class MyClass : public RarityClass
     {
     public:
       MyClass(const std::string& name) : RarityClass("MyClass"), name(name)
       {}
     
       const std::string& GetName() const
       {
         return (name);
       }
       
      void SetName(const std::string& str)
      {
        name = str;
      }

     private:
       std::string name;
     };    
     
Notice that your constructor needs to call RarityClass's constructor, which takes as parameter the name of the Ruby class
that will be bound with this class.

Now to the actual binding part.
For Rarity to generate the bindings, you will need to create a binding YAML file that will look like this:

./bindings-myclass.yml

    MyClass:
      include: 'myclass.hpp' # The path to the file including your class
      methods:
        initialize: # The 'initialize' method will call the C++ constructor.
          params:
            - std::string
          return: void
        GetName:
          return: std::string
        SetName:
          params:
            - std::string
          return: void


The Rarity script will recursively look for YAML files whose names start with 'bindings-', so your YAML file must begin
with those characters (e.g.: bindings-myclass.yml).

How to use the bindings from a C++ application?
===
Rarity also comes with a set of tools for easily using the bindings. A few objects allow you to use Ruby objects from C++ and
handle exceptions properly.
Let's write a Ruby script and C++ main function using our previous bindings:

./scripts/test.rb

    class MyRubyClass
      def initialize
        puts "Initializing Ruby class"
        @my_class = MyClass.new "Name set from Ruby"
      end
      
      def run my_class = nil
        my_class ||= @my_class
        puts "[Ruby] #{my_class.get_name}"
      end
    end

./main.cpp

    #include "rarity.hpp"
    #include "myclass.hpp"
    
    int main(void)
    {
      RarityInitialize(); // Must be called before any construction of RarityClass instances
      try
      {
        MyClass my_class("C++-created MyClass");
      
        Ruby::PushIncludePath("./scripts");
        Ruby::Require("test.rb");
        
        Ruby::Constant my_ruby_class("MyRubyClass");
        Ruby::Object   my_ruby_instance = my_ruby_class.Apply("new");

        my_ruby_instance.Apply("run", 1, &my_class); // Method name, argument count, argument list of pointers to Rarity objects
        my_ruby_instance.Apply("run"); // Call the same script method using the default parameter value

      }
      catch (const std::exception* e) // Ruby exceptions are converted to std::exception-compatible objects
      {
        std::cerr << "Caught exception " << e->what() << std::endl;
      }
      Ruby::Constant("GC").Apply("start"); // Forces Ruby's garbage collector to start
      RarityFinalize();
      return (0);
    }
    
And that's it. We've seen pretty much everything Rarity offers.


Notes on API generation
======
Naming convention
============
In Ruby, method names are supposed to be written in snake case. Note that regardless of your naming convention in C++,
the Ruby bindings will use snake case (this means that in our previous example, the API generated for MyClass is actually:

    MyClass#initialize
    MyClass#get_name
    MyClass#set_name

Initialize
============
It is possible not to expose any constructor to Ruby, in which case it will be impossible from Ruby scripts to
instantiate the class.

Memory Management
============
Depending on the context in which objects are created, they might not answer to the same garbage collecting rules.

If you create an object from Ruby using the "new" method, your object will answer to Ruby's garbage collector. The linked C++
object will be destroyed when Ruby's garbage collector collects its Ruby counterpart. However, if your object is instantiated
from C++, you will have to delete it yourself.

Consequently, it is possible for a Ruby object to outlive its C++ counterpart. In that case, the Ruby object will not crash the
application; however, it will throw an exception.
