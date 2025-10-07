'''
This python file is not a standard part of the project, and it was used through the development by me so I could 
debug and see what the server receives from clients, and what is sends in response 

You can disable or enable debug mode by changing the config.py :
 class Config: 
    def __init__(self):
        self.port = FALLBACK_PORT
        self.version = 2
        self.debug = DEBUG

just set DEBUG = False in the start of the file and then the debugs will be removed
'''


from protocol.codes import Protocol, RequestCode, ResponseCode
import binascii
import struct

def print_request_debug(request):
    print("\n================= [DEBUG REQUEST] =================")
    print(f"Client ID : {binascii.hexlify(request.client_id).decode()}")
    print(f"Version   : {request.version}")
    try:
        code_name = RequestCode(request.code).name
    except ValueError:
        code_name = "UNKNOWN"
    print(f"Code      : {request.code} ({code_name})")
    print(f"P Size    : {len(request.payload)} bytes")

    if request.code == RequestCode.REGISTER:
        raw_name = request.payload[:Protocol.MAX_USERNAME_LEN]
        username = raw_name.split(b"\x00", 1)[0].decode("ascii", errors="ignore")
        pubkey = request.payload[Protocol.MAX_USERNAME_LEN:Protocol.MAX_USERNAME_LEN + Protocol.MAX_PUBKEY_LEN]
        print("---- REGISTER ----")
        print(f"Username  : {username}")
        print(f"PubKey[0:{Protocol.CLIENT_ID_LEN}]: {binascii.hexlify(pubkey[:Protocol.CLIENT_ID_LEN]).decode()}...")

    elif request.code == RequestCode.LIST_CLIENTS:
        print("---- LIST CLIENTS ---- (no payload)")

    elif request.code == RequestCode.GET_PUBKEY:
        target_id = request.payload[:Protocol.CLIENT_ID_LEN]
        print("---- GET PUBKEY ----")
        print(f"Target ID : {binascii.hexlify(target_id).decode()}")

    elif request.code == RequestCode.SEND_MESSAGE:
        to_id = request.payload[:Protocol.CLIENT_ID_LEN]
        msg_type = request.payload[Protocol.CLIENT_ID_LEN]
        content_size = struct.unpack_from("<I", request.payload, Protocol.CLIENT_ID_LEN + 1)[0]
        content_start_index = Protocol.CLIENT_ID_LEN + Protocol.TYPE_LEN + Protocol.MAX_MESSAGE_CONTENT_SIZE
        content = request.payload[content_start_index:content_start_index + content_size]
        print("---- SEND MESSAGE ----")
        print(f"To ID     : {binascii.hexlify(to_id).decode()}")
        print(f"Type      : {msg_type}")
        print(f"Size      : {content_size}")
        print(f"Content[0:{Protocol.CLIENT_ID_LEN}]: {binascii.hexlify(content[:Protocol.CLIENT_ID_LEN]).decode()}...")

    elif request.code == RequestCode.PULL_MESSAGES:
        print("---- PULL MESSAGES ---- (no payload)")

    print("===================================================\n")

def print_response_debug(response):
    print("\n================= [DEBUG RESPONSE] =================")
    print(f"Version   : {response.version}")
    try:
        code_name = ResponseCode(response.code)
    except ValueError:
        code_name = "UNKNOWN"
    print(f"Code      : {response.code} ({code_name})")
    print(f"P Size    : {len(response.payload)} bytes")

    if response.code == ResponseCode.REGISTER_OK:
        cid = response.payload[:Protocol.CLIENT_ID_LEN]
        print("---- REGISTER_OK ----")
        print(f"Assigned ID: {binascii.hexlify(cid).decode()}")

    elif response.code == ResponseCode.LIST_CLIENTS:
        rec_size = Protocol.CLIENT_ID_LEN + Protocol.MAX_PUBKEY_LEN
        n = len(response.payload) // rec_size
        print(f"---- LIST_CLIENTS ---- ({n} clients)")
        for i in range(n):
            start = i * rec_size
            cid = response.payload[start:start + Protocol.CLIENT_ID_LEN]
            name_bytes = response.payload[start + Protocol.CLIENT_ID_LEN:start + Protocol.CLIENT_ID_LEN + Protocol.MAX_USERNAME_LEN]
            name = name_bytes.split(b"\x00", 1)[0].decode("ascii","ignore")
            print(f"{i+1}. {name} (id={binascii.hexlify(cid).decode()})")

    elif response.code == ResponseCode.GET_PUBKEY:
        cid = response.payload[:Protocol.CLIENT_ID_LEN]
        pubkey = response.payload[Protocol.CLIENT_ID_LEN:]
        print("---- GET_PUBKEY ----")
        print(f"Client ID : {binascii.hexlify(cid).decode()}")
        print(f"PubKey[0:{Protocol.CLIENT_ID_LEN}]: {binascii.hexlify(pubkey[:Protocol.CLIENT_ID_LEN]).decode()}...")

    elif response.code == ResponseCode.SEND_OK:
        print("---- SEND_OK ---- (message stored)")

    elif response.code == ResponseCode.PULL_MESSAGES:
        off = 0
        i = 1
        print("---- PULL_MESSAGES ----")
        while off < len(response.payload):
            from_id = response.payload[off:off + Protocol.CLIENT_ID_LEN]
            off+=Protocol.CLIENT_ID_LEN
            msg_type = response.payload[off]
            off += Protocol.TYPE_LEN
            size = struct.unpack_from("<I", response.payload, off)[0]
            off += Protocol.MAX_MESSAGE_CONTENT_SIZE
            content = response.payload[off:off+size]; off+=size
            print(f"{i}. From={binascii.hexlify(from_id).decode()}, "
                  f"Type = {msg_type}, Size = {size},"
                  f"Content[0:{Protocol.CLIENT_ID_LEN}]={binascii.hexlify(content[:Protocol.CLIENT_ID_LEN]).decode()}...")
            i += 1

    elif response.code == ResponseCode.ERROR:
        print("---- ERROR ----")
        print(f"Message: {response.payload.decode(errors='ignore')}")

    print("===================================================\n")