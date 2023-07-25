

#       FT_IRC

- exit to throw + ask chatgpt if runtime_error is the most appropriate

- clean code: canonical classes / long lines / take vars upstairs /
  split functions
- clean code: constness / init server func

- SIG handlin
- stress test
- clear README

      Process:                    Kernel:               Filesystem: ( File
      descriptor table )=> ( Open file table )=> ( I-node table )

- a fork sys call results in descriptors being shared by the parent and the
  child with share by reference semantics.  both the parent and the child are
  using the same descriptorshare by reference semantics.  both the parent and
  the child are using the same descriptor and reference the same offset in the
  file entry.  the same semantics apply to dup/dup2 sys call.

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

- a large i/o operation on a single descriptor has the potential to starve other
  descriptors thus even with the cas of *level triggered* notifications a large
  enough write or send call has the *potential to block*.

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

        int epfd = epoll_create( 0xCAFE );
        // 1.
        // if ( epfd < 0 ) report error
        ...  ...

        struct epoll_event ev = { 0 };

        // When a client connect we create an epoll_event object and add it to
        // the context

        for ( i = 0; i < 5; i++ ) { memset( &client, 0, sizeof (client) );
        addrlen = sizeof( client );

            // Initialize the 'struct epoll ev' with the wanted event and the
            // context data pointer

            // Associate the Connection class instance with the event:

            ev.data.fd = accept( sockfd,(struct sockaddr*)&client, &addrlen );

            // XXX TRY ev.data.ptr = _clients[i]; you can attach meaningful
            // context to the monitored event instead of socket file
            // descriptors. In our example we attached the class pointers which
            // could be called directly, saving you another lookup.

            ev.events = EPOLLIN; // TODO EPOLLET | EPOLLIN | EPOLLONESHOT

            // Add into the epoll set / interest list

            epoll_ctl( epfd, EPOLL_CTL_ADD, ev.data.fd, &ev );
            // 2.

            // XXX TRY epoll_ctl( epfd, EPOLL_CTL_ADD, ev.data.ptr->get(), &ev
            // );
            // https://www.ulduzsoft.com/2014/01/select-poll-epoll-practical-difference-for-system-architects/
        }

        struct epoll_event evlist[5];

        while(1){ puts( "round again" );

            // Fills the empty 'struct epoll evlist' with the triggered events

            nfds = epoll_wait( epfd, evlist, 5, 10000 );
            // 3.

            // Iterate through the triggered events

            for( i = 0; i < nfds; i++ ) { memset( buffer, 0, MAXBUF ); read(
            evlist[i].data.fd, buffer, MAXBUF ); puts( buffer ); }
            // XXX TRY so i receive my event with the pointer on the
            // corresponding client => one lookup economy
        }

Resources:
https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642

        // Create the epoll descriptor. Only one is needed per app, and is used
        // to monitor all sockets.  The function argument is ignored (it was not
        // before, but now it is), so put your favorite number here
        int pollingfd = epoll_create( 0xCAFE );

        if ( pollingfd < 0 )
        // report error

        // Initialize the epoll structure in case more members are added in
        // future
        struct epoll_event ev = { 0 };

        // Associate the connection class instance with the event. You can
        // associate anything you want, epoll does not use this information. We
        // store a connection class pointer, pConnection1
        ev.data.ptr = pConnection1;

        // Monitor for input, and do not automatically rearm the descriptor
        // after the event
        ev.events = EPOLLIN | EPOLLONESHOT;

        // Add the descriptor into the monitoring list. We can do it even if
        // another thread is waiting in epoll_wait - the descriptor will be
        // properly added
        if ( epoll_ctl( epollfd, EPOLL_CTL_ADD, pConnection1->getSocket(), &ev )
        != 0 )
            // report error

        // Wait for up to 20 events (assuming we have added maybe 200 sockets
        // before that it may happen)
        struct epoll_event pevents[ 20 ];

        // Wait for 10 seconds
        int ready = epoll_wait( pollingfd, pevents, 20, 10000 );

        // Check if epoll actually succeed
        if ( ret == -1 )
            // report error and abort
        else if ( ret == 0 )
            // timeout; no event detected
        else {
            // Check if any events detected
            for ( int i = 0; i < ret; i++ ) { if ( pevents[i].events & EPOLLIN )
            {
                    // Get back our connection pointer
                    Connection * c = (Connection*) pevents[i].data.ptr;
                    c->handleReadEvent(); } } }
