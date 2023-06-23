#ifndef SERVERSOCKET_HPP_
#define SERVERSOCKET_HPP_

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
 * @brief       ServerSocket
 */

class ServerSocket {
 private:
  int _fd;

 public:
  ServerSocket();
  ~ServerSocket( void );

  void create( void );
  int  get( void );

 private:
  void close( void );
};

#endif  // SERVERSOCKET_HPP_
