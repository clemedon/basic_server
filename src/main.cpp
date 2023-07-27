#include "Server.hpp"

#include <csignal>
#include <iostream>
#include <string>

/**
 * @brief       SINGINT ^-C Handling
 */

void signalHandler( int signal ) {
  if( signal == SIGINT ) {
    throw std::runtime_error( "SIGINT" );
  }
}

int main() {
  signal( SIGINT, signalHandler );
  try {
    Server server;
    server.start();
  } catch( const std::runtime_error& e ) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
