#include "array.hpp"
#include <stdexcept>

using namespace Ruby;

bool Array::iterator::operator==(const iterator& other) const
{
  int positionA = rb_array_len(instance) <= position ? -1 : position;
  int positionB = rb_array_len(instance) <= other.position ? -1 : other.position;
  return positionA == positionB;
}

Object Array::operator[](int index)
{
  if (index >= size())
    throw std::out_of_range("Ruby::Array: index is out of range");
  return rb_ary_entry(instance, index);
}

Array::iterator Array::insert(iterator at, VALUE value)
{
  if (at.position == -1)
  {
    rb_ary_push(instance, value);
    return iterator(instance, size() - 1);
  }
  else
    rb_ary_store(instance, at.position, value);
  return at;
}

Array::iterator Array::erase(iterator at)
{
  if (at.position != -1 && at.position < size())
    rb_ary_delete_at(instance, at.position);
  return at;
}
