from protocol.codes import RequestCode, ResponseCode
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
        raw_name = request.payload[:255]
        username = raw_name.split(b"\x00", 1)[0].decode("ascii", errors="ignore")
        pubkey = request.payload[255:255+160]
        print("---- REGISTER ----")
        print(f"Username  : {username}")
        print(f"PubKey[0:16]: {binascii.hexlify(pubkey[:16]).decode()}...")

    elif request.code == RequestCode.LIST_CLIENTS:
        print("---- LIST CLIENTS ---- (no payload)")

    elif request.code == RequestCode.GET_PUBKEY:
        target_id = request.payload[:16]
        print("---- GET PUBKEY ----")
        print(f"Target ID : {binascii.hexlify(target_id).decode()}")

    elif request.code == RequestCode.SEND_MESSAGE:
        to_id = request.payload[:16]
        msg_type = request.payload[16]
        content_size = struct.unpack_from("<I", request.payload, 17)[0]
        content = request.payload[21:21+content_size]
        print("---- SEND MESSAGE ----")
        print(f"To ID     : {binascii.hexlify(to_id).decode()}")
        print(f"Type      : {msg_type}")
        print(f"Size      : {content_size}")
        print(f"Content[0:16]: {binascii.hexlify(content[:16]).decode()}...")

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
        cid = response.payload[:16]
        print("---- REGISTER_OK ----")
        print(f"Assigned ID: {binascii.hexlify(cid).decode()}")

    elif response.code == ResponseCode.LIST_CLIENTS:
        rec_size = 16+255
        n = len(response.payload)//rec_size
        print(f"---- LIST_CLIENTS ---- ({n} clients)")
        for i in range(n):
            start = i*rec_size
            cid = response.payload[start:start+16]
            name_bytes = response.payload[start+16:start+16+255]
            name = name_bytes.split(b"\x00",1)[0].decode("ascii","ignore")
            print(f"{i+1}. {name} (id={binascii.hexlify(cid).decode()})")

    elif response.code == ResponseCode.GET_PUBKEY:
        cid = response.payload[:16]
        pubkey = response.payload[16:]
        print("---- GET_PUBKEY ----")
        print(f"Client ID : {binascii.hexlify(cid).decode()}")
        print(f"PubKey[0:16]: {binascii.hexlify(pubkey[:16]).decode()}...")

    elif response.code == ResponseCode.SEND_OK:
        print("---- SEND_OK ---- (message stored)")

    elif response.code == ResponseCode.PULL_MESSAGES:
        off = 0
        idx = 1
        print("---- PULL_MESSAGES ----")
        while off < len(response.payload):
            from_id = response.payload[off:off+16]; off+=16
            msg_type = response.payload[off]; off+=1
            size = struct.unpack_from("<I", response.payload, off)[0]; off+=4
            content = response.payload[off:off+size]; off+=size
            print(f"{idx}. From={binascii.hexlify(from_id).decode()}, "
                  f"Type={msg_type}, Size={size}, "
                  f"Content[0:16]={binascii.hexlify(content[:16]).decode()}...")
            idx+=1

    elif response.code == ResponseCode.ERROR:
        print("---- ERROR ----")
        print(f"Message: {response.payload.decode(errors='ignore')}")

    print("===================================================\n")