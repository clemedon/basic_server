/* study/socket */
/* Created: 230531 19:17:04 by clem9nt@imac */
/* Updated: 230531 19:17:04 by clem9nt@imac */
/* Maintainer: Clément Vidon */

#include <fcntl.h>  // cerr, cout
#include <netdb.h>  // recv, send, sockaddr, accept, addrinfo, getaddrinfo, socket, setsockopt, bind, freeaddrinfo, listen
#include <poll.h>   // pollfd, poll
#include <unistd.h>  // close
#include <iostream>  // cerr, cout

// LISTENER SOCKET

class ListenerSocket {
 public:
  explicit ListenerSocket( void ) : _fd( -1 ) {
    std::cout << "Listener: Construction with socket " << _fd << std::endl;
    _fd = open( __FILE__, O_RDONLY );
    std::cout << "Listener: Created socket " << _fd << std::endl;
  }

  ~ListenerSocket( void ) {
    closeSocket();
    std::cout << "Listener: Destruction with socket " << _fd << std::endl;
  }

  int& getSocket( void ) const {
    std::cout << "Listener: Get socket " << _fd << std::endl;
    return _fd;
  }

  void closeSocket( void ) {
    std::cout << "Listener: Closed socket " << _fd << std::endl;
    if( _fd != -1 ) {
      close( _fd );
      _fd = -1;
    }
  }

 private:
  int _fd;
};

// POLL SOCKET

/**
 * @brief       Manage pollfd's fd
 *
 * Create a new fd for struct pollfd, require a listener fd.
 */

class PollSocket : public ISocket {
 public:
  explicit PollSocket( int fd, short events, short revents )
    : _pfd.fd( fd ),
  _pfd.events( events ), _pfd.revents( revents ) {
    std::cout << "Poll: Construction with socket " << _pfd.fd << std::endl;
    /* struct sockaddr_storage remoteAddr; */
    /* socklen_t               remoteAddrLen; */
    pollfd pfd;  // New pollfd for new connection
    /* remoteAddrLen = sizeof( remoteAddr ); */
    /* _pfd.fd = accept( listener, */
    /*                   reinterpret_cast<struct sockaddr*>( &remoteAddr ), */
    /*                   &remoteAddrLen ); */
    /* if( _pfd.fd == -1 ) { */
    /*   std::cerr << "accept: " << strerror( errno ) << "\n"; */
    /*   exit( 1 ); */
    /* } */
    _pfd.fd = open( __FILE__, O_RDONLY );
    _pfd.events = POLLIN;
    _pfd.revents = 0;
    _pfds.push_back( pfd );
    std::cout << "Poll: Created socket " << _pfd.fd << std::endl;
  }

  ~PollSocket() {
    closeSocket();
    std::cout << "Poll: Destruction with socket " << _pfd.fd << std::endl;
  }

  int& getSocket( void ) const {
    std::cout << "Poll: Get socket " << _fd << std::endl;
    return _pfd.fd;
  }

  pollfd& getPfd( void ) const {
    std::cout << "Poll:" << std::endl;
    std::cout << "fd = " << _pfd.fd << std::endl;
    std::cout << "events = " << _events.fd << std::endl;
    std::cout << "revents = " << _revents.fd << std::endl;
    return _pfd;
  }

  void closeSocket( void ) {
    std::cout << "Poll: Closed socket " << _pfd.fd << std::endl;
    if( _pfd.fd != -1 ) {
      close( _pfd.fd );
      _pfd.fd = -1;
    }
  }

 private:
  pollfd _pfd;
};

int main( void ) {
  std::cout << std::endl;

  ListenerSocket s;
  s.createSocket();
  s.getSocket();
  s.getSocket();

  int newsocket = open( __FILE__, O_RDONLY );

  PollSocket s;
  s.createSocket();
  s.getSocket();
  s.getSocket();
}
