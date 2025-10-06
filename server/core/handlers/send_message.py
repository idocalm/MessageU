import struct
from core.handlers.base import AbstractRequestHandler
from protocol.codes import ResponseCode, MessageType
from protocol.framing import ResponseFrame

SIZE = 16 + 1 + 4  # ToClient(16) + Type(1) + ContentSize(4)

class SendMessageHandler(AbstractRequestHandler):
    def handle(self, request):
        p = request.payload
        if len(p) < SIZE:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"invalid payload")

        to_id = p[:16]
        msg_type = p[16]
        (content_size,) = struct.unpack_from("<I", p, 17)

        if len(p) != SIZE + content_size:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"size mismatch")

        if msg_type not in (MessageType.SYM_REQ, MessageType.SYM_KEY, MessageType.TEXT, MessageType.FILE):
            return ResponseFrame(request.version, ResponseCode.ERROR, b"invalid message type")

        content = p[SIZE:SIZE + content_size]

        if self.db.get_pubkey(to_id) is None:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"dest not found")

        from_id = request.client_id

        if from_id == to_id:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"can't send message to yourself")

        try:
            self.db.add_message(to_id, from_id, int(msg_type), content)
            # Update last seen for sender
            self.db.update_last_seen(from_id)
            return ResponseFrame(request.version, ResponseCode.SEND_OK, b"")
        except Exception as e:
            return ResponseFrame(request.version, ResponseCode.ERROR, str(e).encode())