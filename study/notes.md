

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
    "it used to be that you would use a function called gethostbyname() to do
    DNS lookups. Then you’d load that information by hand into a struct
    sockaddr_in, and use that in your calls."
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
    inet_pton (a new version that work with IPv6)
 inet_ntoa: conv given addr from network byte order to ipv4 string notation
    inet_ntop (a new version that work with IPv6)
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

## Beej's Guide to Network Programming"

https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch

###  Chapter 2 - What is a socket?

- A socket is a way to speak to other programs using standard unix file
  descriptors.

- There are all kinds of sockets but we will focus on the DARPA Internet
  addresses (*Internet Sockets*)

- There are many types of Internet Socket but we will focus on the *Stream
  Sockets SOCK_STREAM* and the *Datagram Sockets SOCK_DGRAM*.  PS. dig about
  powerful Raw Sockets TODO.


- Stream Sockets are *reliable* two-way connected communication streams (socket's
  item will arrive in the same order as they were sent) (i.e. telnet, ssh, http)

- Stream Sockets use TCP = Transmission Control Protocol to achieve this data
  transmission quality
- In TCP/IP, the IP part deals primarily with Internet routing and is not
  generally responsible for data integrity

- Datagram also called connectionless sockets
- Datagram are used when it is not a big deal to lost packets on the way
  (i.e. video games, audio, video)
- UDP/IP use IP for routing but use User Datagram Protocol UDP instead of TCP
- Datagram are *unreliable* but way *faster*

- Data Encapsulation

    [ Ethernet [ IP [ UDP [ TFTP [ Data ] ] ] ] ]

    - the packet is born, wrapped in a header by the first protocol (here tftp)
      then the whole thing is wrapped by the next protocol (here UDP) then again
      by the next (IP), then again by the final protocol on the hardware layer
      (Ethernet)

    - the packet is received, the hardware strips the Ethernet header, the
      kernel strips the IP and UDP headers, the TFTP program strips the TFTP
      header and finally has the data.

- Layered model

    Application Layer (telnet, ftp, etc.)
    Host-to-Host Transport Layer (TCP, UDP)
    Internet Layer (IP and routing)
    Network Access Layer (Ethernet, wi-fi, or whatever)

###  Chapter 3 - IP Addresses, structs, and Data Munging

- Think of the IP address as the street address of a hotel, and the port number
  as the room number

- Host Byte Order can be Little-Endian or Big-Endian
- b34f on a Big-Endian      machine would be stored: b3 first and then 4f
- b34f on a Little-Endian   machine would be stored: 4f first and then b3
- Big-Endian is also called Network Byte Order

- There are two type of numbers we can convert: short (two bytes) and long (four
  bytes)

- If our *Host* Byte Order is Little-Endian we want to convert it *to Network*
  Byte Order so we will use: htons() h-ost to n-etwork s-short

    htons host to network short
    htonl host to network long
    ntohs network to host short
    ntohl network to host long

- struct addrinfo

    struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // 0 for UDP or TCP based on ai_socktype
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname
    struct addrinfo *ai_next;      // linked list, next node
    };

        struct sockaddr_in {
           short int          sin_family;  // Address family, AF_INET
           unsigned short int sin_port;    // Port number
           struct in_addr     sin_addr;    // Internet address
           unsigned char      sin_zero[8]; // Same size as struct sockaddr
        };


    inet_pton()

> pton stands for p-resentation to n-etwork = printable to network


- firewall hides network from the rest of the world, firewall translates
  "internal" IP addr to "external" IP addr using a process called NAT = Network
  Address Translation

  So my private IP is different that my public one because my firewall is doing
  NAT.

###  Chapter 4 Jumping from IPv4 to IPv6

###  Chapter 5 System Calls or Bust

1. manually fill hints with hints about the type of socket we want

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);  // clear up hints memory
    hints.ai_family   = AF_UNSPEC;    // don't care AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
    hints.ai_flags    = AI_PASSIVE;   // fill in my IP for me

2. getaddrinfo() auto fills out the rest of struct addrinfo

    int getaddrinfo(const char *hostname,   // e.g. "www.example.com" or IP
                    const char *service,    // e.g. "http" or port number
                    const struct addrinfo *hints,
                    struct addrinfo **res);

    get a list of IP addresses and port numbers for host hostname and service
    servname

    it used to be that you use gethostbyname() to do DNS lookups
    then load that info by hand into a struct sockaddr_in
    and use that in your calls - now automated by getaddrinfo()

> if i understand well, we pre-fill 'hint' and getaddrinfo() use this 'hint' to
> full-fill 'res'?

3. socket() returns a 'socket descriptor' (or -1 and set errno on error).  Filled with
   getaddrinfo call res.

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

4. bind() associate the socket with a port on our local machine (or return -1
   and set errno on error)

    bind(sockfd, res->ai_addr, res->ai_addrlen);

> the port is stored in res->ai_addr->sin_port

> All ports below 1024 are RESERVED, make sure to choose one between 1024 and
> 65535.

> not useful if we only connect() in which case the kernel will choose a local
> port for us and the site we connect to will automatically get this info.

5. CLIENT CASE: connect() to a remote host (or return and set errno on error)

    connect(sockfd, res->ai_addr, res->ai_addrlen);

6. listen() to a remote host (or return -1 on error)

    listen(sockfd, BACKLOG);

    BACKLOG the number of connections allowed in the incoming queue until
    accept() call

> We need to call bind() before so that the server is running on a specific port
> that we can share with the clients for them to connect()

7. accept() an incoming connection (or return -1 on error)

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    sockfd: the listening fd
    their_addr: store info about incoming connection (host and port)

> Someone will try to connect() on a port you are listen()ing on.  Their
> connection will be queued up until they are accept()ed.  You call accept() and
> you tell it to get a pending connection.  It will return to you a new socket
> fd to use for this single connection.  Now you have two socket fd, one that is
> listening for more new connections and the newly created one is available to
> send()/recv() to/from the client.

> If only one connection is excpected we can close() the listening sockfd.

8. send() and recv() to communicate over stream sockets or connected datagram
   Return the number of bytes actually sent/received out.

    char *msg = "Beej was here!";
    len = strlen(msg);
    bytes_sent = send(sockfd, msg, len, 0);

    sockfd: the fd we want to send data to
    0: no flags

> with a 0 flags argument, send() is equivalent to write()


    sendto(sockfd, buf, len, flags, NULL, 0); is equivalent send()


###  Chapter 6

###  Chapter 7

###  Chapter 8

###  Chapter 9

###  Chapter 10
