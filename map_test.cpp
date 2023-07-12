#include <iostream>
#include <map>
#include <string>

int main() {
  std::map<int, std::string> _clients;
  /* _clients[0] = "One"; */

  _clients.insert( std::make_pair( 0, "One" ) );
    for( std::size_t i = 0; i < _clients.size(); ++i ) {
    std::cout << ">>cli: " << i << " -> " << _clients[i] << std::endl;
  }


  /* for( size_t i = 0; i < _clients.size(); i++ ) { */
  /*   std::cout << "Key: " << i << ", Value: " << _clients[i] << std::endl; */
  /* } */

  return 0;
}
