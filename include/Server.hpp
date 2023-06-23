#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <poll.h>  // pollfds
#include <iosfwd>
#include <string>
#include <vector>

#include "ServerSocket.hpp"
class Client;

/**
 * @brief       Handles the communication between multiple clients, managing
 *              client connections, receiving and sending data, and handling
 *              disconnections.
 */

class Server {
 public:
  Server( void );
  /* Server( Server const& src ); */
  virtual ~Server( void );
  /* Server&      operator=( Test const& rhs ); */
  virtual void print( std::ostream& o ) const;

  void run( void );

 private:
  void handleNewClient( void );                              // accept
  void handleExistingClient( std::size_t index );            // recv, send
  void parseData( const char* data, std::size_t index );     //
  void broadcastMsg( std::string& msg, std::size_t index );  // send
  void disconnectClient( std::size_t index );                //
  void removeDisconnectedClients( void );                    // close
  void shutdown( void );                                     // close

  ServerSocket        _serverSocket;
  std::vector<int>    _clientSockets;
  std::vector<int>    _disconnectedClients;
  std::vector<Client> _clients;
  std::vector<pollfd> _pollfds;
};

std::ostream& operator<<( std::ostream& o, Server const& i );

#endif  // SERVER_HPP_
