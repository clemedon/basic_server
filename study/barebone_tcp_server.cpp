#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>

// TODO always check that getaddrinfo() succeed and return a valid entry in the linked list

using namespace std;

// create a socket
// bind the socket to a ip / port
// mark the socket for listening in
// accept a call
// close the listening socket
// while receiving display message, echo message
// close socket

int main() {
  /// Create a socket
  int listening = socket( AF_INET, SOCK_STREAM, 0 );
  // Listening socket takes
  //  AF_INET = ipv4 address family
  //  SOCK_STREAM = the type of socket
  //  0 = the protocol TODO

  // domain:    PF_INET (which is the same value as AF_INET, use AF for struct
  //                    sockaddr_in and PF for socket())
  // type:      SOCK_STREAM or SOCK_DGRAM
  // protocol:  0 to choose the proper protocol for the given type
  //            or by name with getprotobyname("tcp") / getprotobyname("udp")

  if( listening == -1 ) {
    cerr << "Can't create a socket! Quitting" << endl;
    return -1;
  }

  /// Bind the ip address and port to a socket
  sockaddr_in hint;
  //    struct sockaddr_in {
  //      short int          sin_family;  // Address family, AF_INET
  //      unsigned short int sin_port;    // Port number
  //
  //      struct in_addr     sin_addr;    // Internet address
  //      unsigned char      sin_zero[8]; // Same size as struct sockaddr
  //  };
  //
  //  TODO sin_zero is used for padding, to make possible cast with struct
  //  sockaddr?

  hint.sin_family = AF_INET; // IPv4
  hint.sin_port = htons( 54000 );
  // htons = host to network short

  // TODO replace pton with inet_addr
  // pton mean presentation to network (printable to network)
  inet_pton( AF_INET, "0.0.0.0", &( hint.sin_addr ) );
  // TODO returns -1 on error or 0 if the address is messed up
  // make sure the result is greater than 0
  //
  // Convert an IPv4 and IPv6 address in numbers-and-dots notation into a struct
  // in_addr. ||  Convert string IP addresses to their binary representations
  //
  // takes:
  // format of the addr: AF_INET (vs AF_INET6 for ipv6)
  // string: any address (numeric, NOT hostname (use getaddrinfo() for that))
  // buffer: &hint.sin_addr
  // = our binding info, "hint", that we want to bind to our socket

  // p-rintable to n-etwork

  bind( listening, (sockaddr*)&hint, sizeof( hint ) );
  // takes:
  // listening: fd / socket number
  // AF_INET: ipv4
  // hint: the address
  // sizeof hint: the socket len

  /// Tell Winsock the socket is for listening
  listen( listening, SOMAXCONN );
  // listening our socket number
  // SOMAXCONN our max num of connection constant

  /// Wait for a connection
  sockaddr_in client;
  socklen_t   clientSize = sizeof( client );

  // accept  incoming connection
  int clientSocket = accept( listening, (sockaddr*)&client, &clientSize );
  if( clientSocket == -1 ) {
    std::cerr << "Problem with client connecting!\n";
    return -4;
  }

  // we cast client cause sockaddr != sockaddr_in

  char host[NI_MAXHOST];     /// Client's remote name
  char service[NI_MAXSERV];  /// Service (i.e. port) the client is connect on

  // clean up host and service memory areas
  memset( host, 0, NI_MAXHOST );  /// same as memset(host, 0, NI_MAXHOST);
  memset( service, 0, NI_MAXSERV );

  // trying to get and display the name of the computer
  if( getnameinfo( (sockaddr*)&client, sizeof( client ), host, NI_MAXHOST,
                   service, NI_MAXSERV, 0 )
      == 0 ) {
    // takes:
    // address of the socket
    // client size
    // the host
    // max host
    // the service
    // max service
    // flag
    cout << host << " connected on port " << service << endl;
  } else {
    // TODO replace ntop with inet_ntoa
    //       ( AF_INET, &(sa.sin_addr    ), ip4,  INET_ADDRSTRLEN); TODO strlen?
    inet_ntop( AF_INET, &(client.sin_addr), host, NI_MAXHOST );
    // we pass the address type
    // the address
    // a pointer to a string to hold the result
    // and the max len of that string
    // TODO which is the largest IPv? address: INET?_ADDRSTRLEN
    //
    // inet_ntop is opposite of inet_pton => binary to string
    // ntop means network to presentation (network to printable)

    cout << host << " connected on port " << ntohs( client.sin_port ) << endl;
  }

  /// Close listening socket
  close( listening );

  /// While loop: accept and echo message back to client
  char buf[4096];

  while( true ) {
    // clear the buffer
    memset( buf, 0, 4096 );

    // wait for a message
    /// Wait for client to send data
    int bytesReceived = recv( clientSocket, buf, 4096, 0 );
    if( bytesReceived == -1 ) {
      cerr << "Error in recv(). Quitting" << endl;
      break;
    }

    if( bytesReceived == 0 ) {
      cout << "Client disconnected " << endl;
      break;
    }

    // display message
    cout << string( buf, 0, bytesReceived ) << endl;

    // resend message
    /// Echo message back to client
    send( clientSocket, buf, bytesReceived + 1, 0 );
  }

  /// Close the socket
  close( clientSocket );

  return 0;
}
