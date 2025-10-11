from protocol.framing import RequestFrame, ResponseFrame
from core.dispatcher import Dispatcher
from protocol.codes import Protocol
class Connection:
    def __init__(self, sock, dispatcher: Dispatcher):
        self.sock = sock
        self.dispatcher = dispatcher

    def handle(self):
        try: 
            while True:
                header = self.sock.recv(Protocol.HEADER_LEN)
                if not header:
                    break
                
                header_size = struct.calcsize(RequestFrame.HEADER_FORMAT)
                if len(header) < header_size:
                    raise ValueError("Data too short to contain a valid request frame")
                
                client_id, version, code, payload_size = struct.unpack(RequestFrame.HEADER_FORMAT, header[:header_size])
                if not isinstance(payload_size, int) or payload_size < 0:
                    raise ValueError("Payload size is negative")
                    break

                payload = self.sock.recv(payload_size)
                full = header + payload

                req = RequestFrame.from_bytes(full)
                resp = self.dispatcher.dispatch(req)
                self.sock.sendall(resp.to_bytes())
            
        except Exception as e:
            print(f"Connection error: {e}")
        finally:
            self.sock.close()