#include <iostream>
#include <string>

#include "Client.hpp"

/*  CANON
------------------------------------------------- */

/* Client::Client( void ) : _name( "TODO" ) { return; }; */

Client::Client( std::string name ) : _name( name ) { return; }

Client::Client( Client const& src ) : _name( src._name ) {}

Client::~Client( void ) { return; }

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

/*  GETTER SETTER
------------------------------------------------- */

void Client::setName( std::string const& name ) { _name = name; }

std::string Client::getName( void ) const { return _name; }

/* ---------------------------------------------- */
