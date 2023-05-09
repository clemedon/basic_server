

#       FT_IRC

## Description

An IRC server in C++98

Externam functions:
 socket: create an end point for communication
 close: close fd
 setsockopt: get / set socket options
 getsockname: get socket name
 getprotobyname: get a protocol entry by name
 gethostbyname: get IP address for a specified host name
 getaddrinfo: converts hostnames or IP strings into a LL of addrinfo structs
 freeaddrinfo: free one or more addrinfo structs returned by getaddrinfo
 bind: bind a name to a socket
 connect: initiate a connection on a socket
 listen: listen for connections on a socket
 accept: accept a connection on a socket
 *htons*: conv u_short int hostshort from host byte order to network byte order
 htonl: conv u_int hostlong from host byte order to network byte order
 *ntohs*: conv u_short int netshort from network byte order to host byte order
 ntohl: conv u_int netlong from network byte order to host byte order
 inet_addr: conv 'cp' addr from ipv4 notation to binary data in nework byte order
 inet_ntoa: conv given addr from network byte order to ipv4 string notation
 send: send a message on a socket
 recv: receive a message from a socket
 signal: ANSI C signal handling
 lseek: reposition read/write file offset
 fstat: get file status
 fcntl: manip fd
 poll: wait for some event on a fd

## Resources

- subject: https://cdn.intra.42.fr/pdf/pdf/81719/en.subject.pdf
- good intro https://github.com/Ccommiss/ft_irc/blob/main/README.md#t-i-introduction-t
- technical https://www.ibm.com/docs/en/i/7.3?topic=designs-example-nonblocking-io-select
- step by step https://www.bogotobogo.com/cplusplus/sockets_server_client.php


/list
/join
/msg
/part (quitter)
/query <username> <msg> (private msg)

server test
    telnet localhost <port_num>
