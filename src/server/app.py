from config import Config
from core.db import Database
from core.dispatcher import Dispatcher
from network.tcp_server import TCPServer


def main():
    """ Main project function: creates db, stars server """
    cfg = Config()

    db = Database()

    dispatcher = Dispatcher(db, debug=cfg.debug)
    server = TCPServer(cfg.port, dispatcher)
    server.start()

if __name__ == "__main__":
    main()