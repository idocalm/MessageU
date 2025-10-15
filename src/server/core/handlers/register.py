from core.handlers.base import RequestHandler
from protocol.codes import ResponseCode, Protocol
from protocol.framing import ResponseFrame
import sqlite3

class RegisterHandler(RequestHandler):
    """
    A request handler designed to handle the request type: Register
    """
    def handle(self, request):
        payload = request.payload

        # check that payload is in the correct size
        if len(payload) < Protocol.MAX_USERNAME_LEN + Protocol.MAX_PUBKEY_LEN:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Invalid payload size")
        
        raw_name = payload[:Protocol.MAX_USERNAME_LEN]
        username = raw_name.split(b'\x00', 1)[0].decode('ascii', errors='ignore')
        
        pubkey = payload[Protocol.MAX_USERNAME_LEN:Protocol.MAX_USERNAME_LEN+Protocol.MAX_PUBKEY_LEN]

        try:
            # check if a user exists
            existing = self.db.get_client_by_name(username)
            if existing:
                return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Username already exists")
            
            # no user with this username, create a client and return an id 
            client_id = self.db.add_client(username, pubkey)
            return ResponseFrame(version=request.version, code=ResponseCode.REGISTER_OK, payload=client_id)
        
        except sqlite3.IntegrityError:
            # failure to write to sql
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Username already exists")
        except Exception as e:
            # another sort of failure...
            print(f"Error in RegisterHandler: {e}")
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=str(e).encode())