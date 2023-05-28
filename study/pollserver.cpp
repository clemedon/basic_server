#include <cerrno>    // errno
#include <cstring>   // strerror
#include <iostream>  // cerr, cout
#include <vector>    // vector

#include <sstream>  // stringstream

#include <arpa/inet.h>  // inet_ntop, inet_ntoa
#include <poll.h>       // pollfd, poll
#include <stdlib.h>     // exit
#include <unistd.h>     // close

// recv, send, sockaddr, accept, addrinfo, getaddrinfo,
// socket, setsockopt, bind, freeaddrinfo, listen
#include <netdb.h>
/* #include <sys/socket.h> */
/* #include <sys/types.h> */

// exit() -> exceptions

#define PORT "4242"  // Port we're listening on

// gai_strerror -> custom gai_strerror_lookup

#include <iostream>
#include <string>

/**
 * @brief       Forbidden gai_strerror implementation
 */

std::string gaiStrerror( int errorCode ) {
  switch( errorCode ) {
    case 0:
      return "Success";
    case EAI_AGAIN:
      return "Temporary failure in name resolution";
    case EAI_BADFLAGS:
      return "Invalid value for ai_flags";
    case EAI_FAIL:
      return "Non-recoverable failure in name resolution";
    case EAI_FAMILY:
      return "ai_family not supported";
    case EAI_MEMORY:
      return "Out of memory";
    case EAI_NONAME:
      return "Name or service not known";
    case EAI_SERVICE:
      return "Invalid value for service";
    case EAI_SOCKTYPE:
      return "ai_socktype not supported";
    case EAI_SYSTEM:
      return strerror( errno );
    default:
      return "Unknown error";
  }
}

/**
 * @brief       Forbidden inet_ntop implementation
 */

std::string ntop( const struct sockaddr_storage& socket ) {
  const struct sockaddr_in*  sockaddr;
  const uint8_t*             addr;
  const struct sockaddr_in6* sockaddr6;
  const uint8_t*             addr6;
  std::stringstream          ss;

  if( socket.ss_family == AF_INET ) {
    sockaddr = reinterpret_cast<const struct sockaddr_in*>( &socket );
    addr = reinterpret_cast<const uint8_t*>( &sockaddr->sin_addr.s_addr );
    ss << static_cast<int>( addr[0] ) << ".";
    ss << static_cast<int>( addr[1] ) << ".";
    ss << static_cast<int>( addr[2] ) << ".";
    ss << static_cast<int>( addr[3] ) << ".";
    return ss.str();
  } else if( socket.ss_family == AF_INET6 ) {
    sockaddr6 = reinterpret_cast<const struct sockaddr_in6*>( &socket );
    addr6 = sockaddr6->sin6_addr.s6_addr;
    for( int i = 0; i < 16; ++i ) {
      if( i > 0 && i % 2 == 0 ) {
        ss << "::";
      }
      ss << static_cast<int>( addr6[i] );
    }
    return ss.str();
  } else {
    return std::string( "UNKN_ADDR" );
  }
}

class Server {
 public:
  void run();

 private:
  void delConnection( int i );    // poll
  void clientBroadcast( int i );  // recv, send
  void addConnection();           // accept
  int getListenerSocket();  // hint, getaddri., socket, bind, listen, freeaddri.
  void setup();

  int                 _listenerFd;  // Listening socket descriptor
  std::vector<pollfd> _pfds;        // List of connected clients descriptor
};

/**
 * @brief       Delete a connection
 *
 * Copy one of the fd in place of the deleted one to prevent the re-indexing of
 * our list.
 */

void Server::delConnection( int i ) {
  close( _pfds[i].fd );
  _pfds[i] = _pfds.back();
  _pfds.pop_back();
}

void Server::clientBroadcast( int i ) {
  char buf[256];  // Buffer for client data
  int  nbytes = recv( _pfds[i].fd, buf, sizeof( buf ), 0 );
  int  senderFd = _pfds[i].fd;

  if( nbytes <= 0 ) {
    if( nbytes == 0 ) {
      std::cout << "server: socket " << senderFd << " hung up\n";
    } else {
      std::cerr << "recv: " << strerror( errno ) << "\n";
    }
    delConnection( i );
    return;
  }
  // We got some good data from a client
  for( size_t j = 0; j < _pfds.size(); ++j ) {
    // Send to everyone!
    int destFd = _pfds[j].fd;
    // Except the _listenerFd and ourselves
    if( destFd == _listenerFd || destFd == senderFd ) {
      continue;
    }
    if( send( destFd, buf, nbytes, 0 ) == -1 ) {
      // TODO also check that send ret value == nbytes
      std::cerr << "send: " << strerror( errno ) << "\n";
    }
  }
}

void Server::addConnection() {
  int                     newfd;       // Newly accept()ed socket descriptor
  pollfd                  pfd;         // New pollfd for new connection
  struct sockaddr_storage remoteAddr;  // Client address
  socklen_t               remoteAddrLen;

  remoteAddrLen = sizeof( remoteAddr );
  newfd = accept( _listenerFd,
                  reinterpret_cast<struct sockaddr*>( &remoteAddr ),
                  &remoteAddrLen );
  if( newfd == -1 ) {
    std::cerr << "accept: " << strerror( errno ) << "\n";
    return;
  }
  // IPv6 TEST
  /* sockaddr->sin6_family = AF_INET6; */
  /* sockaddr->sin6_addr.s6_addr[15] = 1; */
  /* std::string ipAddress = ntop( socket ); */
  /* std::cout << "IPv6 Address: " << ipAddress << std::endl; */

  pfd.fd = newfd;
  pfd.events = POLLIN;  // Check ready-to-read
  pfd.revents = 0;      // prevent conditional jump in run()
  _pfds.push_back( pfd );

  std::cout << "pollserver: new connection from ";
  std::cout << ntop( remoteAddr );
  std::cout << " on socket " << newfd << "\n";
  return;
}

int Server::getListenerSocket() {
  int              lsock;    // Listening socket descriptor
  int              yes = 1;  // For setsockopt() SO_REUSEADDR, below
  int              status;
  struct addrinfo  hints;
  struct addrinfo* ai;
  struct addrinfo* ptr;

  // Get us a socket and bind it
  std::memset( &hints, 0, sizeof( hints ) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo( NULL, PORT, &hints, &ai );
  if( status != 0 ) {
    std::cerr << "selectserver: " << gaiStrerror( status ) << "\n";
    exit( 1 );
  }
  for( ptr = ai; ptr != NULL; ptr = ptr->ai_next ) {
    lsock = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol );
    if( lsock < 0 ) {
      continue;
    }
    // Lose the pesky "address already in use" error message
    status = setsockopt( lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );
    if( status != 0 ) {
      std::cerr << "setsockopt: " << strerror( errno ) << "\n";
      exit( 1 );
    }
    if( bind( lsock, ptr->ai_addr, ptr->ai_addrlen ) < 0 ) {
      close( lsock );
      continue;
    }
    break;
  }
  // If we got here, it means we didn't get bound
  if( ptr == NULL ) {
    return -1;
  }
  freeaddrinfo( ai );  // All done with this
  // Listen
  if( listen( lsock, 10 ) == -1 ) {
    return -1;
  }
  return lsock;
}

void Server::setup() {
  // Get a listening socket
  _listenerFd = getListenerSocket();
  if( _listenerFd == -1 ) {
    std::cerr << "error getting listening socket\n";
    exit( 1 );
  }

  pollfd pfd;
  pfd.fd = _listenerFd;
  pfd.events = POLLIN;  // Report read to read on incoming connection

  _pfds.push_back( pfd );
  // connection
}

void Server::run() {
  int poll_count;

  setup();
  // Main loop
  while( 1 ) {
    poll_count = poll( _pfds.data(), _pfds.size(), -1 );
    if( poll_count == -1 ) {
      std::cerr << "poll: " << strerror( errno ) << "\n";
      return;
    }
    // Run through the existing connections looking for data to read
    for( size_t i = 0; i < _pfds.size(); ++i ) {
      if( _pfds[i].revents & POLLIN ) {  // We got one!!
        if( _pfds[i].fd == _listenerFd ) {
          // If _listenerFd (us) is ready to read, handle new connection
          addConnection();
        } else {
          // If not the _listenerFd, we're just a regular client
          clientBroadcast( i );
        }
      }
    }
  }
}

int main( void ) {
  Server server;
  server.run();
  return 0;
}
