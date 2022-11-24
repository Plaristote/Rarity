#include <rarity.hpp>

int main(int, char**)
{
  Rarity rarity; // Must be instantiated once and only once

  Ruby::evaluate("puts 'Hello, world !'");
  return 0;
}
