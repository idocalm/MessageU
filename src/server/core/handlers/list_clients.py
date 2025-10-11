from core.handlers.base import RequestHandler
from protocol.codes import ResponseCode, Protocol
from protocol.framing import ResponseFrame

class ListClientsHandler(RequestHandler):
    def handle(self, request):
        clients = self.db.get_all_clients()
        print(clients)

        payload_parts = []
        for client in clients: 
            if client.id == request.client_id:
                # Skip the requesting client in the list
                continue

            # TODO: Maybe this is incorrect
            name_bytes = client.username.encode('ascii', errors='ignore')
            name_field = name_bytes + b'\x00' * (Protocol.MAX_USERNAME_LEN - len(name_bytes))
            payload_parts.append(client.id + name_field)

        payload = b''.join(payload_parts)
        return ResponseFrame(version=request.version, code=ResponseCode.LIST_CLIENTS, payload=payload)
