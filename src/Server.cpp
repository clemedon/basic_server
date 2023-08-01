/* src/Server */
/* Created: 230725 06:55:47 by clem@spectre */
/* Updated: 230727 16:53:28 by clem@spectre */
/* Maintainer: Clément Vidon */

#include <netdb.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

#include "Client.hpp"
#include "Server.hpp"
#include "Utility.hpp"

#define PORT            "6667"
#define MAX_CONNECTIONS  100
#define BUFFER_SIZE      20
#define MAX_MESSAGE_SIZE 30
#define CRLF             std::string( "\r\n" )

/*  CANON ------------------------------------------- */

Server::Server( void ) {
  _serverSocket = -1;
  _epollFd = -1;
  createServerSocket();
  std::cout << "Server initialized.\n";
  return;
}

Server::~Server( void ) {
  stop();
  std::cout << "Server shutting down...\n";
}

Server::Server( const Server& src )
  : _serverSocket( src._serverSocket ),
    _epollFd( src._epollFd ) {
  std::map<int, Client>::const_iterator it;

  for( it = src._clients.begin(); it != src._clients.end(); ++it ) {
    _clients[it->first] = Client( it->second );
  }
  _disconnectedClients = src._disconnectedClients;
}

Server& Server::operator=( Server const& rhs ) {
  std::map<int, Client>::const_iterator it;

  if( this == &rhs ) {
    return *this;
  }
  _serverSocket = rhs._serverSocket;
  _epollFd = rhs._epollFd;
  for( it = rhs._clients.begin(); it != rhs._clients.end(); ++it ) {
    _clients[it->first] = Client( it->second );
  }
  _disconnectedClients = rhs._disconnectedClients;
  return *this;
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
 * @brief       Stop the server.
 */

void Server::stop( void ) {
  disconnectAllClients();
  removeDisconnectedClients();
  Utility::closeFd( _epollFd );
  Utility::closeFd( _serverSocket );
}

/**
 * @brief      Removes disconnected clients from the server.
 */

void Server::removeDisconnectedClients( void ) {
  std::size_t disconnectCientsNumber = _disconnectedClients.size();
  size_t      clientsNumber = _clients.size();

  for( std::size_t i = 0; i < disconnectCientsNumber; ++i ) {
    _clients.erase( _disconnectedClients[i] );
    Utility::closeFd( _disconnectedClients[i] );
  }
  if( disconnectCientsNumber != 0 ) {
    std::cout << "<" << disconnectCientsNumber << "/" << clientsNumber
              << "_clients removed>\n";
  }
  _disconnectedClients.clear();
}

/**
 * @brief      Disconnects all the clients.
 */

void Server::disconnectAllClients() {
  std::map<int, Client>::const_iterator it;

  for( it = _clients.begin(); it != _clients.end(); ++it ) {
    std::cout << "<" << it->second.getName() << " disconnected>\n";
    _disconnectedClients.push_back( it->first );
  }
}

/**
 * @brief      Disconnects a client at the specified index.
 */

void Server::disconnectAClient( int clientSocket ) {
  std::cout << "<" << _clients.at( clientSocket ).getName()
            << " disconnected>\n";
  _disconnectedClients.push_back( static_cast<int>( clientSocket ) );
}

/**
 * @brief       Broadcasts a message to all connected clients except the sender.
 */

void Server::broadcastMsg( std::string& msg, int clientSocket ) {
  std::map<int, Client>::const_iterator it;
  int                                   recipient;

  msg = _clients.at( clientSocket ).getName() + ": " + msg + "\n";
  for( it = _clients.begin(); it != _clients.end(); ++it ) {
    recipient = it->first;
    if( recipient != _serverSocket && recipient != clientSocket ) {
      if( send( recipient, msg.c_str(), msg.length(), 0 ) < 0 ) {
        std::string message = "send: " + std::string( strerror( errno ) );
        throw std::runtime_error( message );
      }
    }
  }
  std::cout << msg;
}

/**
 * @brief       Parses the data received from a client.
 */

void Server::parseData( std::string& msg, int clientSocket ) {
  if( msg == "s" ) {
    Utility::closeFd( _serverSocket );
  } else if( msg == "q" ) {
    disconnectAClient( clientSocket );
  } else if( msg[0] == 'n' && msg[1] == ' ' ) {
    std::cout << "<" << _clients.at( clientSocket ).getName();
    _clients.at( clientSocket ).setName( msg.substr( 2 ) );
    std::cout << " became " << _clients.at( clientSocket ).getName() << ">\n";
  } else {
    broadcastMsg( msg, clientSocket );
  }
}

/**
 * @brief       Handles communication with existing clients.
 */

void Server::handleExistingClient( int clientSocket ) {
  char        buf[BUFFER_SIZE];
  ssize_t     bytesRead = 0;
  std::string message;

  std::memset( buf, 0, BUFFER_SIZE );

  while( true ) {
    bytesRead = recv( clientSocket, buf, sizeof( buf ), 0 );

    if( bytesRead < 0 ) {
      std::string message = "recv: " + std::string( strerror( errno ) );
      throw std::runtime_error( message );
    } else if( bytesRead == 0 ) {
      disconnectAClient( clientSocket );
      return;
    }

    message += std::string( buf, static_cast<size_t>( bytesRead ) );

    if( message.size() > MAX_MESSAGE_SIZE ) {
      std::cout << "Error: Received message is too long.\n";
      message.clear();
      char tempBuf[BUFFER_SIZE];
      while( recv( clientSocket, tempBuf, BUFFER_SIZE, MSG_DONTWAIT ) > 0 ) {
      }
      return;
    }
    size_t delimiterPos = message.find( CRLF );
    if( delimiterPos != std::string::npos ) {
      message = message.substr( 0, delimiterPos );
      parseData( message, clientSocket );
      message.erase( 0, message.find( CRLF ) +  CRLF.size() );
      return;
    }
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
  std::cout << "New connection from " << Utility::ntop( clientAddress ) << "\n";
  /* std::cout << ":" << ntohs( clientAddress.sin_port ) << "\n"; */
  struct epoll_event event;
  memset( &event, 0, sizeof( event ) );
  event.events = EPOLLIN;
  event.data.fd = clientSocket;
  if( epoll_ctl( _epollFd, EPOLL_CTL_ADD, event.data.fd, &event ) < 0 ) {
    std::string message = "epoll_ctl: " + std::string( strerror( errno ) );
    throw std::runtime_error( message );
  }
  _clients.insert( std::make_pair( clientSocket, Client( "Unknown" ) ) );
  std::cout << "<" << _clients.at( clientSocket ).getName()
            << " joined the channel>\n";
}

/**
 * @brief       Creates a listening server socket.
 */

void Server::createServerSocket( void ) {
  int             opt = 1;
  int             status;
  struct addrinfo hints, *res, *p;

  std::memset( &hints, 0, sizeof( hints ) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  status = getaddrinfo( NULL, PORT, &hints, &res );
  if( status != 0 ) {
    std::string message = "selectserver: " + Utility::gaiStrerror( status );
    throw std::runtime_error( message );
  }
  for( p = res; p != NULL; p = p->ai_next ) {
    _serverSocket = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
    if( _serverSocket < 0 ) {
      continue;
    };
    if( setsockopt( _serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt,
                    sizeof( int ) )
        != 0 ) {
      freeaddrinfo( res );
      std::string message = "setsockopt: " + std::string( strerror( errno ) );
      throw std::runtime_error( message );
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
  struct epoll_event event;

  _epollFd = epoll_create( 1 );
  if( _epollFd < 0 ) {
    std::string message = "epoll_create: " + std::string( strerror( errno ) );
    throw std::runtime_error( message );
  }
  memset( &event, 0, sizeof( event ) );
  event.events = EPOLLIN;
  event.data.fd = _serverSocket;
  if( epoll_ctl( _epollFd, EPOLL_CTL_ADD, event.data.fd, &event ) < 0 ) {
    std::string message = "epoll_ctl: " + std::string( strerror( errno ) );
    throw std::runtime_error( message );
  }
  std::cout << "Ready for incoming connection PORT " << PORT << " :)\n";
  while( _serverSocket != -1 ) {
    eventsSize = epoll_wait( _epollFd, events, MAX_EVENTS, -1 );
    if( eventsSize == -1 ) {
      std::string message = "epoll_wait: " + std::string( strerror( errno ) );
      throw std::runtime_error( message );
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
