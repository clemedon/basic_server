#include <netdb.h>  // recv, send, sockaddr, accept, addrinfo, getaddrinfo, socket, setsockopt, bind, freeaddrinfo, listen
#include <stdlib.h>     // exit XXX
#include <sys/epoll.h>  // epoll stuff
#include <unistd.h>     // close
#include <cerrno>       // errno
#include <cstring>      // strerror
#include <iostream>     // cerr, cout
#include <string>       // string

#include "Client.hpp"
#include "Server.hpp"
#include "Utility.hpp"

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
  o << "Server:";
  o << "  Server socket:" << _serverSocket;
  o << "  Connected Clients: " << _disconnectedClients.size() << "\n";
  o << "  Disconnected Clients: " << _disconnectedClients.size() << "\n";
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

  _clients.insert( std::make_pair( clientSocket, Client( "Unknown" ) ) );

  std::cout << "<" << _clients.at( clientSocket ).getName();
  std::cout << " joined the channel>\n";
}

/**
 * @brief       Handles communication with existing clients.
 *
 * @param[in]   clientSocket The client socket
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
 * @param[in]   data The data received from the client.
 * @param[in]   clientSocket The client socket
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
 * @param[in]   msg The message to broadcast.
 * @param[in]   clientSocket The client socket
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
 * @param[in]  index The index of the client to disconnect.
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
 */

void Server::removeDisconnectedClients( void ) {
  std::size_t totalDisconnectedClients = _disconnectedClients.size();
  size_t      totalClients = _clients.size();

  for( std::size_t i = 0; i < totalDisconnectedClients; ++i ) {
    _clients.erase( _disconnectedClients[i] );
    Utility::closeFd( _disconnectedClients[i] );
  }
  // DEV_BEG
  if( totalDisconnectedClients ) {
    std::cout << "<" << totalDisconnectedClients << "/" << totalClients;
    std::cout << "_clients removed>\n";
  }
  // END_END
  _disconnectedClients.clear();
}

void Server::stop( void ) {
  disconnectAllClients();
  removeDisconnectedClients();
  /* _clients.clear(); */
  Utility::closeFd( _epollFd );
  Utility::closeFd( _serverSocket );
}
