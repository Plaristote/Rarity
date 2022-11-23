#include "exception.hpp"
#include "array.hpp"

using namespace Ruby;
using namespace std;

Exception::Exception(void) : Object(rb_gv_get("$!"))
{
  initialize();
}

Exception::Exception(VALUE value) : Object(value)
{
  initialize();
}

bool Exception::has_exception()
{
  return rb_gv_get("$!") != Qnil;
}

void Exception::initialize()
{
  VALUE inspect = apply("inspect");

  message = RSTRING_PTR(inspect);
  trace = Ruby::Array(apply("backtrace").ruby_instance()).as_vector<string>();
}

ostream& operator<<(ostream& stream, const Exception& exception)
{
  stream << exception.what() << endl;
  for (const auto& part : exception.backtrace())
    stream << "  " << part << endl;
  return stream;
}
