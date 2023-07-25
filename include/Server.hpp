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
    Server( Server const& src );
    virtual ~Server( void );
    Server&      operator=( Server const& rhs );
    virtual void print( std::ostream& o ) const;

  private:
    void stop( void );

    void removeDisconnectedClients( void );
    void disconnectAllClients( void );
    void disconnectAClient( int clientSocket );

    void broadcastMsg( std::string& msg, int clientSocket );
    void parseData( const char* data, int clientSocket );

    void handleExistingClient( int clientSocket );
    void handleNewClient( void );
    void createServerSocket( void );

  public:
    void start( void );

  private:
    int                   _serverSocket;
    int                   _epollFd;
    std::map<int, Client> _clients;
    std::vector<int> _disconnectedClients;
};

std::ostream& operator<<( std::ostream& o, Server const& i );

#endif  // SERVER_HPP_
