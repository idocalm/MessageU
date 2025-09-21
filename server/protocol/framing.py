import struct

class RequestFrame:
    '''
    A request frame is constructed of:
    - 16 bytes of client ID
    - 1 byte for version
    - 2 bytes for request code
    - 4 bytes for payload size
    '''
    HEADER_FORMAT = "<16sBHI"

    def __init__(self, client_id, version, code, payload):
        self.client_id = client_id
        self.version = version
        self.code = code
        self.payload = payload

    @classmethod
    def from_bytes(cls, data):
        header_size = struct.calcsize(cls.HEADER_FORMAT)
        client_id, version, code, payload_size = struct.unpack(cls.HEADER_FORMAT, data[:header_size])
        payload = data[header_size:header_size + payload_size]
        return cls(client_id, version, code, payload)
    

class ResponseFrame:
    '''
    A response frame is constructed of:
    - 1 byte for version
    - 2 bytes for response code
    - 4 bytes for payload size
    '''

    HEADER_FORMAT = "<BHI"

    def __init__(self, version, code, payload=b""):
        self.version = version
        self.code = code
        self.payload = payload
    
    def to_bytes(self):
        header = struct.pack(self.HEADER_FORMAT, self.version, self.code, len(self.payload))
        return header + self.payload

    