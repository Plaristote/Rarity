#ifndef  RARITY_TYPE_TRAITS_HPP
# define RARITY_TYPE_TRAITS_HPP

# include <type_traits>
# include <vector>
# include <list>
# include <set>
# include <functional>

namespace Ruby
{
  template<typename Test, template<typename...> class Ref>
  struct is_specialization : std::false_type {};

  template<template<typename...> class Ref, typename... Args>
  struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

  template<typename T>
  struct is_std_vector : is_specialization<T, std::vector> {};

  template<typename T>
  struct is_std_list : is_specialization<T, std::list> {};

  template<typename T>
  struct is_std_set : is_specialization<T, std::set> {};

  template<typename T>
  struct is_iterable
  {
    static constexpr bool value =
      is_std_vector<T>::value || is_std_list<T>::value || is_std_set<T>::value;
  };

  template<typename>
  struct is_std_function : public std::false_type {};

  template<typename A, typename... Args>
  struct is_std_function<std::function<A (Args...)> > : public std::true_type {};

  template<typename A, typename B>
  struct is_base_of : public std::is_base_of<A, B> {};

  template<typename A, typename B>
  struct is_base_of<A, B*> : public std::is_base_of<A, B> {};
}

#endif
