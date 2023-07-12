#define UTILITY_HPP_

#include <iosfwd>
#include <map>
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

  static void        closeFd( int& fd );
  static int         fdIsValid( int fd );
  static int         stringToInt( const std::string& str );
  static std::string intToString( int number );
  static std::string ntop( const struct sockaddr_storage& socket );
  static std::string gaiStrerror( int errorCode );
};

#include <fcntl.h>  // fcntl
#include <netdb.h>
#include <stdlib.h>  // exit XXX
#include <unistd.h>  // close
#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>  // stringstream
#include <string>

/*  CANON
    ------------------------------------------------- */

Utility::Utility( void ) {}

Utility::~Utility( void ) {}

/* ---------------------------------------------- */

/**
 * @brief      Close a socket and set it to -1
 *
 * @param[in]  socket  The socket to close
 */

void Utility::closeFd( int& fd ) {
  if( fd != -1 ) {
    if( ::close( fd ) != -1 ) {
      fd = -1;
    } else {
      std::cerr << "close fd" << fd << ":" << strerror( errno ) << "\n";
      exit( 1 );
    }
  } else {
    std::cout << __func__ << ": fd already close." << std::endl;
  }
}

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
  explicit Client( std::string name, int& socket );
  Client( Client const& src );
  virtual ~Client( void );
  Client&      operator=( Client const& rhs );
  virtual void print( std::ostream& o ) const;

  void        setName( std::string const& name );
  std::string getName( void ) const;
  void        setSocket( int& socket );
  int&        getSocket( void );

 private:
  Client( void );

  std::string _name;
  int&        _socket;
};

std::ostream& operator<<( std::ostream& o, Client const& i );

#include <iostream>
#include <string>

/*  CANON
    ------------------------------------------------- */

/* Client::Client( void ) : _name( "TODO" ), _socket( -1 ) { return; }; */

Client::Client( std::string name, int& socket )
  : _name( name ),
    _socket( socket ) {
  return;
}

Client::Client( Client const& src )
  : _name( src._name ),
    _socket( src._socket ) {}

Client::~Client( void ) { return; }

Client& Client::operator=( Client const& rhs ) {
  if( this == &rhs ) {
    return *this;
  }
  _name = rhs._name;
  _socket = rhs._socket;
  return *this;
}

void Client::print( std::ostream& o ) const { o << _socket << "_" << _name; }

std::ostream& operator<<( std::ostream& o, Client const& i ) {
  i.print( o );
  return o;
}

/*  GETTER SETTER
    ------------------------------------------------- */

void Client::setName( std::string const& name ) { _name = name; }

std::string Client::getName( void ) const { return _name; }

void Client::setSocket( int& socket ) { _socket = socket; }

int& Client::getSocket( void ) { return _socket; }

/* ---------------------------------------------- */

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

/* class Client; */

#define MAX_EVENTS 10

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

  void start( void );

 private:
  void createServerSocket( void );
  void handleNewClient( void );
  void handleExistingClient( int clientSocket );

  // TODO ? remove second argument and use Server::_clientSocket
  void parseData( const char* data, int clientSocket );
  void broadcastMsg( std::string& msg, int clientSocket );

  void disconnectAClient( int clientSocket );
  void disconnectAllClients( void );
  void removeDisconnectedClients( void );
  void stop( void );

  int _serverSocket;
  int _epollFd;

  std::map<int, Client> _clients;
  /* int _clientSocket; // instead of int clientSocket args !!*/
  std::vector<int> _disconnectedClients;
};

std::ostream& operator<<( std::ostream& o, Server const& i );

#include <netdb.h>  // recv, send, sockaddr, accept, addrinfo, getaddrinfo, socket, setsockopt, bind, freeaddrinfo, listen
#include <stdlib.h>     // exit XXX
#include <sys/epoll.h>  // epoll stuff
#include <unistd.h>     // close
#include <cerrno>       // errno
#include <cstring>      // strerror
#include <iostream>     // cerr, cout
#include <string>       // string

#define PORT "4242"

#define MAX_BUFFER_SIZE 1024

/*  CANON
    ------------------------------------------------- */

Server::Server( void ) {
  // init()?
  _serverSocket = -1;
  _epollFd = -1;

  /* add start() and stop() that create/delete _serverSocket and _epollfd */
  /* stop() is called by the destructor and the exceptions ( double free safe )
   */

  createServerSocket();
  return;
}

Server::~Server( void ) {
  stop();
  std::cout << "Server shutting down...\n";
}

void Server::print( std::ostream& o ) const {
  o << _serverSocket;
  // TODO
  // list of connected clients
  // list of disconnected clients
}

std::ostream& operator<<( std::ostream& o, Server const& i ) {
  i.print( o );
  return o;
}

/* ---------------------------------------------- */

/**
 * @brief       Creates a listening server socket.
 */

void Server::createServerSocket( void ) {
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
    _serverSocket = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
    if( _serverSocket < 0 ) {
      continue;
    };
    if( setsockopt( _serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt,
                    sizeof( int ) )
        != 0 ) {
      std::cerr << "setsockopt: " << strerror( errno ) << "\n";
      exit( 1 );
    }
    if( bind( _serverSocket, p->ai_addr, p->ai_addrlen ) != 0 ) {
      Utility::closeFd( _serverSocket );
      continue;
    }
    break;
  }
  freeaddrinfo( res );
  if( p == NULL ) {
    throw std::runtime_error( "Error creating socket" );
  }
  if( listen( _serverSocket, 10 ) == -1 ) {
    throw std::runtime_error( "Error listening on socket" );
  }
  return;
}

/**
 * @brief       Start listening.
 */

void Server::start( void ) {
  int                eventsSize;
  struct epoll_event events[MAX_EVENTS];

  _epollFd = epoll_create( 1 );
  if( _epollFd < 0 ) {
    std::cerr << "epoll_create: " << strerror( errno ) << "\n";
    exit( 1 );
  }
  struct epoll_event event;
  memset( &event, 0, sizeof( event ) );
  event.events = EPOLLIN;  // TODO | EPOLLONESHOT ?
  event.data.fd = _serverSocket;
  if( epoll_ctl( _epollFd, EPOLL_CTL_ADD, event.data.fd, &event ) < 0 ) {
    std::cerr << "epoll_ctl: " << strerror( errno ) << "\n";
    exit( 1 );
  }
  while( _serverSocket != -1 ) {
    eventsSize = epoll_wait( _epollFd, events, MAX_EVENTS, -1 );  // 3.
    if( eventsSize == -1 ) {
      std::cerr << "epoll_wait: " << strerror( errno ) << "\n";
      exit( 1 );
    }
    for( int i = 0; i < eventsSize; i++ ) {
      if( events[i].events & EPOLLIN ) {
        if( events[i].data.fd == _serverSocket ) {
          handleNewClient();
          std::cout << "111" << std::endl;
          std::cout << ">>> " << _clients.at( 5 ) << "\n";
          std::cout << "222" << std::endl;
        } else {
          handleExistingClient( events[i].data.fd );
        }
      }
    }
    removeDisconnectedClients();
  }
}

/**
 * @brief       Handles new client connections.
 */

void Server::handleNewClient( void ) {
  sockaddr_storage clientAddress;
  socklen_t        clientAddressLength;
  int              clientSocket;

  clientAddressLength = sizeof( clientAddress );
  clientSocket = accept( _serverSocket,
                         reinterpret_cast<struct sockaddr*>( &clientAddress ),
                         &clientAddressLength );
  if( clientSocket < 0 ) {
    std::cerr << "accept: " << strerror( errno ) << "\n";
    return;
  }
  std::cout << "New connection from " << Utility::ntop( clientAddress );
  /* std::cout << ":" << ntohs( clientAddress.sin_port ) << std::endl; */
  std::cout << std::endl;

  struct epoll_event event;
  memset( &event, 0, sizeof( event ) );
  event.events = EPOLLIN;  // TODO EPOLLIN | EPOLLONESHOT
  event.data.fd = clientSocket;
  if( epoll_ctl( _epollFd, EPOLL_CTL_ADD, event.data.fd, &event ) < 0 ) {
    std::cerr << "epoll_ctl: " << strerror( errno ) << "\n";
    exit( 1 );
  }

  Client newClient( "Unknown", clientSocket );
  _clients.insert( std::make_pair( clientSocket, newClient ) );

  std::cout << "<" << _clients.at( clientSocket ).getName();
  std::cout << " joined the channel>\n";

  std::cout << "333" << std::endl;
  std::cout << ">>> " << _clients.at( 5 ) << "\n";
  std::cout << "444" << std::endl;
}

/**
 * @brief       Handles communication with existing clients.
 *
 * @param[in]   clientSocket  The client socket
 */

void Server::handleExistingClient( int clientSocket ) {
  char buffer[MAX_BUFFER_SIZE];

  std::memset( buffer, 0, sizeof( buffer ) );
  ssize_t bytesRead = recv( clientSocket, buffer, sizeof( buffer ), 0 );
  if( bytesRead < 0 ) {
    std::cerr << "recv: " << strerror( errno ) << "\n";
    exit( 1 );
  } else if( bytesRead == 0 ) {
    disconnectAClient( clientSocket );
    return;
  }
  buffer[bytesRead - 2] = '\0';
  parseData( buffer, clientSocket );
}

/**
 * @brief       Parses the data received from a client.
 *
 * @param[in]   data   The data received from the client.
 * @param[in]   clientSocket  The client socket
 */

void Server::parseData( const char* data, int clientSocket ) {
  std::string msg( data );
  if( msg == "s" ) {
    Utility::closeFd( _serverSocket );
  } else if( msg == "q" ) {
    disconnectAClient( clientSocket );
  } else if( msg.substr( 0, 6 ) == "n" ) {
    std::cout << "<" << _clients.at( clientSocket ).getName();
    _clients.at( clientSocket ).setName( msg.substr( 6 ) );
    std::cout << " became " << _clients.at( clientSocket ).getName() << ">\n";
  } else {
    broadcastMsg( msg, clientSocket );
  }
}

/**
 * @brief       Broadcasts a message to all connected clients except the sender
 *
 * @param[in]   msg    The message to broadcast.
 * @param[in]   clientSocket  The client socket
 */

void Server::broadcastMsg( std::string& msg, int clientSocket ) {
  std::map<int, Client>::const_iterator it;
  int                                   recipient;

  msg = _clients.at( clientSocket ).getName() + ": " + msg + "\n";
  for( it = _clients.begin(); it != _clients.end(); ++it ) {
    recipient = it->first;
    if( recipient != _serverSocket && recipient != clientSocket ) {
      if( send( recipient, msg.c_str(), msg.length(), 0 ) < 0 ) {
        std::cerr << "send: " << strerror( errno ) << "\n";
        exit( 1 );
      }
    }
  }
  std::cout << msg;
}

/**
 * @brief      Disconnects a client at the specified index.
 *
 * @param[in]  index  The index of the client to disconnect.
 */

void Server::disconnectAClient( int clientSocket ) {
  std::cout << "<" << _clients.at( clientSocket ).getName();
  std::cout << " disconnected>\n";
  _disconnectedClients.push_back( static_cast<int>( clientSocket ) );
}

/**
 * @brief      Disconnects all the clients
 */

void Server::disconnectAllClients() {
  std::map<int, Client>::const_iterator it;

  for( it = _clients.begin(); it != _clients.end(); ++it ) {
    std::cout << "<" << it->second.getName() << " disconnected>\n";
    _disconnectedClients.push_back( it->first );
  }
}

/**
 * @brief      Removes disconnected clients from the server.
 *
 * TODO iterate in reverse order to prevent the same element to removed twice
 * because of swap() logic
 */

void Server::removeDisconnectedClients( void ) {
  std::map<int, Client>::iterator it;

  /* std::cout << _clients.size() << " clients: " << std::endl; */
  /* for( it = _clients.begin(); it != _clients.end(); ++it ) { */
  /* std::cout << "vvvvvvvvvvv" << std::endl; */
  /* std::cout << " " << it->second << "\n"; */
  /* std::cout << " " << it->first << "\n"; */
  /* std::cout << "^^^^^^^^^^^" << std::endl; */
  /* } */

  std::size_t size = _disconnectedClients.size();
  std::cout << _disconnectedClients.size();
  std::cout << " _disconnectedClients: " << std::endl;
  for( std::size_t i = 0; i < size; ++i ) {
    std::cout << " " << _disconnectedClients[i] << "\n";
  }

  std::cout << "END" << std::endl;

  // TODO find where my socket's value get fucked up
  for( std::size_t i = 0; i < size; ++i ) {
    // XXX remove getSocket and use the KEY
    Utility::closeFd( _clients.at( _disconnectedClients[i] ).getSocket() );
    _clients.erase( _disconnectedClients[i] );
  }
  _disconnectedClients.clear();
  std::cout << "<" << size << " clients removed>\n";
}

void Server::stop( void ) {
  disconnectAllClients();
  removeDisconnectedClients();
  /* _clients.clear(); */
  Utility::closeFd( _epollFd );
  Utility::closeFd( _serverSocket );
}

int main() {
  Server server;
  server.start();
  return 0;
}
