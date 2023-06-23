#include <netdb.h>  // addrinfo, getaddrinfo, socket, setsockopt, bind, freeaddrinfo, listen
#include <stdlib.h>  // exit XXX
#include <unistd.h>  // close
#include <cerrno>    // errno
#include <cstring>   // strerror
#include <iostream>  // cerr, cout

#include "ServerSocket.hpp"
#include "Utility.hpp"

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
