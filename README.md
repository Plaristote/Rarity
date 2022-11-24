# Rarity

Rarity is a tool designed to facilitate interactions between C++ and Ruby code. It consists of two elements:

- Rarity itself, which is libclang-based tool that parses your C++ code and generates Ruby bindings for it
- librarity, which wraps the ruby C library and allows you to interact with Ruby objects with a simple and efficient C++ API

Warning: this software hasn't been nearly tested enough to be declared stable. It is not recommended for production use.
You are very much encouraged to participate the testing effort by sending feedback and bug reports.

Build
===
Rarity uses the [build2](https://www.build2.org) build system and depends on `libclang` and `libruby`. Once you've
[installed build2](https://www.build2.org/install.xhtml) and the other dependencies, use the following commands to
build Rarity from source:

```sh
git clone https://github.com/Plaristote/Rarity.git

bpkg create -d "build-Rarity-gcc" cc config.cxx=g++

cd build-Rarity

bpkg add --type dir ../Rarity
bpkg fetch
bpkg build Rarity '?sys:libclang/*' '?sys:libruby/*'
```

Tweak the `config.cxx=g++` argument if you wish to use another compiler (ex: `config.cxx=clang++`).

If you're using Linux or FreeBSD, You can then install Rarity to your system using:

```sh
bpkg install Rarity \
  config.install.root=/usr \
  config.install.sudo=sudo
```

You should now have installed the `librarity` library, the `rarity` code generator, as well as the headers for `librarity`. Let's see how to put those to good use:

# Usage
## librarity
Let's first start by learning the basics of librarity by initializing a Ruby VM and using
it to evaluate some Ruby code:

```c++
#include <rarity.hpp>

int main(int, char**)
{
  Rarity rarity; // Must be instantiated once and only once

  Ruby::evaluate("puts 'Hello, world !'");
  return 0;
}
```

That was easy. How about loading a Ruby script ? Let's see how that goes:

```c++
Ruby::push_include_path(".");
Ruby::require("script.rb");
```

#### Interacting with Ruby objects from C++
Let's see how to call a method on a Ruby object:

```c++
Ruby::Object number1 = Ruby::evaluate("42");
Ruby::Object number2 = number.apply("+", 23);
```

The `apply` method calls a Ruby method which symbol is given as a first parameter (here we call the `+` method), and returns the result as a Ruby object. Parameters can be sent using any of the
supported C++ type, instances of `Ruby::Object`, or pointers to a class implementing the `IRarityClass` interface.

You may also use template parameters to have the `apply` method return a supported C++ type, such as:

```c++
int result = Ruby::evaluate("42").apply<int>("+", 23);
```

#### Ruby wrappers
librarity comes with a handful or wrappers designed to facilitate interaction with Ruby objects from C++. These types cover the 'Proc', `Array`, and `Hash`` Ruby classes.
Check out their usages in the examples for [Proc](https://github.com/Plaristote/Rarity/tree/master/examples/05-lambda), [Array](https://github.com/Plaristote/Rarity/tree/master/examples/06-array), and [Hash](https://github.com/Plaristote/Rarity/tree/master/examples/07-hash).

#### Converting from Ruby to C++ types
librarity comes with 3 templates functions to help you convert types smoothly from one context to another:

- `Ruby::to_cpp_type` takes a `Ruby::Object` as a parameter, and returns a C++ value as a result. You can use it like this:
```c++
std::string native_string = Ruby::to_cpp_type<std::string>(ruby_string)
```
- `Ruby::as_cpp_type` is similar to `to_cpp_type`, but it's safer to use: while `to_cpp_type` will crash when the requested C++ type doesn't match the Ruby value's class, `as_cpp_type` will first convert the Ruby value to an acceptable form. For instance:
```c++
std::string recipe_for_disaster = Ruby::to_cpp_type<std::string>(Ruby::evaluate("42")); // won't work
std::string recipe_for_success = Ruby::as_cpp_type<std::string>(Ruby::evaluate("42")); // will work
```
- `Ruby::to_ruby_type` takes a C++ value and returns `std::shared_ptr<IRarityClass>`. You can then convert that result to `Ruby::Object` using the `IRarityClass::ruby_instance` method, such as:
```c++
Ruby::Object object = Ruby::to_ruby_type<std::string>("Hello, world")->ruby_instance()
```

## C++ to Ruby bindings
As you can see, librarity allows us to easily interact with Ruby object from C++. But what if we want to expose some of our C++ program to Ruby ? That's where Rarity's magic truly starts.

```sh
rarity \
  -i header.hpp \
  -o rarity_bindings.cpp \
  --clang \
  -I/usr/lib64/llvm13/lib/clang/13.0.1/include
```

TODO

## Ruby extensions
TODO
At some point, Rarity will also work as a way to write Ruby extensions using C++.

# Notes on API generation
### Naming convention
In Ruby, method names are supposed to be written in snake case. Note that regardless of your naming convention in C++, the Ruby bindings will use snake case.

Similarly, constants are supposed to be written using upper camelcase, and Rarity will enforce that on your classes and namespaces when generating their bindings.

For instance, the following C++ code:
```
struct my_struct
{
  static bool Method() { return true; }
};
```
Will be bound in Ruby as: `MyStruct.method` instead of `my_struct.Method`.

### Natively supported types
The generated bindings will support the type you described (at least as pointers). Moreover, the native following
types can be converted from one side to the other (or both).

    <-  = Ruby to C++ conversion supported
    ->  = C++ to Ruby conversion supported
    <-> = Ruby to C++ and C++ to Ruby conversions supported
    
    std::string   <-> String
    unsigned int  <-> Fixnum
    int           <-> Fixnum
    float         <-> Float
    std::function <-  Proc
    std::vector   <-> Array
    Ruby::Object  <-> Any ruby object

### Initialize
It is possible not to expose any constructor to Ruby, in which case it will be impossible from Ruby scripts to
instantiate the class.

### Memory Management
Depending on the context in which objects are created, they might not answer to the same garbage collecting rules.

If you create an object from Ruby using the "new" method, your object will answer to Ruby's garbage collector. The linked C++
object will be destroyed when Ruby's garbage collector collects its Ruby counterpart. However, if your object is instantiated
from C++, you will have to delete it yourself.

Consequently, it is possible for a Ruby object to outlive its C++ counterpart. In that case, the Ruby object will not crash the
application; however, it will throw an exception.
