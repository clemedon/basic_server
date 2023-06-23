

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

