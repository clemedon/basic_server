#include "Server.hpp"

#include <iostream>
#include <string>

int main() {
  try {
    Server server;
    server.start();
  } catch( const std::runtime_error& e ) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
