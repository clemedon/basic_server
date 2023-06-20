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

const int MAX_BUFFER_SIZE = 1024;
#define PORT "4242"  // Port we're listening on

#include <fcntl.h>

int fdIsValid( int fd ) {
  return fcntl( fd, F_GETFD ) != -1 || errno != EBADF;
}

/**
 * @brief       Int to String
 */

std::string intToString( int number ) {
  std::ostringstream oss;
  oss << number;
  return oss.str();
}

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

/**
 * @brief       ServerSocket
 */

class ServerSocket {
 private:
  int _fd;

 public:
  ServerSocket() : _fd( -1 ) {}
  ~ServerSocket() { close(); }

  void createListenerSocket( void ) {
    int             yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int             status;
    struct addrinfo hints, *res, *p;
    // Get us a socket and bind it
    std::memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    status = getaddrinfo( NULL, PORT, &hints, &res );
    if( status != 0 ) {
      std::cerr << "selectserver: " << gaiStrerror( status ) << "\n";
      exit( 1 );
    }
    for( p = res; p != NULL; p = p->ai_next ) {
      _fd = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
      if( _fd < 0 ) {
        continue;
      };
      if( setsockopt( _fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) )
          != 0 ) {
        std::cerr << "setsockopt: " << strerror( errno ) << "\n";
        exit( 1 );
      }
      if( bind( _fd, p->ai_addr, p->ai_addrlen ) != 0 ) {
        ::close( _fd );
        continue;
      }
      break;
    }
    freeaddrinfo( res );
    if( p == NULL ) {
      throw std::runtime_error( "Error creating socket" );
    }
    if( listen( _fd, 10 ) == -1 ) {
      throw std::runtime_error( "Error listening on socket" );
    }
  }

  int get( void ) { return _fd; }

 private:
  void close( void ) {
    if( _fd != -1 ) {
      ::close( _fd );
      _fd = -1;
    }
  }
};

class Server {
 private:
  ServerSocket        _listenerSocket;
  std::vector<int>    _clientSockets; // TODO NO RAII ? managed by server ?
  std::vector<int>    _disconnectedClients;
  std::vector<pollfd> _pollfds;

 public:
  Server( void ) {
    pollfd listenerPfd;

    _listenerSocket.createListenerSocket();

    listenerPfd.fd = _listenerSocket.get();
    listenerPfd.events = POLLIN;
    listenerPfd.revents = 0;
    _pollfds.push_back( listenerPfd );
  }

  void run() {
    while( true ) {
      if( poll( _pollfds.data(), _pollfds.size(), -1 ) < 0 ) {
        std::cerr << "Error in poll" << std::endl;
        exit( 1 );
      }
      if( _pollfds[0].revents & POLLIN ) {
        handleNewClient();
      }
      for( std::size_t i = 1; i < _pollfds.size(); ++i ) {
        if( _pollfds[i].revents & POLLIN ) {
          handleExistingClient( i );
        }
      }
      removeDisconnectedClients();
    }
  }

 private:
  void handleNewClient() {
    sockaddr_storage clientAddress;
    socklen_t        clientAddressLength;
    int              clientSocket;
    pollfd           clientPfd;

    clientAddressLength = sizeof( clientAddress );
    clientSocket = accept( _listenerSocket.get(),
                           reinterpret_cast<struct sockaddr*>( &clientAddress ),
                           &clientAddressLength );
    if( clientSocket < 0 ) {
      std::cerr << "accept: " << strerror( errno ) << "\n";
      return;
    }
    std::cout << "New connection from " << ntop( clientAddress );
    /* std::cout << ":" << ntohs( clientAddress.sin_port ) << std::endl; */
    std::cout << std::endl;

    _clientSockets.push_back( clientSocket );
    // Add the client socket to _pollfds

    clientPfd.fd = clientSocket;
    clientPfd.events = POLLIN;
    clientPfd.revents = 0;
    _pollfds.push_back( clientPfd );
    std::cout << "<Anon_" << clientSocket << " joined the channel>\n";
    return;
  }

  void broadcastMsg( std::string& msg, std::size_t index ) {
    int recipient;
    int clientSocket = _pollfds[index].fd;

    msg = intToString( clientSocket ) + ": " + msg + "\n";
    for( std::size_t i = 0; i < _pollfds.size(); ++i ) {
      recipient = _pollfds[i].fd;
      if( recipient != _listenerSocket.get() && recipient != clientSocket ) {
        if( send( recipient, msg.c_str(), msg.length(), 0 ) < 0 ) {
          std::cerr << "send: " << strerror( errno ) << "\n";
          exit( 1 );
        }
      }
    }
    std::cout << msg;
  }

  void parseData( const char* data, std::size_t index ) {
    std::string msg( data );

    /* if( msg == "/shutdown" ) { */
    /*   shutdown(); */
    /* } else if( msg == "/quit" ) { */
    /*   delConnection( index ); */
    /* } else if( msg.substr( 0, 6 ) == "/name " ) { */
    /*   std::cout << "<" << _clients[index].getName(); */
    /*   _clients[index].setName( msg.substr( 6 ) ); */
    /*   std::cout << " became " << _clients[index].getName() << ">\n"; */
    /* } else { */
    broadcastMsg( msg, index );
    /* } */
  }

  void handleExistingClient( std::size_t index ) {
    char buffer[MAX_BUFFER_SIZE];
    int  clientSocket = _pollfds[index].fd;
    std::memset( buffer, 0, sizeof( buffer ) );
    int bytesRead = recv( clientSocket, buffer, sizeof( buffer ), 0 );

    if( bytesRead < 0 ) {
      std::cerr << "recv: " << strerror( errno ) << "\n";
      exit( 1 );
    } else if( bytesRead == 0 ) {
      std::cout << "server: socket " << _pollfds[index].fd << " hung up\n";
      _disconnectedClients.push_back( index );
      return;
    }
    // Process and respond to the received data

    buffer[bytesRead - 2] = '\0';
    parseData( buffer, index );
  }

  void removeDisconnectedClients() {
    for( std::size_t i = _disconnectedClients.size(); i > 0; --i ) {
      std::size_t index = _disconnectedClients[i - 1];
      close( _pollfds[index].fd );
      std::swap( _pollfds[index], _pollfds.back() );
      _pollfds.pop_back();
      std::swap( _clientSockets[index - 1], _clientSockets.back() );
      _clientSockets.pop_back();
      std::cout << "Client disconnected" << std::endl;
    }
    _disconnectedClients.clear();
  }
};

int main() {
  Server server;
  server.run();
  return 0;
}
