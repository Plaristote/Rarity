#include "hash.hpp"

using namespace Ruby;

Ruby::Object Hash::iterator::key() const
{
  if (position >= 0)
  {
    Array array = Hash(hash).keys();

    if (array.size() > position)
      return array[position];
  }
  return Qnil;
}

Ruby::Object Hash::iterator::value() const
{
  Object key_value = key();

  if (!key_value.is_nil())
    return Hash(hash)[key_value];
  return Qnil;
}
