#ifndef SERVER_HPP_
#define SERVER_HPP_

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

#include "Client.hpp"
#include "ServerSocket.hpp"

class Server {
 private:
  ServerSocket        _serverSocket;
  std::vector<int>    _clientSockets;  // TODO managed by the server, no raii
  std::vector<int>    _disconnectedClients;
  std::vector<pollfd> _pollfds;
  std::vector<Client> _clients;  // Pollable file descriptors

 public:
  Server( void );
  ~Server( void );

  void run( void );

 private:
  void shutdown( void );                                     // close
  void handleNewClient( void );                              // accept
  void broadcastMsg( std::string& msg, std::size_t index );  // send
  void parseData( const char* data, std::size_t index );
  void handleExistingClient( std::size_t index );  // recv, send
  void disconnectClient( std::size_t index );      // close
  void removeDisconnectedClients( void );          // close
};

#endif  // SERVER_HPP_
