from core.handlers.base import RequestHandler
from protocol.codes import ResponseCode, Protocol
from protocol.framing import ResponseFrame
import sqlite3

class RegisterHandler(RequestHandler):
    def handle(self, request):
        payload = request.payload

        # Payload contains 
        if len(payload) < Protocol.MAX_USERNAME_LEN + Protocol.MAX_PUBKEY_LEN:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Invalid payload size")
        
        raw_name = payload[:Protocol.MAX_USERNAME_LEN]
        username = raw_name.split(b'\x00', 1)[0].decode('ascii', errors='ignore')
        
        pubkey = payload[Protocol.MAX_USERNAME_LEN:Protocol.MAX_USERNAME_LEN+Protocol.MAX_PUBKEY_LEN]

        try:
            existing = self.db.get_client_by_name(username)
            if existing:
                return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Username already exists")
            
            client_id = self.db.add_client(username, pubkey)
            return ResponseFrame(version=request.version, code=ResponseCode.REGISTER_OK, payload=client_id)
        
        except sqlite3.IntegrityError:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Username already exists")
        except Exception as e:
            print(f"Error in RegisterHandler: {e}")
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=str(e).encode())