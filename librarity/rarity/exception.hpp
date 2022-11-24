#ifndef  RARITY_EXCEPTION_HPP
# define RARITY_EXCEPTION_HPP

# include <ruby.h>
# include <exception>
# include <ostream>
# include <vector>

namespace Ruby
{
  struct Exception : public std::exception
  {
    Exception(void);
    Exception(VALUE);

    virtual ~Exception(void) throw()
    {
    }

    static bool has_exception();

    const char* what(void) const throw()
    {
      return message.c_str();
    }

    const std::vector<std::string>& backtrace(void) const throw()
    {
      return trace;
    }

    operator VALUE() const { return instance; }

  private:
    void initialize();
    std::string message;
    std::vector<std::string> trace;
    VALUE instance;
  };
}

std::ostream& operator<<(std::ostream& stream, const Ruby::Exception&);

#endif
