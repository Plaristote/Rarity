#include "exception.hpp"
#include "array.hpp"
#include "object.hpp"

using namespace Ruby;
using namespace std;

Exception::Exception(void) : instance(rb_gv_get("$!"))
{
  initialize();
}

Exception::Exception(VALUE value) : instance(value)
{
  initialize();
}

bool Exception::has_exception()
{
  return rb_gv_get("$!") != Qnil;
}

void Exception::initialize()
{
  VALUE inspect = Object(instance).apply("inspect");

  message = RSTRING_PTR(inspect);
  trace = Ruby::Array(Object(instance).apply("backtrace").ruby_instance()).as_vector<string>();
}

ostream& operator<<(ostream& stream, const Exception& exception)
{
  stream << exception.what() << endl;
  for (const auto& part : exception.backtrace())
    stream << "  " << part << endl;
  return stream;
}
