#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define STDERR  2
#define MYPORT  "4242"  // port users will be connecting to
#define BACKLOG 10      // pending connections queue max size

int main( void ) {
  // int getaddrinfo(
  //  const char* hostname,         // "www.example.com" or IP
  //  const char* service,          // "http", "ftp" or port number "80"
  //  const struct addrinfo* hints, // ai struct filled out with relevant info
  //  struct addrinfo** res         // pointer to the results
  // );

  int              status;
  int              sockfd;
  struct addrinfo  hints;  // hints concerning the type of socket we want
  struct addrinfo* res;    // pointer to the resulting struct addrinfo
  socklen_t        addr_size;
  int              new_fd;
  struct sockaddr_storage
    their_addr;

  memset( &hints, 0, sizeof hints );  // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;        // don't care AF_INET or AF_INET6
  hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;

  // AI_PASSIVE to assign the address of my local host to the socket structures
  // (host) therefor we can pass NULL to getaddrinfo first param instead of
  // an hardcoded hostname/IP if we are a server.

  if( ( status = getaddrinfo( NULL, MYPORT, &hints, &res ) ) != 0 ) {
    dprintf( STDERR, "getaddrinfo error: %s\n", gai_strerror( status ) );
    exit( 1 );
  }

  // TODO walk the "res" linked list looking for valid entries instead of just
  // assuming the first one is good (like many of these examples do).  See the
  // section on client/server for real examples.

  sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );

  bind( sockfd, res->ai_addr, res->ai_addrlen );
  // lose the pesky "Address already in use" error message by allowing to reuse
  // a port that is hogged by an old socket.
  int yes = 1;
  if( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes ) == -1 ) {
    perror( "setsockopt" );
    exit( 1 );
  }

  /* connect( sockfd, res->ai_addr, res->ai_addrlen ); */

  listen( sockfd, BACKLOG );
  // now accept an incoming connection:
  addr_size = sizeof their_addr;
  new_fd = accept( sockfd, (struct sockaddr*)&their_addr, &addr_size );

  freeaddrinfo( res );  // free the linked-list
  return 0;
}
