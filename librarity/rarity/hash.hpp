#ifndef  RARITY_HASH_HPP
# define RARITY_HASH_HPP

# include "object.hpp"
# include "array.hpp"
# include "cpp2ruby.hpp"
# include <map>

namespace Ruby
{
  class Hash : public Object
  {
  public:
    struct iterator
    {
      iterator(VALUE hash, long position = 0) : hash(hash), position(position)
      {
      }

      Ruby::Object key() const;
      Ruby::Object value() const;
      Ruby::Object operator*() { return value(); }
      Ruby::Object operator->() { return value(); }
      iterator& operator++() { position++; return *this; }
      iterator& operator++(int i) { position += i; return *this; }
      bool operator==(const iterator& other) const { return position == other.position || (key().is_nil() && other.key().is_nil()); }
      bool operator!=(const iterator& other) const { return !operator==(other); }
    private:
      VALUE hash;
      long position;
    };

    Hash(void) : Object(rb_hash_new()) {}
    Hash(VALUE copy) : Object(copy) {}
    Hash(const Hash& copy) : Object(rb_hash_dup(copy.instance)) {}

    iterator begin() const { return iterator(instance); }
    iterator end() const { return iterator(instance, -1); }

    Object operator[](VALUE key) const { return rb_hash_aref(instance, key); }
    Object operator[](Object key) const { return operator[](key.ruby_instance()); }

    template<typename PARAM>
    Object operator[](PARAM& param) const
    {
      return operator[](to_ruby_type<PARAM>(param)->ruby_instance());
    }

    template<typename VALUE, typename KEY>
    VALUE at(KEY& param) const
    {
      Object value = operator[](param);
      return to_cpp_type<VALUE>(value);
    }

    template<typename KEY, typename VALUE>
    void emplace(KEY cpp_key, VALUE cpp_value)
    {
      auto key   = to_ruby_type<KEY>(cpp_key);
      auto value = to_ruby_type<VALUE>(cpp_value);

      rb_hash_aset(instance, key->ruby_instance(), value->ruby_instance());
    }

    void clear() { rb_hash_clear(instance); }

    std::size_t size() const { return rb_hash_size_num(instance); }

    bool contains(VALUE key) const { return rb_hash_aref(instance, key) != Qnil; }

    template<typename KEY>
    bool contains(KEY key) const
    {
      return contains(to_ruby_type<KEY>(key)->ruby_instance());
    }

    void remove(VALUE key) { rb_hash_delete(instance, key); }

    template<typename KEY>
    void remove(KEY cpp_key)
    {
      return remove(to_ruby_type<KEY>(cpp_key)->ruby_instance());
    }

    Array keys() { return Array(apply("keys").ruby_instance()); }

    template<typename KEY>
    std::vector<KEY> keys()
    {
      return keys().as_vector<KEY>();
    }

    template<typename KEY, typename VALUE>
    std::map<KEY, VALUE> to_map() const
    {
      std::map<KEY, VALUE> result;

      for (auto it = begin() ; it != end() ; ++it)
      {
        result.emplace(
          as_cpp_type<KEY>(it.key()),
          as_cpp_type<VALUE>(it.value())
        );
      }
      return result;
    }

    template<typename KEY, typename VALUE>
    operator std::map<KEY, VALUE>() const
    {
      return to_map<KEY, VALUE>();
    }
  };
}

#endif
