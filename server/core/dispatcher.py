from protocol.codes import RequestCode, ResponseCode
from core.handlers.register import RegisterHandler
from core.handlers.list_clients import ListClientsHandler
from core.handlers.get_pubkey import GetPubKeyHandler
from core.handlers.send_message import SendMessageHandler
from core.handlers.pull_messages import PullMessagesHandler
from protocol.framing import ResponseFrame
from debugger import print_request_debug, print_response_debug

class Dispatcher:
    def __init__(self, db):
        self.db = db
        self.handlers = {
            RequestCode.REGISTER: RegisterHandler(db),
            RequestCode.LIST_CLIENTS: ListClientsHandler(db),
            RequestCode.GET_PUBKEY: GetPubKeyHandler(db),
            RequestCode.SEND_MESSAGE: SendMessageHandler(db),
            RequestCode.PULL_MESSAGES: PullMessagesHandler(db),
        }

    def dispatch(self, request):

        print_request_debug(request)

        handler = self.handlers.get(request.code)
        if not handler: 
            resp = ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=b"Unknown request code")
            print_response_debug(resp)
        try: 
            resp = handler.handle(request)
            print_response_debug(resp)
            return resp
        except Exception as e:
            resp = ResponseFrame(version=request.version, code=ResponseCode.ERROR, payload=str(e).encode())
            print_response_debug(resp)
            return resp
