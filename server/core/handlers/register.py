from core.handlers.base import AbstractRequestHandler
from protocol.codes import ResponseCode
from protocol.framing import ResponseFrame
import sqlite3

class RegisterHandler(AbstractRequestHandler):
    def handle(self, request):
        payload = request.payload

        # Payload contains 
        if len(payload) < 255 + 160:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Invalid payload size")
        
        raw_name = payload[:255]
        username = raw_name.split(b'\x00', 1)[0].decode('ascii', errors='ignore')
        pubkey = payload[255:255+160]

        try:
            existing = self.db.get_client_by_name(username)
            if existing:
                return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Username already exists")
            
            client_id = self.db.add_client(username, pubkey)
            return ResponseFrame(version=request.version, code=ResponseCode.OK, payload=client_id)
        
        except sqlite3.IntegrityError:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Username already exists")
        except Exception as e:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=str(e).encode())