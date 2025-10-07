# MessageU Server:

### IDO CALMAN, T.Z. 331771535, 2025C

### VERSION 2: THIS SERVER INCLUDES THE SQL BONUS

The server works like this:

Once the app starts, it:

1. Setup db, create the file if needed, create the tables if needed
2. create a dispatcher instance - this part is in charge of handling the requests and passing them to the different handlers by the request code
3. create a TCPServer instance, which initiates the server socket and starts listening. On any connection we create a new thread for it, which runs the Connection.handle function
4. The handle function on the Connection instance class receives data from the connection, parses it as a RequestFrame, hands it to the dispatcher which returns a ResponseFrame, which is encoded as bytes and sent over to the client.

A RequestFrame mentioned above is a class that represents any kind of request that arrives to the server as the protocol defines:

```
class RequestFrame:

    '''
    A request frame is constructed of:
    - 16 bytes of client ID
    - 1 byte for version
    - 2 bytes for request code
    - 4 bytes for payload size
    '''

    - from_bytes function: unpacks a byte stream into a client_id, version, code and payload, and returns a RequestFrame that represents it.


```

Very similar, a ResponseFrame is:

```
class ResponseFrame:
    '''
    A response frame is constructed of:
    - 1 byte for version
    - 2 bytes for response code
    - 4 bytes for payload size
    '''

    This one doesn't need a from_bytes (the server creates it), but we do need a

    - to_bytes: packs the class version, code, payload into a byte stream of header + payload

```

So, given all the above, a normal server operation would go like this:

- Setup
- Client connects, new thread #1 is created
- Thread #1 waits for data
- Client sends a byte stream (maybe he is trying to register)
- Thread #1 receives the byte stream:
  - Creates a RequestFrame of client_id, version, code, payload
  - Dispatcher receives the RequestFrame, sees code = RequestCode.REGISTER
  - Dispatcher calls RegisterHandler.handle with the built RequestFrame
  - RegisterHandler.handle runs validation on the format, checks if the username is ok for registering with, creates / rejects a new user, returns a fitting ResponseFrame with status REGISTER_OK or ERROR
  - Dispatcher takes the ResponseFrame, calls to_bytes on, and sends it through the socket.

The handlers we support are:

1. GetPubKeyHandler
2. ListClientsHandler
3. PullMessagesHandler
4. RegisterHandler
5. SendMessageHandler

Each handler hides his own logic inside the handle function. they might interact with the database, do their own checks, etc, as long as they end up returning a ResponseFrame

### TESTING

While developing the server I made some python tests for the server, they are inside

tests/

### RUNNING THE SERVER

to run: python app.py (maybe you need python3 app.py in your system)
