#include <iostream>
#include <string>

#include "Server.hpp"

/*  CANON
------------------------------------------------- */

Server::Server( void ) {
  return;
}

Server::Server( Server const& src ) {
  *this = src;
  return;
}

Server::~Server( void ) {
  return;
}

Server& Server::operator=( Server const& rhs ) {
  if( this == &rhs ) {
    return *this;
  }
  return *this;
}

void Server::print( std::ostream& o ) const {
  o << "";  // TODO
  return;
}

std::ostream& operator<<( std::ostream& o, Server const& i ) {
  i.print( o );
  return o;
}

/* ---------------------------------------------- */
