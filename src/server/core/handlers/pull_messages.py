import struct
from core.handlers.base import RequestHandler
from protocol.codes import ResponseCode
from protocol.framing import ResponseFrame

class PullMessagesHandler(RequestHandler):
    """
    A request handler designed to handle the request type: Pull all my clients unread messages
    """
    def handle(self, request):
        client_id = request.client_id
        messages = self.db.pull_messages(client_id)

        # Create the payload by concating together all the messages: header + content
        payload_parts = []
        for message in messages:
            
            # Each message header has: 
                # from id 
                # content type
                # content size
            # then the content

            message_header = message.from_id + bytes([message.type]) + struct.pack("<I", len(message.content))
            payload_parts.append(message_header + message.content)

        payload = b"".join(payload_parts)
        return ResponseFrame(request.version, ResponseCode.PULL_MESSAGES, payload)
