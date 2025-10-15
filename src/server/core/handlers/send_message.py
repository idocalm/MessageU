import struct
from core.handlers.base import RequestHandler
from protocol.codes import ResponseCode, MessageType, Protocol
from protocol.framing import ResponseFrame

SIZE = Protocol.CLIENT_ID_LEN + Protocol.TYPE_LEN + Protocol.MAX_MESSAGE_CONTENT_SIZE

class SendMessageHandler(RequestHandler):
    """
    A request handler designed to handle the request type: Send a new message
    """
    def handle(self, request):
        p = request.payload
        if len(p) < SIZE:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"Invalid payload")

        # unpack the params of the message from the payload
        to_id = p[:Protocol.CLIENT_ID_LEN]
        msg_type = p[Protocol.CLIENT_ID_LEN]
        (content_size,) = struct.unpack_from("<I", p, Protocol.CLIENT_ID_LEN + 1)

        # size checks
        if len(p) != SIZE + content_size:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"Size mismatch")

        if msg_type not in (MessageType.SYM_REQ, MessageType.SYM_KEY, MessageType.TEXT, MessageType.FILE):
            return ResponseFrame(request.version, ResponseCode.ERROR, b"Invalid message type")

        content = p[SIZE:SIZE + content_size]

        # check that the user exist
        if self.db.get_client_by_id(to_id) is None:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"Dest not found")

        from_id = request.client_id

        # avoid spam and don't let users send message to themself
        if from_id == to_id:
            return ResponseFrame(request.version, ResponseCode.ERROR, b"Can't send message to yourself")

        try:
            self.db.add_message(to_id, from_id, int(msg_type), content)
            # Update last seen for sender
            self.db.update_last_seen(from_id)
            return ResponseFrame(request.version, ResponseCode.SEND_OK, b"")
        except Exception as e:
            return ResponseFrame(request.version, ResponseCode.ERROR, str(e).encode())