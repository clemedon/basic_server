import socket
import threading

def create_connection(host, port):
    try:
        # Create a new socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # Connect to the server
        sock.connect((host, port))
        # Send some data (optional)
        sock.send(b"Hello from client!")
        # Receive response from the server (optional)
        response = sock.recv(1024)
        print("Received response:", response.decode())
        # Close the connection
        sock.close()
    except Exception as e:
        print("Error:", str(e))

# Server details
server_host = "localhost"
server_port = 4242

# Number of connections to create
num_connections = 100

# Create multiple connections concurrently
threads = []
for _ in range(num_connections):
    t = threading.Thread(target=create_connection, args=(server_host, server_port))
    threads.append(t)
    t.start()

# Wait for all threads to complete
for t in threads:
    t.join()
