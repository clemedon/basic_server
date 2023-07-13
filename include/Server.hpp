#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

class Client;

#define MAX_EVENTS 10

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

  void start( void );

 private:
  void createServerSocket( void );
  void handleNewClient( void );
  void handleExistingClient( int clientSocket );

  // TODO ? remove second argument and use Server::_clientSocket
  void parseData( const char* data, int clientSocket );
  void broadcastMsg( std::string& msg, int clientSocket );

  void disconnectAClient( int clientSocket );
  void disconnectAllClients( void );
  void removeDisconnectedClients( void );
  void stop( void );

  int _serverSocket;
  int _epollFd;

  std::map<int, Client> _clients;
  /* TODO int _clientSocket; // instead of int clientSocket args !! */
  std::vector<int> _disconnectedClients;
};

std::ostream& operator<<( std::ostream& o, Server const& i );

#endif  // SERVER_HPP_
