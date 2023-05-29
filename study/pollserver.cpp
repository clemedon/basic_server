/* study/pollserver */
/* Created: 230529 16:50:17 by clem9nt@imac */
/* Updated: 230529 16:50:19 by clem9nt@imac */
/* Maintainer: Clément Vidon */

#include <cerrno>    // errno
#include <cstring>   // strerror
#include <iostream>  // cerr, cout
#include <sstream>   // stringstream
#include <string>    // string
#include <vector>    // vector

#include <arpa/inet.h>  // inet_ntoa
#include <netdb.h>  // recv, send, sockaddr, accept, addrinfo, getaddrinfo, socket, setsockopt, bind, freeaddrinfo, listen
#include <poll.h>   // pollfd, poll
#include <stdlib.h>  // exit
#include <unistd.h>  // close

/**
 * TODO
 * - [ ] const correctness
 * - [ ] design pattern suggestion: RAII TODO Socket class !!
 * - [ ] add a way to quit the server
 * - [ ] add a way to quit the client
 * - [ ] turn exit() into exceptions
 */

#define PORT "4242"  // Port we're listening on

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
  std::stringstream ss;

  if( socket.ss_family == AF_INET ) {
    const struct sockaddr_in* sockaddr;
    const uint8_t*            addr;
    sockaddr = reinterpret_cast<const struct sockaddr_in*>( &socket );
    addr = reinterpret_cast<const uint8_t*>( &sockaddr->sin_addr.s_addr );
    ss << static_cast<int>( addr[0] ) << ".";
    ss << static_cast<int>( addr[1] ) << ".";
    ss << static_cast<int>( addr[2] ) << ".";
    ss << static_cast<int>( addr[3] ) << ".";
    return ss.str();
  } else if( socket.ss_family == AF_INET6 ) {
    const struct sockaddr_in6* sockaddr;
    const uint8_t*             addr;
    sockaddr = reinterpret_cast<const struct sockaddr_in6*>( &socket );
    addr = sockaddr->sin6_addr.s6_addr;
    for( int i = 0; i < 16; ++i ) {
      if( i > 0 && i % 2 == 0 ) {
        ss << "::";
      }
      ss << static_cast<int>( addr[i] );
    }
    return ss.str();
  } else {
    return std::string( "UNKN_ADDR" );
  }
}

struct Client {
 public:
  Client( std::string name ) : _name( name ) {}

  void         setName( std::string const& name );
  std::string& getName( void );

 private:
  std::string _name;
};

void Client::setName( std::string const& name ) {
  _name = name;
}

std::string& Client::getName( void ) {
  return _name;
}

class Server {
 public:
  void run();

 private:
  void shutdown( void );                              // close
  void delConnection( size_t cid );                   // close
  void broadcastMsg( std::string& msg, size_t cid );  // send
  void parseData( const char* data, size_t cid );
  void receiveData( size_t cid );  // recv, senb
  void addConnection();            // accept
  int getListenerSocket();  // hint, getaddri., socket, bind, listen, freeaddri.
  void setup();

  // add a map of clients that contains their name, a struct of their socket and
  // a struct of their status
  int                 _listener;  // Listening socket descriptor
  std::vector<pollfd> _pfds;      // Pollable file descriptors
  std::vector<Client> _clients;   // Pollable file descriptors
};

void Server::shutdown( void ) {
  std::cout << "Server shutting down...\n";
  for( size_t cid = 0; cid < _pfds.size(); ++cid ) {
    if( _pfds[cid].fd != _listener ) {
      close( _pfds[cid].fd );
    }
  }
  _pfds.clear();
  _clients.clear();
  close( _listener );
  exit( 0 );
}

/**
 * @brief       Delete a connection
 *
 * Copy one of the fd in place of the deleted one to prevent the re-indexing of
 * our list.
 */

void Server::delConnection( size_t cid ) {
  close( _pfds[cid].fd );

  _pfds[cid] = _pfds.back();
  _pfds.pop_back();

  _clients[cid] = _clients.back();
  _clients.pop_back();

  std::cout << "<" << _clients[cid].getName() << " left the channel>\n";
}

/**
 * @brief       Broadcast a message to all clients
 */

void Server::broadcastMsg( std::string& msg, size_t cid ) {
  int dest;

  msg = _clients[cid].getName() + ": " + msg + "\n";
  for( size_t i = 0; i < _pfds.size(); ++i ) {
    dest = _pfds[i].fd;
    if( dest != _listener && dest != _pfds[cid].fd ) {
      if( send( dest, msg.c_str(), msg.length(), 0 ) == -1 ) {
        std::cerr << "send: " << strerror( errno ) << "\n";
      }
    }
  }
  std::cout << msg;
}

void Server::parseData( const char* data, size_t cid ) {
  std::string msg( data );

  if( msg == "/shutdown" ) {
    shutdown();
  } else if( msg == "/quit" ) {
    delConnection( cid );
  } else if( msg.substr( 0, 6 ) == "/name " ) {
    std::cout << "<" << _clients[cid].getName();
    _clients[cid].setName( msg.substr( 6 ) );
    std::cout << " became " << _clients[cid].getName() << ">\n";
  } else {
    broadcastMsg( msg, cid );
  }
}

void Server::receiveData( size_t cid ) {
  char buf[256];  // Buffer for client data
  long nbytes = recv( _pfds[cid].fd, buf, sizeof( buf ), 0 );

  if( nbytes <= 0 ) {
    if( nbytes == 0 ) {
      std::cout << "server: socket " << _pfds[cid].fd << " hung up\n";
    } else {
      std::cerr << "recv: " << strerror( errno ) << "\n";
    }
    delConnection( cid );
    return;
  }
  // Turn "^M\n" into "\0" TODO OS compatibility
  buf[nbytes - 2] = '\0';
  parseData( buf, cid );
}

void Server::addConnection() {
  int                     newfd;       // Newly accept()ed socket descriptor
  pollfd                  pfd;         // New pollfd for new connection
  struct sockaddr_storage remoteAddr;  // Client address
  socklen_t               remoteAddrLen;

  remoteAddrLen = sizeof( remoteAddr );
  newfd = accept( _listener, reinterpret_cast<struct sockaddr*>( &remoteAddr ),
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

  if( _clients.size() == 0 ) {
    _clients.push_back( Client( "NONE" ) );
  }
  _clients.push_back( Client( "Anon_0" + std::to_string( newfd ) ) );

  std::cout << "<Anon_0" << newfd << " joined the channel>\n";
  /* std::cout << "pollserver: new connection from "; */
  /* std::cout << ntop( remoteAddr ); */
  /* std::cout << " on socket " << newfd << "\n"; */
  return;
}

/**
 * @brief       Create the listener socket
 */

int Server::getListenerSocket() {
  int              listener;  // Listening socket descriptor
  int              yes = 1;   // For setsockopt() SO_REUSEADDR, below
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
    listener = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol );
    if( listener < 0 ) {
      continue;
    }
    // Lose the pesky "address already in use" error message
    status = setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                         sizeof( int ) );
    if( status != 0 ) {
      std::cerr << "setsockopt: " << strerror( errno ) << "\n";
      exit( 1 );
    }
    if( bind( listener, ptr->ai_addr, ptr->ai_addrlen ) < 0 ) {
      close( listener );
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
  if( listen( listener, 10 ) == -1 ) {
    return -1;
  }
  return listener;
}

void Server::setup() {
  // Get a listening socket
  _listener = getListenerSocket();
  if( _listener == -1 ) {
    std::cerr << "error getting listening socket\n";
    exit( 1 );
  }

  pollfd pfd;
  pfd.fd = _listener;
  pfd.events = POLLIN;  // Report read to read on incoming connection
  _pfds.push_back( pfd );
  // connection
}

void Server::run() {
  int poll_count;

  setup();
  // Main loop
  while( 1 ) {
    poll_count = poll( _pfds.data(), static_cast<nfds_t>( _pfds.size() ), -1 );
    if( poll_count == -1 ) {
      std::cerr << "poll: " << strerror( errno ) << "\n";
      return;
    }
    // Run through the existing connections looking for data to read

    for( size_t cid = 0; cid < _pfds.size(); ++cid ) {
      if( _pfds[cid].revents & POLLIN ) {  // We got one!!
        if( _pfds[cid].fd == _listener ) {
          // If _listener (us) is ready to read, handle new connection
          addConnection();
        } else {
          // If not the _listener, we're just a regular client
          receiveData( cid );
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
