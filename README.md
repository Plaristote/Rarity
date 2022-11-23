Rarity
======

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

```
bpkg install Rarity \
  config.install.root=/usr \
  config.install.sudo=sudo
```

You should now have installed the `librarity` library, the `rarity` code generator, as well as the headers for `librarity`. Let's see how to put those to good use:

Usage
===
librarity
====
Let's first start by learning the basics of librarity by initializing a Ruby VM from a C++ program:

```
#include <rarity.hpp>
#include <iostream>

int main(int, char**)
{
  Rarity rarity; // Must be instantiated once and only once

  try
  {
    Ruby::Constant string_class("String");
    Ruby::Object string_instance = string_class.apply("new");
    std::string part1("Hello"), part2(", world!");

    string_instance = string_instance.apply("+", 1, Ruby::to_ruby_type(part1).get());
    string_instance = string_instance.apply("+", 1, Ruby::to_ruby_type(part2).get());
    std::cout << "String: " << Ruby::inspect(string_instance) << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Catched exception: " << e.what() << std::endl;
  }
  return 0;
}
```

This example should display `String: "Hello, world!"` when executed. This involved the use of two types:

Ruby::Object
========
`Ruby::Object` is used to interact with any Ruby object, allowing implicit conversion to any supported C++ type, and providing the `apply` method, which works similarly to the `send` method in Ruby.

When sending parameters to a Ruby method via `apply`, we must first specify how many parameters will be sent, then we send pointers to Ruby objects. In this example, we also created Ruby objects from C++ objects on the fly using `Ruby::to_ruby_type`, which returns an `std::shared_ptr`. `apply` doesn't support complex types (limitations of `va_args`), so you must also make sure to send raw pointers instead of smart pointers (hence the use of `.get()` after calls to `Ruby::to_ruby_type`).

On the other hand, `Ruby::Object` can be implicitely casted to any supported C++ type (see [Natively supporteed type](#Natively_supported_types)), such as:

```
Ruby::Object result = Ruby::Constant("String").apply("new");
std::string str = result; // implicit cast to std::string
```

Ruby::Constant
========
`Ruby::Constant` implements `Ruby::Object`, and allows you to quickly get a hold of a constant, such as a class object, a module. By default, constants are looked up in Ruby's global object, but you can also specify in which module you wish to look for a constant:

```
Ruby::require("net/http");
Ruby::Object net = Ruby::Constant("Net");
Ruby::Object http = Ruby::Constant("HTTP", net);
std::string domain("example.com"), path("/index.html");
Ruby::Object response = http.apply("get",
  Ruby::to_ruby_type(domain).get(),
  Ruby::to_ruby_type(path).get()
);

std::cout << (std::string)(response) << std::endl;
```

Ruby::Lambda
========
librarity also allows you to invoke Ruby's `Proc` objects from C++,
using the `Ruby::Lambda` object. Let's see how that works:

```
Ruby::Lambda lambda = Ruby::evaluate(
  "Proc.new { |param| puts \"Ruby lambda says: #{param.inspect}\" }"
);

lambda.call<void, std::string>("Hello, world !");
```

In this example, we used `call` to invoke the `Proc` object as if it were
a C++ function. We used template parameters to hint that the return type
would be `void`, and that we would send one paramter with the `std::string`
type.

We could've also used any other type supported by Rarity:

```
lambda.call<void, int>(42);
```

On top of that, if you do not wish to invoke the encapsulated code right
away, you may also get a hold of it as an instance of `std::function`:

```
std::function<void (std::string)> callback = lambda.as_function<void, std::string>();

callback("Hello, world !");
```

C++ to Ruby bindings
====
As you can see, librarity allows us to easily interact with Ruby object from C++. But what if we want to expose some of our C++ program to Ruby ? That's where Rarity's magic truly starts.

```
rarity \
  -i header.hpp \
  -o rarity_bindings.cpp \
  --clang \
  -I/usr/lib64/llvm13/lib/clang/13.0.1/include
```

TODO

Ruby extensions
=====
TODO
At some point, Rarity will also work as a way to write Ruby extensions using C++.

Notes on API generation
======
Naming convention
============
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

Natively supported types
============
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
