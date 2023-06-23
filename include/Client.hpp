#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <iosfwd>
#include <string>

/**
 * @brief       Client class representing a client connection.
 */

struct Client {
 public:
  explicit Client( std::string name );
  Client( Client const& src );
  virtual ~Client( void );
  Client&      operator=( Client const& rhs );
  virtual void print( std::ostream& o ) const;

  void         setName( std::string const& name );
  std::string& getName( void );

 private:
  std::string _name;
};

std::ostream& operator<<( std::ostream& o, Client const& i );

#endif  // CLIENT_HPP_
