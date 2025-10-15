import os 

FALLBACK_PORT = 1357
PORT_INFO_FILE = "myport.info"
DEBUG = True

PORT_RANGE = (1, 65535)

class Config: 
    def __init__(self):
        """
        the config holds values for other classes to use like port which is read from myport.info (or fallback)
        whether or not the app runs in debug mode (see README.md for details on debug mode)
        and the server version, which is 2 since we have sql db as a bonus
        """
        self.port = FALLBACK_PORT
        self.version = 2
        self.debug = DEBUG

        if os.path.exists(PORT_INFO_FILE):
            try:
                with open(PORT_INFO_FILE, "r") as f:
                    port = int(f.read().strip())
                    # check port is between the valid range
                    if PORT_RANGE[0] <= port <= PORT_RANGE[1]:
                        self.port = port
                    else:
                        raise ValueError(f"Port {port} out of range {PORT_RANGE}")
            except Exception as e:
                print(f"Error reading port from file: {e}. Using fallback port {FALLBACK_PORT}.")

