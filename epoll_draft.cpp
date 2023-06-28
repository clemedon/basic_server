#define UTILITY_HPP_

#include <iosfwd>
#include <string>

/**
 * @brief       Various utility functions.
 */

class Utility {
 public:
  Utility( void );
  /* Utility( Utility const& src ); */
  virtual ~Utility( void );
  /* Utility& operator=( Utility const& rhs ); */

  static int         fdIsValid( int fd );
  static int         stringToInt( const std::string& str );
  static std::string intToString( int number );
  static std::string ntop( const struct sockaddr_storage& socket );
  static std::string gaiStrerror( int errorCode );
};

#include <fcntl.h>  // fcntl
#include <netdb.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>  // stringstream
#include <string>

/*  CANON
------------------------------------------------- */

Utility::Utility( void ) {
  return;
}

Utility::~Utility( void ) {
  return;
}

/* ---------------------------------------------- */

/**
 * @brief       Check if the file descriptor is valid.
 *
 * @param[in]   fd  The file descriptor to check.
 * @return      Returns 1 if the file descriptor is valid, otherwise 0.
 */

int Utility::fdIsValid( int fd ) {
  return fcntl( fd, F_GETFD ) != -1 || errno != EBADF;
}

/**
 * @brief       Convert a string to an integer.
 *
 * @param[in]   str The string to convert.
 * @return      Returns the converted integer value.
 */

int Utility::stringToInt( const std::string& str ) {
  std::istringstream iss( str );
  int                result = 0;
  iss >> result;
  return result;
}

/**
 * @brief       Convert an integer to a string.
 *
 * @param[in]   number The integer to convert.
 * @return      Returns the converted string value.
 */

std::string Utility::intToString( int number ) {
  std::ostringstream oss;
  oss << number;
  return oss.str();
}

/**
 * @brief       Convert a network address to a string representation.
 *
 * @param[in]   socket The network address to convert.
 * @return      Returns the string representation of the network address.
 */

std::string Utility::ntop( const struct sockaddr_storage& socket ) {
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
 * @brief       Get the error message for a given error code from getaddrinfo.
 *
 * @param[in]   errorCode The error code.
 * @return      Returns the error message corresponding to the error code.
 */

std::string Utility::gaiStrerror( int errorCode ) {
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

#include <iostream>
#include <string>

/*  CANON
------------------------------------------------- */

Client::Client( std::string name ) : _name( name ) {
  return;
}

Client::Client( Client const& src ) : _name( src._name ) {
  return;
}

Client::~Client( void ) {
  return;
}

Client& Client::operator=( Client const& rhs ) {
  if( this == &rhs ) {
    return *this;
  }
  _name = rhs._name;
  return *this;
}

void Client::print( std::ostream& o ) const {
  o << _name;
  return;
}

std::ostream& operator<<( std::ostream& o, Client const& i ) {
  i.print( o );
  return o;
}

/*  GETTER SETTER
------------------------------------------------- */

void Client::setName( std::string const& name ) {
  _name = name;
  return;
}

std::string& Client::getName( void ) {
  return _name;
}

/* ---------------------------------------------- */
#define SERVERSOCKET_HPP_

#include <iosfwd>
#include <string>

/** @brief      Represents a server-side listening socket and manages the
 *              underlying socket file descriptor. It follows the RAII principle
 *              by automatically managing the socket life cycle.
 */

class ServerSocket {
 public:
  ServerSocket( void );
  /* ServerSocket( ServerSocket const& src ); */
  virtual ~ServerSocket( void );
  /* ServerSocket& operator=( ServerSocket const& rhs ); */
  virtual void print( std::ostream& o ) const;

  int  get( void );
  void create( void );

 private:
  void close( void );

  int _fd;
};

std::ostream& operator<<( std::ostream& o, ServerSocket const& i );

#include <netdb.h>  // addrinfo, getaddrinfo, socket, setsockopt, bind, freeaddrinfo, listen
#include <stdlib.h>  // exit XXX
#include <unistd.h>  // close
#include <cerrno>    // errno
#include <cstring>   // strerror
#include <iostream>  // cerr, cout

#define PORT "4242"

/*  CANON
------------------------------------------------- */

ServerSocket::ServerSocket( void ) : _fd( -1 ) {
  return;
}

ServerSocket::~ServerSocket( void ) {
  close();
  return;
}

void ServerSocket::print( std::ostream& o ) const {
  o << _fd;
  return;
}

std::ostream& operator<<( std::ostream& o, ServerSocket const& i ) {
  i.print( o );
  return o;
}

/*  GETTER SETTER
------------------------------------------------- */

int ServerSocket::get( void ) {
  return _fd;
}

/* ---------------------------------------------- */

/**
 * @brief       Creates a listening server socket.
 */

void ServerSocket::create( void ) {
  int             opt = 1;  // For setsockopt() SO_REUSEADDR, below
  int             status;
  struct addrinfo hints, *res, *p;

  std::memset( &hints, 0, sizeof( hints ) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  status = getaddrinfo( NULL, PORT, &hints, &res );
  if( status != 0 ) {
    std::cerr << "selectserver: " << Utility::gaiStrerror( status ) << "\n";
    exit( 1 );
  }
  for( p = res; p != NULL; p = p->ai_next ) {
    _fd = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
    if( _fd < 0 ) {
      continue;
    };
    if( setsockopt( _fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( int ) )
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
  return;
}

/**
 * @brief       Closes the listening server socket.
 */

void ServerSocket::close( void ) {
  if( _fd != -1 ) {
    ::close( _fd );
    _fd = -1;
  }
  return;
}
#define SERVER_HPP_

#include <poll.h>  // pollfds
#include <iosfwd>
#include <string>
#include <vector>

class Client;

/**
 * @brief       Handles the communication between multiple clients, managing
 *              client connections, receiving and sending data, and handling
 *              disconnections.
 */

class Server {
 public:
  Server( void );
  /* Server( Server const& src ); */
  virtual ~Server( void );
  /* Server&      operator=( Test const& rhs ); */
  virtual void print( std::ostream& o ) const;

  void run( void );

 private:
  void handleNewClient( void );                              // accept
  void handleExistingClient( std::size_t index );            // recv, send
  void parseData( const char* data, std::size_t index );     //
  void broadcastMsg( std::string& msg, std::size_t index );  // send
  void disconnectClient( std::size_t index );                //
  void removeDisconnectedClients( void );                    // close
  void shutdown( void );                                     // close

  ServerSocket        _serverSocket;
  std::vector<int>    _clientSockets;
  std::vector<int>    _disconnectedClients;
  std::vector<Client> _clients;
  std::vector<pollfd> _pollfds;
};

std::ostream& operator<<( std::ostream& o, Server const& i );

#include <netdb.h>   // recv, send, sockaddr, accept
#include <stdlib.h>  // exit XXX
#include <unistd.h>  // close
#include <cerrno>    // errno
#include <cstring>   // strerror
#include <iostream>  // cerr, cout
#include <string>    // string

#define MAX_BUFFER_SIZE 1024

/*  CANON
------------------------------------------------- */

Server::Server( void ) {
  pollfd serverPfd;

  _serverSocket.create();
  serverPfd.fd = _serverSocket.get();
  serverPfd.events = POLLIN;
  serverPfd.revents = 0;
  _pollfds.push_back( serverPfd );
  return;
}

Server::~Server( void ) {
  shutdown();
}

void Server::print( std::ostream& o ) const {
  o << _serverSocket;
  // TODO print the list of connected clients
  // TODO print the list of disconnected clients
  return;
}

std::ostream& operator<<( std::ostream& o, Server const& i ) {
  i.print( o );
  return o;
}

/* ---------------------------------------------- */

/**
 * @brief       Runs the server.
 */

#include "sys/epoll.h"
void Server::run( void ) {
  // Create the epoll instance
  int epollFd = epoll_create( 1 );
  if( epollFd == -1 ) {
    /* perror( "epoll_create" ); */
    return;
  }

  // Create and initialize the epoll event structure
  struct epoll_event event;
  event.data.fd = _serverSocket.get();
  event.events = EPOLLIN | EPOLLET;  // Enable edge-triggered behavior and
                                     // listen for input events

  // Add the server socket to the epoll instance
  if( epoll_ctl( epollFd, EPOLL_CTL_ADD, _serverSocket.get(), &event ) == -1 ) {
    /* perror( "epoll_ctl" ); */
    close( epollFd );
    return;
  }

  // Buffer to store the events returned by epoll_wait
  const int          maxEvents = 10;
  struct epoll_event events[maxEvents];

  while( true ) {
    // Wait for events to occur
    int numEvents = epoll_wait( epollFd, events, maxEvents, -1 );
    if( numEvents == -1 ) {
      /* perror( "epoll_wait" ); */
      close( epollFd );
      return;
    }

    std::cout << "numEvents ----→ " << numEvents << std::endl;

    // Handle the events

    for( int i = 0; i < numEvents; ++i ) {
      if( events[i].data.fd == _serverSocket.get() ) {
        // Accept a new client connection
        handleNewClient();
      } else {
        std::cout << "helllloo" << std::endl;
        // Handle an existing client
        handleExistingClient( events[i].data.fd );
      }
    }
  }
  // Close the epoll instance
  close( epollFd );
}

/**
 * @brief       Handles new client connections.
 */

void Server::handleNewClient( void ) {
  sockaddr_storage clientAddress;
  socklen_t        clientAddressLength;
  int              clientSocket;
  pollfd           clientPfd;

  clientAddressLength = sizeof( clientAddress );
  clientSocket = accept( _serverSocket.get(),
                         reinterpret_cast<struct sockaddr*>( &clientAddress ),
                         &clientAddressLength );
  if( clientSocket < 0 ) {
    std::cerr << "accept: " << strerror( errno ) << "\n";
    return;
  }
  std::cout << "New connection from " << Utility::ntop( clientAddress );
  /* std::cout << ":" << ntohs( clientAddress.sin_port ) << std::endl; */
  std::cout << std::endl;

  _clientSockets.push_back( clientSocket );
  // Add the client socket to _pollfds

  clientPfd.fd = clientSocket;
  clientPfd.events = POLLIN;
  clientPfd.revents = 0;
  _pollfds.push_back( clientPfd );

  if( _clients.size() == 0 ) {
    _clients.push_back( Client( "NONE" ) );
  }
  _clients.push_back(
    Client( "Anon_0" + Utility::intToString( clientSocket ) ) );

  std::cout << "<Anon_" << clientSocket << " joined the channel>\n";
  return;
}

/**
 * @brief       Handles communication with existing clients.
 *
 * @param[in]   index  The index of the client in the _pollfds vector.
 */

void Server::handleExistingClient( std::size_t index ) {
  char buffer[MAX_BUFFER_SIZE];
  int  clientSocket = _pollfds[index].fd;
  std::memset( buffer, 0, sizeof( buffer ) );
  ssize_t bytesRead = recv( clientSocket, buffer, sizeof( buffer ), 0 );
  if( bytesRead < 0 ) {
    std::cerr << "recv: " << strerror( errno ) << "\n";
    exit( 1 );
  } else if( bytesRead == 0 ) {
    disconnectClient( index );
    return;
  }
  buffer[bytesRead - 2] = '\0';
  parseData( buffer, index );
  return;
}

/**
 * @brief       Parses the data received from a client.
 *
 * @param[in]   data   The data received from the client.
 * @param[in]   index  The index of the client in the _pollfds vector.
 */

void Server::parseData( const char* data, std::size_t index ) {
  std::string msg( data );
  if( msg == "/shutdown" ) {
    shutdown();
  } else if( msg == "/quit" ) {
    disconnectClient( index );
  } else if( msg.substr( 0, 6 ) == "/name " ) {
    std::cout << "<" << _clients[index].getName();
    _clients[index].setName( msg.substr( 6 ) );
    std::cout << " became " << _clients[index].getName() << ">\n";
  } else {
    broadcastMsg( msg, index );
  }
  return;
}

/**
 * @brief       Broadcasts a message to all connected clients except the one at
 *              the specified index.
 *
 * @param[in]   msg    The message to broadcast.
 * @param[in]   index  The index of the client to exclude from broadcasting.
 */

void Server::broadcastMsg( std::string& msg, std::size_t index ) {
  int recipient;
  int clientSocket = _pollfds[index].fd;

  msg = _clients[index].getName() + ": " + msg + "\n";
  for( std::size_t i = 0; i < _pollfds.size(); ++i ) {
    recipient = _pollfds[i].fd;
    if( recipient != _serverSocket.get() && recipient != clientSocket ) {
      if( send( recipient, msg.c_str(), msg.length(), 0 ) < 0 ) {
        std::cerr << "send: " << strerror( errno ) << "\n";
        exit( 1 );
      }
    }
  }
  std::cout << msg;
  return;
}

/**
 * @brief      Disconnects a client at the specified index.
 *
 * @param[in]  index  The index of the client to disconnect.
 */

void Server::disconnectClient( std::size_t index ) {
  std::cout << "server: socket " << _pollfds[index].fd << " hung up\n";
  _disconnectedClients.push_back( static_cast<int>( index ) );
  return;
}

/**
 * @brief      Removes disconnected clients from the server.
 */

void Server::removeDisconnectedClients( void ) {
  // TODO remove corresponding Client object
  for( std::size_t i = _disconnectedClients.size(); i > 0; --i ) {
    std::size_t index = static_cast<std::size_t>( _disconnectedClients[i - 1] );
    close( _pollfds[index].fd );
    std::swap( _pollfds[index], _pollfds.back() );
    _pollfds.pop_back();
    std::swap( _clientSockets[index - 1], _clientSockets.back() );
    _clientSockets.pop_back();
    std::cout << "Client disconnected" << std::endl;
  }
  _disconnectedClients.clear();
  return;
}

/**
 * @brief      Shuts down the server and closes all connections.
 */

void Server::shutdown( void ) {
  std::cout << "Server shutting down...\n";
  for( std::size_t index = 0; index < _pollfds.size(); ++index ) {
    if( _pollfds[index].fd != _serverSocket.get() ) {
      close( _pollfds[index].fd );
    }
  }
  _pollfds.clear();
  _clients.clear();
  exit( 0 );
  return;
}

int main() {
  Server server;
  server.run();
  return 0;
}

