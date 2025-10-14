from config import Config
from core.db import Database
from core.dispatcher import Dispatcher
from network.tcp_server import TCPServer

DB = "defensive.db"

def main():
    cfg = Config()

    db = Database(DB)

    dispatcher = Dispatcher(db, debug=cfg.debug)
    server = TCPServer(cfg.port, dispatcher)
    server.start()
    
    print(f"[+] Server v{cfg.version} listening on port {cfg.port}, debug={cfg.debug}")

if __name__ == "__main__":
    main()