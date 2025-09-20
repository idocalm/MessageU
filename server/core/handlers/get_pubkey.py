from core.handlers.base import AbstractRequestHandler
from protocol.codes import ResponseCode
from protocol.framing import ResponseFrame

class GetPubKeyHandler(AbstractRequestHandler):
    def handle(self, request):
        payload = request.payload
        if len(payload) != 16:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Invalid payload size")
        
        client_id = payload # 16 bytes client id
        pubkey = self.db.get_pubkey(client_id)

        if not pubkey:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Client not found")
        
        response = client_id + pubkey
        return ResponseFrame(version=request.version, code=ResponseCode.GET_PUBKEY, payload=response)
    
