#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <iosfwd>
#include <string>

class Server {
 public:
  Server( void );
  Server( Server const& src );
  virtual ~Server( void );
  Server& operator=( Server const& rhs );
  virtual void print( std::ostream& o ) const;

 private:
};

std::ostream& operator<<( std::ostream& o, Server const& i );

#endif  // SERVER_HPP_
