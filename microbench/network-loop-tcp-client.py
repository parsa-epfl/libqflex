import socket

# Define the server's IP address and port
server_ip = "127.0.0.1"
server_port = 12344

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the server
client_socket.connect((server_ip, server_port))
i = 0
while True:

    # Send a message to the server
    message = f"{i}:Hello, Server!"
    client_socket.send(message.encode('utf-8'))

    # Receive the server's response
    response = client_socket.recv(1024).decode('utf-8')
    print(f"Received response from server: {response}")

    i += 1

# Close the socket
client_socket.close()
