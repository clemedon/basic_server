

#       FT_IRC

- when a process is forked the child process inherits a duplicate set of the
  parent process's open file descriptors.  both the parent and child process
  will have file descriptors that refer to the same open file descriptions. any
  operations performed on the file descriptors in either process will affect the
  shared open file descriptions

- if you open a new file descriptor in the child process, referring to a file
  that is already opened in the parent process ( after forking ), the child
  process will have a separate file descriptor that refers to the same file but
  has a different file description. the file descriptors will be independent,
  and changes made to one of the file descriptor will not affect the other one.

- within the same process, it is not possible to open a new file descriptor that
  refers to the same file ( inode ) but has a different file description. the
  file descriptions are tied to the file descriptors at the time of opening, and
  it is not possible to change or duplicate the file description of an existing
  file descriptor within the same process.

- there is no direct way to know if two file descriptors refer to the same file
  description. however, you can compare file descriptors using the == operator
  to check if they refer to the same underlying file or socket. if two file
  descriptors compare as equal, it means they are referring to the same open
  file description. it is not possible to control or explicitly specify which
  file description a file descriptor should refer to. the file descriptions are
  managed internally by the operating system and are associated with file
  descriptors at the time of opening or duplicating them.

## SELECT / POLL / EPOLL

Allows for a process to monitor multiple file descriptors and get notifications
when I/O is possible on them.

    Epoll*
    1. epoll_create( int size )   creates a context in the kernel
        * the parameter is ignored but has to be positive
    2. epoll_ctl()      add and remove file descriptors to/from context
    3. epoll_wait()     wait for events in the context



        // Create epoll descriptor that create a connection with the kernel
        // epoll instance

        int epfd = epoll_create( 0xCAFE );                                       // 1.
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

            ev.events = EPOLLIN; // TODO EPOLLET | EPOLLIN | EPOLLONESHOT

            // Add into the monitoring list

            epoll_ctl( epfd, EPOLL_CTL_ADD, ev.data.fd, &ev );                  // 2.

            // XXX TRY epoll_ctl( epfd, EPOLL_CTL_ADD, ev.data.ptr->get(), &ev );
            // https://www.ulduzsoft.com/2014/01/select-poll-epoll-practical-difference-for-system-architects/
        }

        struct epoll_event evlist[5];

        while(1){
            puts( "round again" );

            // Fills the empty 'struct epoll evlist' with the triggered events

            nfds = epoll_wait( epfd, evlist, 5, 10000 );                        // 3.

            // Iterate through the triggered events

            for( i = 0; i < nfds; i++ ) {
                    memset( buffer, 0, MAXBUF );
                    read( evlist[i].data.fd, buffer, MAXBUF );
                    puts( buffer );
            }
            // XXX TRY so i receive my event with the pointer on the
            // corresponding client => one lookup economy
        }

        Resources:
        https://devarea.com/linux-io-multiplexing-select-vs-poll-vs-epoll/
        https://www.ulduzsoft.com/2014/01/select-poll-epoll-practical-difference-for-system-architects/
        https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642

