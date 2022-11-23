#ifndef  RARITY_EXCEPTION_HPP
# define RARITY_EXCEPTION_HPP

# include "object.hpp"
# include <exception>
# include <ostream>
# include <vector>

namespace Ruby
{
  struct Exception : public std::exception, public Object
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

  private:
    void initialize();
    std::string message;
    std::vector<std::string> trace;
  };
}

std::ostream& operator<<(std::ostream& stream, const Ruby::Exception&);

#endif
