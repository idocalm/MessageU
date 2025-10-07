from core.handlers.base import RequestHandler
from protocol.codes import ResponseCode, Protocol
from protocol.framing import ResponseFrame

class GetPubKeyHandler(RequestHandler):
    def handle(self, request):
        payload = request.payload
        if len(payload) != Protocol.CLIENT_ID_LEN:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Invalid payload size")
        
        client_id = payload
        pubkey = self.db.get_pubkey(client_id)

        if not pubkey:
            return ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Client not found")
        
        response = client_id + pubkey
        return ResponseFrame(version=request.version, code=ResponseCode.GET_PUBKEY, payload=response)