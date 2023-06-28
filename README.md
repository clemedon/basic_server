

#       FT_IRC

## RAII

Using RAII (Resource Acquisition Is Initialization) for `_serverSocket` in the
`Server` ensures that the server socket is properly created and closed,
regardless of how the execution flow exits the class or function.

RAII is a programming technique where resources, such as file handles or network
sockets, are tied to the lifespan of an object. The resource acquisition and
release are managed automatically through constructors and destructors of the
object, ensuring that the resource is always properly released when the object
goes out of scope.

The `ServerSocket` class encapsulates the server socket creation, and the
constructor initializes the socket, while the destructor closes it. By using the
`ServerSocket` object as a member variable in the `Server` class, the server
socket's lifecycle is automatically managed.

The socket is created when the `Server` object is constructed and closed when it
is destructed. This guarantees that the server socket is properly closed even if
an exception is thrown during the execution.

On the other hand, `_clientSockets` in the `Server` class is a vector of client
socket file descriptors. Managing client sockets does not require RAII in the
same way as the server socket. The reason is that closing client sockets can be
explicitly handled when the client disconnects, which is already implemented in
the `removeDisconnectedClients` function. Client sockets are managed by
explicitly closing them using the `close` function. Since the vector stores the
file descriptors and not the socket objects themselves, there is no need for an
additional level of RAII abstraction for the client sockets.

In summary, using RAII for `_serverSocket` ensures proper creation and
destruction of the server socket, whereas explicit management of client sockets
is sufficient in the given code.

## SELECT / POLL / EPOLL

    Epoll*
    1. epoll_create( int size )   creates a context in the kernel
        * the parameter is ignored but has to be positive
    2. epoll_ctl()      add and remove file descriptors to/from context
    3. epoll_wait()     wait for events in the context



        // Create epoll descriptor

        int epfd = epoll_create( 0xCAFE );                                      // 1.
        // if ( epfd < 0 ) report error
        ...
        ...

        struct epoll_event ev = { 0 };

        // When a client connect we create an epoll_event object and add it to
        // the context

        for ( i = 0; i < 5; i++ )
        {
            memset( &client, 0, sizeof (client) );
            addrlen = sizeof( client );

            // Initialize the 'struct epoll ev' with the wanted event and
            // the context data pointer

            // Associate the Connection class instance with the event:

            ev.data.fd = accept( sockfd,(struct sockaddr*)&client, &addrlen );

            // XXX TRY ev.data.ptr = _clients[i];
            // you can attach meaningful context to the monitored event instead
            // of socket file descriptors. In our example we attached the class
            // pointers which could be called directly, saving you another
            // lookup.

            ev.events = EPOLLIN; // TODO EPOLLIN | EPOLLONESHOT

            // Add into the monitoring list

            epoll_ctl( epfd, EPOLL_CTL_ADD, ev.data.fd, &ev );    // 2.

            // XXX TRY epoll_ctl( epfd, EPOLL_CTL_ADD, ev.data.ptr->get(), &ev );
            // https://www.ulduzsoft.com/2014/01/select-poll-epoll-practical-difference-for-system-architects/
        }

        struct epoll_event ready[5];

        while(1){
            puts( "round again" );

            // Fills the empty 'struct epoll ready' with the triggered events

            nfds = epoll_wait( epfd, ready, 5, 10000 );                         // 3.

            // Iterate through the triggered events

            for( i = 0; i < nfds; i++ ) {
                    memset( buffer, 0, MAXBUF );
                    read( ready[i].data.fd, buffer, MAXBUF );
                    puts( buffer );
            }
            // XXX TRY so i receive my event with the pointer on the
            // corresponding client => one lookup economy
        }

        Resources:
        https://devarea.com/linux-io-multiplexing-select-vs-poll-vs-epoll/
        https://www.ulduzsoft.com/2014/01/select-poll-epoll-practical-difference-for-system-architects/

