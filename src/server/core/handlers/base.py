from protocol.framing import RequestFrame
from core.db import Database

class RequestHandler:
    def __init__(self, db: Database):
        self.db = db

    def handle(self, request: RequestFrame):
        raise NotImplementedError()