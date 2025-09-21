import struct
from core.handlers.base import AbstractRequestHandler
from protocol.codes import ResponseCode
from protocol.framing import ResponseFrame

class PullMessagesHandler(AbstractRequestHandler):
    def handle(self, request):
        client_id = request.client_id
        messages = self.storage.pull_messages(client_id)

        payload_parts = []
        for message in messages:
            # FromClient (16) + Type (1) + ContentSize (4) + Content
            hdr = message.from_id + bytes([message.type]) + struct.pack("<I", len(message.content))
            payload_parts.append(hdr + message.content)

        payload = b"".join(payload_parts)
        return ResponseFrame(request.version, ResponseCode.PULL_MESSAGES, payload)
