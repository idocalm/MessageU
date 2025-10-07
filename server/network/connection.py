from protocol.framing import RequestFrame, ResponseFrame
from core.dispatcher import Dispatcher

class Connection:
    def __init__(self, sock, dispatcher: Dispatcher):
        self.sock = sock
        self.dispatcher = dispatcher

    def handle(self):
        try: 
            while True:
                data = self.sock.recv(4096)
                if not data:
                    break

                req = RequestFrame.from_bytes(data)
                resp = self.dispatcher.dispatch(req)
                self.sock.sendall(resp.to_bytes())
            
        except Exception as e:
            print(f"Connection error: {e}")
        finally:
            self.sock.close()