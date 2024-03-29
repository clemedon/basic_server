/* src/Utility */
/* Created: 230725 08:27:19 by clem@spectre */
/* Updated: 230725 08:27:19 by clem@spectre */
/* Maintainer: Clément Vidon */

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "Utility.hpp"

/*  CANON --------------------------------------- */

Utility::Utility( void ) {}

Utility::Utility( Utility const& src ) { *this = src; }

Utility::~Utility( void ) {}

Utility& Utility::operator=( Utility const& rhs ) {
  if( this == &rhs ) {
    return *this;
  }
  return *this;
}

/* IMPLEMENTATION ------------------------------- */

/**
 * @brief       Check if a file descriptor is valid.
 */

bool Utility::isSocketNonBlocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    return (flags & O_NONBLOCK) != 0;
}

/**
 * @brief      Close a file descriptor and set it to -1
 */

void Utility::closeFd( int& fd ) {
  if( fd != -1 ) {
    if( ::close( fd ) != -1 ) {
      fd = -1;
    } else {
      std::string message = "close fd" + intToString( fd ) + ":";
      message += std::string( strerror( errno ) );
      throw std::runtime_error( message );
    }
  } else {
    std::cout << __func__ << ": fd already close." << std::endl;
  }
}

/**
 * @brief       Check if a file descriptor is valid.
 */

bool Utility::isFdValid( int fd ) {
  return fcntl( fd, F_GETFD ) != -1 || errno != EBADF;
}

/**
 * @brief       Convert a string to an integer.
 */

int Utility::stringToInt( const std::string& str ) {
  std::istringstream iss( str );
  int                result = 0;
  iss >> result;
  return result;
}

/**
 * @brief       Convert an integer to a string.
 */

std::string Utility::intToString( int number ) {
  std::ostringstream oss;
  oss << number;
  return oss.str();
}

/**
 * @brief       Convert a network address to a string representation.
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
    ss << static_cast<int>( addr[3] );
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
 * @brief       Error messages corresponding to getaddrinfo error code.
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
