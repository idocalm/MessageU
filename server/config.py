import os 

FALLBACK_PORT = 1357
PORT_INFO_FILE = "myport.info"
DEBUG = True

PORT_RANGE = (1, 65535)

class Config: 
    def __init__(self):
        self.port = FALLBACK_PORT
        self.version = 2
        self.debug = DEBUG

        if os.path.exists(PORT_INFO_FILE):
            try:
                with open(PORT_INFO_FILE, "r") as f:
                    port = int(f.read().strip())
                    if PORT_RANGE[0] <= port <= PORT_RANGE[1]:
                        self.port = port
                    else:
                        raise ValueError(f"Port {port} out of range {PORT_RANGE}")
            except Exception as e:
                print(f"Error reading port from file: {e}. Using fallback port {FALLBACK_PORT}.")

