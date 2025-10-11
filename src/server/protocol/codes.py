from enum import IntEnum

class RequestCode(IntEnum):
    REGISTER = 600 # New client wants to register
    LIST_CLIENTS = 601 # Client wants to see other clients
    GET_PUBKEY = 602 # A client wants to get another clients public key from db
    SEND_MESSAGE = 603 # A client wants to send a new message
    PULL_MESSAGES = 604 # A client wants to see his pending messages 

class ResponseCode(IntEnum):
    REGISTER_OK = 2100 # Registered the client 
    LIST_CLIENTS = 2101 # Sent the clients list successfully
    GET_PUBKEY = 2102 # Sent the pubkey successfully
    SEND_OK = 2103 # Server received the message and saved it
    PULL_MESSAGES = 2104 # Pulled and sent awaiting messages 
    ERROR = 9000 # General error

class MessageType(IntEnum):
    SYM_REQ = 1 # Symmetric key request
    SYM_KEY = 2 # Symmetric key for client - encrypted with public key
    TEXT = 3 # A text message (encrypted peer-to-peer)
    FILE = 4 # A file message (bonus), also encrypted peer-to-peer


class Protocol(IntEnum):
    CLIENT_ID_LEN = 16
    TYPE_LEN = 1
    MAX_MESSAGE_CONTENT_SIZE = 4 # max 4 bytes for message content size
    
    MAX_USERNAME_LEN = 255
    MAX_PUBKEY_LEN = 160

    VERSION_LEN = 1
    CODE_LEN = 2
    PAYLOAD_SIZE_LEN = 4

    HEADER_LEN = CLIENT_ID_LEN + VERSION_LEN + CODE_LEN + PAYLOAD_SIZE_LEN