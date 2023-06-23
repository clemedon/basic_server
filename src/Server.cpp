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

#include <fcntl.h>  // fcntl

#include "Client.hpp"
#include "Server.hpp"
#include "ServerSocket.hpp"

#define MAX_BUFFER_SIZE "1024"  // Port we're listening on

// TODO WRAP in DEV cause illegal func
int fdIsValid( int fd ) {
  return fcntl( fd, F_GETFD ) != -1 || errno != EBADF;
}

/**
 * @brief       String to Int
 */

int stringToInt( const std::string& str ) {
  std::istringstream iss( str );
  int                result = 0;
  iss >> result;
  return result;
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

Server::Server( void ) {
  pollfd serverPfd;

  _serverSocket.create();
  serverPfd.fd = _serverSocket.get();
  serverPfd.events = POLLIN;
  serverPfd.revents = 0;
  _pollfds.push_back( serverPfd );
}

Server::~Server( void ) {
  shutdown();
}

void Server::run( void ) {
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

void Server::shutdown( void ) {
  std::cout << "Server shutting down...\n";
  for( std::size_t index = 0; index < _pollfds.size(); ++index ) {
    if( _pollfds[index].fd != _serverSocket.get() ) {
      close( _pollfds[index].fd );
    }
  }
  _pollfds.clear();
  _clients.clear();
  /* _listener.close(); */
  exit( 0 );
}

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
  std::cout << "New connection from " << ntop( clientAddress );
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
  _clients.push_back( Client( "Anon_0" + intToString( clientSocket ) ) );

  std::cout << "<Anon_" << clientSocket << " joined the channel>\n";
  return;
}

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
}

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
}

void Server::handleExistingClient( std::size_t index ) {
  char buffer[stringToInt( MAX_BUFFER_SIZE )];
  int  clientSocket = _pollfds[index].fd;
  std::memset( buffer, 0, sizeof( buffer ) );
  int bytesRead = recv( clientSocket, buffer, sizeof( buffer ), 0 );
  if( bytesRead < 0 ) {
    std::cerr << "recv: " << strerror( errno ) << "\n";
    exit( 1 );
  } else if( bytesRead == 0 ) {
    disconnectClient( index );
    return;
  }
  // Process and respond to the received data
  buffer[bytesRead - 2] = '\0';
  parseData( buffer, index );
}

void Server::disconnectClient( std::size_t index ) {
  std::cout << "server: socket " << _pollfds[index].fd << " hung up\n";
  _disconnectedClients.push_back( index );
}

void Server::removeDisconnectedClients( void ) {
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
