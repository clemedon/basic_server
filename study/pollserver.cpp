#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// perror -> errno ?
// realloc -> vector ?
// malloc -> new
// exit ?
// memset ?
// gai_strerror ?
// stderr FDSTDERR..
// fprintf -> cout <<

#define PORT "4242"  // Port we're listening on

/* void* getInAddr( struct sockaddr* sa ) { */
/*   if( sa->sa_family == AF_INET ) { */
/*     return &( ( (struct sockaddr_in*)sa )->sin_addr ); */
/*   } */
/*   return &( ( (struct sockaddr_in6*)sa )->sin6_addr ); */
/* } */

class Server {
 public:
  void run();

 private:
  int            listener;  // Listening socket descriptor
  struct pollfd* pfds;      // Array of poll file descriptors
  int            fdCount;   // Number of file descriptors in the array
  int            fdSize;    // Size of the file descriptor array

  void delFromPfds( int i );
  void handleDataFromClient( int i );
  void addToPfds( int newfd );
  void handleNewConnection();
  int  getListenerSocket();
  void setup();
};

void Server::delFromPfds( int i ) {
  // Copy the one from the end over this one
  pfds[i] = pfds[fdCount - 1];
  fdCount--;
}

void Server::handleDataFromClient( int i ) {
  char buf[256];  // Buffer for client data
  int  nbytes = recv( pfds[i].fd, buf, sizeof buf, 0 );
  int  senderFd = pfds[i].fd;

  if( nbytes <= 0 ) {
    // Got error or connection closed by client
    if( nbytes == 0 ) {
      // Connection closed
      std::cout << "server: socket " << senderFd << " hung up\n";
      /* printf( "pollserver: socket %d hung up\n", senderFd ); */
    } else {
      perror( "recv" );
    }

    close( pfds[i].fd );  // Bye!

    delFromPfds( i );

  } else {
    // We got some good data from a client

    for( int j = 0; j < fdCount; j++ ) {
      // Send to everyone!
      int destFd = pfds[j].fd;

      // Except the listener and ourselves
      if( destFd != listener && destFd != senderFd ) {
        if( send( destFd, buf, nbytes, 0 ) == -1 ) {
          perror( "send" );
        }
      }
    }
  }
}

void Server::addToPfds( int newfd ) {
  // If we don't have room, add more space in the pfds array
  if( fdCount == fdSize ) {
    fdSize *= 2;  // Double it
    pfds = (struct pollfd*)realloc( pfds, sizeof( struct pollfd ) * fdSize );
  }

  pfds[fdCount].fd = newfd;
  pfds[fdCount].events = POLLIN;  // Check ready-to-read

  fdCount++;
}

void Server::handleNewConnection() {
  int                     newfd;       // Newly accept()ed socket descriptor
  struct sockaddr_storage remoteaddr;  // Client address
  socklen_t               addrlen;
  addrlen = sizeof remoteaddr;
  newfd = accept( listener, (struct sockaddr*)&remoteaddr, &addrlen );

  if( newfd == -1 ) {
    perror( "accept" );
  } else {
    addToPfds( newfd );

    // TODO replace inet_ntop with inet_ntoa
    /* char remoteIP[INET6_ADDRSTRLEN]; */
    /* printf( "pollserver: new connection from %s on socket %d\n", */
    /*         inet_ntop( remoteaddr.ss_family, */
    /*                    getInAddr( (struct sockaddr*)&remoteaddr ),
     * remoteIP, */
    /*                    INET6_ADDRSTRLEN ), newfd ); */
  }
}

void Server::run() {
  setup();  // Main loop
  while( 1 ) {
    int poll_count = poll( pfds, fdCount, -1 );

    if( poll_count == -1 ) {
      perror( "poll" );
      exit( 1 );
    }

    // Run through the existing connections looking for data to read
    for( int i = 0; i < fdCount; i++ ) {
      // Check if someone's ready to read
      if( pfds[i].revents & POLLIN ) {  // We got one!!
        if( pfds[i].fd == listener ) {
          // If listener is ready to read, handle new connection
          handleNewConnection();
        } else {
          // If not the listener, we're just a regular client
          handleDataFromClient( i );
        }
      }
    }
  }
}

int Server::getListenerSocket() {
  int listener;  // Listening socket descriptor
  int yes = 1;   // For setsockopt() SO_REUSEADDR, below
  int rv;

  struct addrinfo hints, *ai, *p;

  // Get us a socket and bind it
  memset( &hints, 0, sizeof hints );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if( ( rv = getaddrinfo( NULL, PORT, &hints, &ai ) ) != 0 ) {
    std::cerr << "selectserver: " << gai_strerror( rv ) << "\n";
    exit( 1 );
  }

  for( p = ai; p != NULL; p = p->ai_next ) {
    listener = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
    if( listener < 0 ) {
      continue;
    }

    // Lose the pesky "address already in use" error message
    setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );

    if( bind( listener, p->ai_addr, p->ai_addrlen ) < 0 ) {
      close( listener );
      continue;
    }

    break;
  }

  // If we got here, it means we didn't get bound
  if( p == NULL ) {
    return -1;
  }

  freeaddrinfo( ai );  // All done with this

  // Listen
  if( listen( listener, 10 ) == -1 ) {
    return -1;
  }

  return listener;
}

void Server::setup() {
  // Get a listening socket
  listener = getListenerSocket();
  if( listener == -1 ) {
    std::cout << "error getting listening socket\n";
    /* fprintf( stderr, "error getting listening socket\n" ); */
    exit( 1 );
  }

  // Allocate initial space for file descriptors
  fdCount = 1;  // For the listener
  fdSize = 5;
  pfds = (struct pollfd*)malloc( sizeof( struct pollfd ) * fdSize );
  pfds[0].fd = listener;
  pfds[0].events = POLLIN;  // Report ready to read on incoming connection
}

int main( void ) {
  Server server;
  server.run();
  return 0;
}
