from protocol.framing import RequestFrame
from core.db import Database

class RequestHandler:
    """ 
    The base RequestHandler is just an abstract class for real handlers to inherit from. 
    Every RequestHandler must have a handle() function that receives the RequestFrame and acts according to the request type
    eventually returning a ResponseFrame, with an error or a success code (and a payload)
    """
    def __init__(self, db: Database):
        self.db = db

    def handle(self, request: RequestFrame):
        raise NotImplementedError()