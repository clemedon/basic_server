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

#include "ServerSocket.hpp"

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
















ServerSocket::ServerSocket( void ) : _fd( -1 ) {
}

ServerSocket::~ServerSocket( void ) {
  close();
}

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
    std::cerr << "selectserver: " << gaiStrerror( status ) << "\n";
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
}

int ServerSocket::get( void ) {
  return _fd;
}

void ServerSocket::close( void ) {
  if( _fd != -1 ) {
    ::close( _fd );
    _fd = -1;
  }
}
