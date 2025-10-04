from enum import IntEnum

class RequestCode(IntEnum):
    REGISTER = 600
    LIST_CLIENTS = 601
    GET_PUBKEY = 602
    SEND_MESSAGE = 603
    PULL_MESSAGES = 604

class ResponseCode(IntEnum):
    REGISTER_OK = 2100
    LIST_CLIENTS = 2101
    GET_PUBKEY = 2102
    SEND_OK = 2103 # Server received the message
    PULL_MESSAGES = 2104
    ERROR = 9000 # General error

class MessageType(IntEnum):
    SYM_REQ = 1 
    SYM_KEY = 2
    TEXT = 3
    FILE = 4