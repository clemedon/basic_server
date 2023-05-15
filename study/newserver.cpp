#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

// int getaddrinfo(
//  const char* node,             // "www.example.com" or IP
//  const char* service,          // "http", "ftp" or port number "80"
//  const struct addrinfo* hints, // ai struct filled out with relevant info
//  struct addrinfo** res         // pointer to the results
// );

int              status;
struct addrinfo  hints;
struct addrinfo* servinfo;  // will point to the results

memset( &hints, 0, sizeof hints );  // make sure the struct is empty
hints.ai_family = AF_UNSPEC;        // don't care AF_INET or AF_INET6
hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
hints.ai_flags = AI_PASSIVE;        // fill in my IP for me

// AI_PASSIVE to assign the address of my local host to the socket structures,
// no need to hardcode it.

if( ( status = getaddrinfo( NULL, "3490", &hints, &servinfo ) ) != 0 ) {
  fprintf( stderr, "getaddrinfo error: %s\n", gai_strerror( status ) );
  exit( 1 );
}

// servinfo now points to a linked list of 1 or more struct addrinfos each of
// which contains a struct sockaddr:
//
//  struct addrinfo {
//     int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
//     int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
//     int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
//     int              ai_protocol;  // use 0 for "any"
//     size_t           ai_addrlen;   // size of ai_addr in bytes
//     struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
//     char            *ai_canonname; // full canonical hostname
//
//     struct addrinfo *ai_next;      // linked list, next node
// };
//
// struct sockaddr {
//     unsigned short    sa_family;    // address family, AF_xxx
//     char              sa_data[14];  // 14 bytes of protocol address
// };
//
// And the custom sockaddr for IPV4 with "Internet Address"
//
// struct sockaddr_in {
//    short int          sin_family;  // Address family, AF_INET
//    unsigned short int sin_port;    // Port number
//    struct in_addr     sin_addr;    // Internet address
//    unsigned char      sin_zero[8]; // Same size as struct sockaddr
// };



// walk the "res" linked list looking for valid entries instead of just assuming
// the first one is good (like many of these examples do).  See the section on
// client/server for real examples. TODO


// ...do everything until you don't need servinfo anymore....

freeaddrinfo( servinfo );  // free the linked-list
