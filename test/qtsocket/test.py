from PyQt5.QtNetwork import QTcpSocket

# Create a QTcpSocket instance
# s = QTcpSocket()

# Connect to the server at 127.0.0.1:3400
# s.connectToHost("127.0.0.1", 3400)
# if s.waitForConnected():
    # print("Connected to server!")

    # Write data to the server
    # s.write(b"Try This")
    # s.waitForBytesWritten()  # Wait until the data is written
    # print("Data sent successfully")

    # Disconnect from the server
    # s.disconnectFromHost()
    # s.waitForDisconnected()
    # print("Disconnected from server")
# else:
    # print("Failed to connect to the server.")

import socket
from time import sleep

# Use Python's socket library for debugging
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("127.0.0.1", 3400))
s.sendall(b"Try This")
sleep(1)  # Short delay to keep the connection open
s.close()