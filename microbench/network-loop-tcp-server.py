import socket

# Define the server's IP address and port
server_ip = "127.0.0.1"
server_port = 12345

# Create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the address and por127.0.0.1t
server_socket.bind((server_ip, server_port))

server_socket.listen(5)
print(f"Server is listening on {server_ip}:{server_port}")

# Accept a connection from a client
client_socket, client_address = server_socket.accept()
print(f"Accepted connection from {client_address}")


# Listen for incoming connections
i = 0
while True:
    # Receive data from the client
    data = client_socket.recv(1024).decode('utf-8')
    print(f"Received data from client: {data}")

    # Send a response back to the client
    response = f"{i}:Hello from the server!"
    client_socket.send(response.encode('utf-8'))
    
    i += 1

# Close the client socket
client_socket.close()

# Close the server socket
server_socket.close()

