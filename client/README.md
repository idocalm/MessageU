# MessageU Client:

### IDO CALMAN, T.Z. 331771535, 2025C

### VERSION 2: THIS CLIENT INCLUDES THE FILE SEND BONUS

File structure:

1. build/.o - object files

2. core/ - client_comm - implements list_clients, pull_messages, get_pubkey and send_message abilites for a user.

   - client_crypto - handles decryption of messages, symmetric keys logic (request and send)

   - client_init - basic client functions, working with me.info, registering

3. crypto/ - wrappers for AES, Base64, RSA (they are mostly completely from the course website and untouched)

4. network/tcp_client.cpp/.h - implementation of the lower level TCPClient for receiving, connecting, and sending things to the socket

5. protocol/

   - codes.h - contains enums for RequestCode per the defined operations in the coursebook, ResponseCode and MessageType-s.

   Codes are mainly used for checking server response and avoiding magic numbers in payloads

   - constants.h - defines the Protocol namespace containing some constants like max payload size (in bytes) and more. There are also helper functions for working with little endian values and transfering from and to hex

   - framing.h - defines the RequestFrame and ResponseFrame (the usage of these is very similar to the work in the server README.md). We always do something like:
        ResponseFrame::from_bytes(tcp_.receive());
    this returns an object with all the fields neccessary - payload, response code and version

    Same goes for RequestFrame. When we want to send something to the server we create a new RequestFrame, fill it with information (client_id, version, code and payload) and then send it after .to_bytes(). 

