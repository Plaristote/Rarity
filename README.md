Rarity
======

Code generator for binding C++ APIs to Ruby with no sweat.
Rarity consists in a few header files you will need to include to your project and a script that will generate
your bindings by checking out a YML file containing your bindings description.

How to generate bindings for a class ?
===
The Rarity script will recursively look for binding yml files describing C++ classes to bind with Ruby.
Let's say you have a C++ class to export, this one for instance:

     class MyClass
     {
     public:
       MyClass(const std::string& name) : name(name)
       {}
     
       const std::string& GetName() const
       {
         return (name);
       }
       
      void SetName(const std::string& str) const
      {
        name = str;
      }

     private:
       std::string name;
     };
     
The first thing to do is to add a component to the class using inheritence. Any class that need to be used from both
C++ and Ruby must include this component. After including the component, our code will look like this:
    
     class MyClass : public RarityClass
     {
     public:
       MyClass(const std::string& name) : RarityClass("MyClass"), name(name)
       {}
     
       const std::string& GetName() const
       {
         return (name);
       }
       
      void SetName(const std::string& str) const
      {
        name = str;
      }

     private:
       std::string name;
     };    
     
Notice that your constructor need to call RarityClass' constructor, which take as parameter the name of the Ruby class
that will be bound with this class.

Now to the actual binding part.
For Rarity to generate the bindings, you will need to create a binding YML file that will look like this:

    MyClass:
      include: 'myclass.hpp' # The path to the file including your class
      methods:
        initialize: # The 'initialize' method will call for the C++ constructor.
          params:
            - std::string
          return: void
        GetName:
          return: std::string
        SetName:
          params:
            std::string
          return: void


The Rarity script will recursively look for yml file which name starts with 'bindings-', so your YML file must begin
with those characters (ex: bindings-myclass.yml).

Notes on API generation
==
Naming convention
=
In Ruby, methods name are supposed to be written in snake case. Note that regardless of your naming convention in C++,
the Ruby bindings will use snake case (this means that in our previous example, the API generated for MyClass is actually:
    MyClass#initialize
    MyClass#get_name
    MyClass#set_name
Initialize
=
It is possible not to expose any constructor to Ruby, in which case it will be impossible from Ruby scripts to
instantiate the class.
Memory Management
=
Depending the context in which objects are created, they might not answer of the same garbage collecting rules.
If you create an object from Ruby using the "new" method, your object will answer to Ruby's garbage collector. The C++
object linked will be destroyed when Ruby garbages collect its Ruby counterpart. However, if your object is instantiated
from C++, you will have to delete it yourself.
It is, thus, possible that a Ruby object outlives its C++ counterpart. In which case the Ruby object will not crash the
application, however it will throw an exception.

How to use the bindings from a C++ application ?
===
Rarity also comes with a set of tools for easily using the bindings. A few objects allow to use Ruby objects from C++ and
handle exceptions propely.
Let's write a Ruby script c++ main using our previous bindings:

./scripts/test,rb
    class MyRubyClass
      def initialize
        puts "Initalizing ruby class"
        @my_class = MyClass.new "Name set from ruby"
      end
      
      def run my_class = nil
        my_class ||= @my_class
        puts "[Ruby] #{prefix} -> #{my_class.get_name}"
      end
    end

./main.cpp
    #include "rarity.hpp"
    #include "myclass.hpp"
    
    int main(void)
    {
      RarityInitialize(); // Must be called before any construction of RarityClass instances
      {
        MyClass my_class("C++ created MyClass");
      
        Ruby::PushIncludePath("./scripts");
        Ruby::Require("test.rb");
        
        Ruby::Constant my_ruby_class("MyRubyClass");
        Ruby::Object   my_ruby_instance = my_ruby_class.Apply("new");

        my_ruby_instance.Apply("run");
        my_ruby_instance.Apply("run", 1, &my_class); // Method name, argument count, argument list of pointers to Rarity objects

        Ruby::Constant("GC").Apply("start"); // Forces Ruby's garbage collector to start
      }
      RarityFinalize();
      return (0);
    }
    
And that's it. We've overseen pretty much everything Rarity offers.
