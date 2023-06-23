#include <netdb.h>   // recv, send, sockaddr, accept
#include <stdlib.h>  // exit XXX
#include <unistd.h>  // close
#include <cerrno>    // errno
#include <cstring>   // strerror
#include <iostream>  // cerr, cout
#include <string>    // string

#include "Client.hpp"
#include "Server.hpp"
#include "ServerSocket.hpp"
#include "Utility.hpp"

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
  return;
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
