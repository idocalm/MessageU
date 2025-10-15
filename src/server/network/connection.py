import struct
from protocol.framing import RequestFrame
from core.dispatcher import Dispatcher
from protocol.codes import Protocol

class Connection:
    """ 
    The connection class lives inside any socket between a client and the server 
    it constantly handles the loop of receiving a packet -> decoding it -> moving it for dispatcher to take care ->
    receving a response from the dispatcher -> sending it 
    
    """
    def __init__(self, sock, dispatcher: Dispatcher):
        self.sock = sock
        self.dispatcher = dispatcher

    def handle(self):
        try: 
            while True:
                # start reading and parsing packets

                header = self.sock.recv(Protocol.HEADER_LEN)
                if not header:
                    break
                
                header_size = struct.calcsize(RequestFrame.HEADER_FORMAT)
                if len(header) < header_size:
                    raise ValueError("Data too short to contain a valid request frame")
                
                client_id, version, code, payload_size = struct.unpack(RequestFrame.HEADER_FORMAT, header[:header_size])
                if not isinstance(payload_size, int) or payload_size < 0:
                    raise ValueError("Payload size is negative")


                payload = self.sock.recv(payload_size)
                full = header + payload

                req = RequestFrame.from_bytes(full)
                # pass handling to dispatcher, and receive a ResponseFrame once the handling is complete
                resp = self.dispatcher.dispatch(req)
                self.sock.sendall(resp.to_bytes())
            
        except Exception as e:
            print(f"Connection error: {e}")
        finally:
            self.sock.close()