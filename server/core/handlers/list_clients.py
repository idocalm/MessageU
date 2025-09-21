from core.handlers.base import AbstractRequestHandler
from protocol.codes import ResponseCode
from protocol.framing import ResponseFrame

class ListClientsHandler(AbstractRequestHandler):
    def handle(self, request):
        clients = self.db.get_all_clients()
        print(clients)

        payload_parts = []
        for client in clients: 
            if client.id == request.client_id:
                # Skip the requesting client in the list
                continue

            # TODO: Understand this ->
            name_bytes = client.username.encode('ascii', errors='ignore')
            if len(name_bytes) > 254:
                name_bytes = name_bytes[:254]
            name_field = name_bytes + b'\x00' * (255 - len(name_bytes))
            payload_parts.append(client.id + name_field)

        payload = b''.join(payload_parts)
        return ResponseFrame(version=request.version, code=ResponseCode.LIST_CLIENTS, payload=payload)
