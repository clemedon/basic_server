/* study/socket */
/* Created: 230531 19:17:04 by clem9nt@imac */
/* Updated: 230531 19:17:04 by clem9nt@imac */
/* Maintainer: Clément Vidon */

#include <fcntl.h>  // cerr, cout
#include <netdb.h>  // recv, send, sockaddr, accept, addrinfo, getaddrinfo, socket, setsockopt, bind, freeaddrinfo, listen
#include <poll.h>   // pollfd, poll
#include <unistd.h>  // close
#include <iostream>  // cerr, cout
#include <vector>    // cerr, cout

// LISTENER SOCKET
// TODO useless since listener go to poll, poll can manage all socket

class Pollfd {
 public:
  explicit Pollfd( int fd, short events ) {
    _pollfd.fd = fd;
    _pollfd.events = events;
    _pollfd.revents = 0;
  }

  ~Pollfd() {
    std::cout << "Destruction of " << _pollfd.fd << std::endl;
    closePollfd();
    return;
  }

  int const&    getFd( void ) const { return _pollfd.fd; }
  pollfd const& getPollfd( void ) const { return _pollfd; }

  void closePollfd( void ) {
    std::cout << "Listener: Closed socket " << _pollfd.fd << std::endl;
    if( _pollfd.fd != -1 ) {
      close( _pollfd.fd );
      _pollfd.fd = -1;
    }
  }

 private:
  struct pollfd _pollfd;
};

int main( void ) {
  std::cout << std::endl;

  std::vector<pollfd> _pfds;  // Pollable file descriptors

  Pollfd l( open( __FILE__, O_RDONLY ), POLLIN );
  Pollfd c( open( __FILE__, O_RDONLY ), POLLIN );

  _pfds.push_back( l.getPollfd() );
  _pfds.push_back( c.getPollfd() );

  std::cout << "Listener socket: " << l.getFd() << std::endl;
  std::cout << "Client socket: " << c.getFd() << std::endl;

  int result = poll( reinterpret_cast<pollfd*>( _pfds.data() ), _pfds.size(),
                     -1 );
  std::cout << result << std::endl;
  return result;
}

// TODO does pfds really need RAII per socket ? cause isnt alreadi RAII to be
// managed by the Server class, i mean pfds is an array and when program stop
// it is cleared by Server.
