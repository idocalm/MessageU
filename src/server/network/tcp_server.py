import socket
import threading
from network.connection import Connection

class TCPServer:
    def __init__(self, port, dispatcher):
        self.port = port
        self.dispatcher = dispatcher

    def start(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
        s.bind(("0.0.0.0", self.port))
        s.listen(5)
        
        while True:
            conn, addr = s.accept()
            print(f"Accepted connection from {addr}")
            conn = Connection(conn, self.dispatcher)
            t = threading.Thread(target=conn.handle)
            t.daemon = True
            t.start()
