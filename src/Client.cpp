/* src/Client */
/* Created: 230725 08:27:04 by clem@spectre */
/* Updated: 230725 08:27:04 by clem@spectre */
/* Maintainer: Clément Vidon */

#include <iostream>
#include <string>

#include "Client.hpp"

/*  CANON --------------------------------------- */

Client::Client( void ) : _name( "Unknown" ) {}

Client::Client( std::string name ) : _name( name ) {}

Client::Client( Client const& src ) : _name( src._name ) {}

Client::~Client( void ) {}

Client& Client::operator=( Client const& rhs ) {
  if( this == &rhs ) {
    return *this;
  }
  _name = rhs._name;
  return *this;
}

void Client::print( std::ostream& o ) const { o << _name; }

std::ostream& operator<<( std::ostream& o, Client const& i ) {
  i.print( o );
  return o;
}

/*  ACCESSORS ----------------------------------- */

void Client::setName( std::string const& name ) { _name = name; }

std::string Client::getName( void ) const { return _name; }

/* ---------------------------------------------- */
