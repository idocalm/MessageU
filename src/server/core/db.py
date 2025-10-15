import sqlite3 
import uuid
from datetime import datetime
from core.models import Client, Message
from protocol.codes import Protocol

class Database:
    """ 
    The database class handles all operations with defensive.db
    This includes reading and writing to db, parsing data from db as Client or Message
    """
    def __init__(self, db_path):
        """ Calls on server init, creates the db file if doesn't exist """
        self.conn = sqlite3.connect(db_path, check_same_thread=False)
        self.conn.row_factory = sqlite3.Row
        self._init_db()

    def _init_db(self):
        """ Calls on server init, creates tables clients & messages if they don't exist already in the file """
        cur = self.conn.cursor()
        cur.execute(f"""
            CREATE TABLE IF NOT EXISTS clients (           
                ID BLOB({Protocol.CLIENT_ID_LEN}) PRIMARY KEY,
                UserName TEXT(255) UNIQUE NOT NULL,
                PublicKey BLOB({Protocol.MAX_PUBKEY_LEN}) NOT NULL,
                LastSeen TEXT NOT NULL      
            )
        """)
        
        cur.execute(f"""
            CREATE TABLE IF NOT EXISTS messages (
                    ID INTEGER PRIMARY KEY AUTOINCREMENT,
                    ToClient BLOB({Protocol.CLIENT_ID_LEN}) NOT NULL,
                    FromClient BLOB({Protocol.CLIENT_ID_LEN}) NOT NULL,
                    Type INTEGER NOT NULL,
                    Content BLOB NOT NULL
                )
        """)

        self.conn.commit()
    
    # Client operations
    def add_client(self, username, pubkey):
        """
        Adds a new client to the DB, and returns its 
        client_id ({Protocol.CLIENT_ID_LEN} bytes), or raises an exception if the username exists. 
        """

        client_id = uuid.uuid4().bytes
        cur = self.conn.cursor()

        cur.execute("""
            INSERT INTO clients (ID, UserName, PublicKey, LastSeen)
            VALUES (?, ?, ?, ?)
        """, (client_id, username, pubkey, self._now()))

        self.conn.commit()

        return client_id
    
    def get_client_by_name(self, username):
        """ Retrieve a Client object representing a client in the db with the username, if exists, or None """
        cur = self.conn.cursor()
        cur.execute("SELECT * FROM clients WHERE UserName = ?", (username,))
        row = cur.fetchone()
        return self._parse_client(row) if row else None
    
    def get_client_by_id(self, id):
        """ Retrieve a Client object representing a client in the db with the id, if exists, or None """
        cur = self.conn.cursor()
        cur.execute("SELECT * FROM clients WHERE ID = ?", (id,))
        row = cur.fetchone()
        return self._parse_client(row) if row else None

    def get_all_clients(self):
        """ Get all clients in the DB as Client objects """
        cur = self.conn.cursor()
        cur.execute("SELECT * FROM clients")
        rows = cur.fetchall()
        return [self._parse_client(row) for row in rows]
    
    def get_pubkey(self, client_id):
        """ Get the PublicKey of a client with the client_id if exists, or None """
        cur = self.conn.cursor()
        cur.execute("SELECT PublicKey FROM clients WHERE ID = ?", (client_id,))
        row = cur.fetchone()
        return row["PublicKey"] if row else None
    
    def update_last_seen(self, client_id):
        """ Update a user last see to now based on his client_id """
        cur = self.conn.cursor()
        cur.execute("UPDATE clients SET LastSeen = ? WHERE ID = ?", (self._now(), client_id))
        self.conn.commit()

    # Messages
    def add_message(self, to_id, from_id, type, content):
        """ Saves a new message in the DB based on the params and return its id in the table """
        cur = self.conn.cursor()
        cur.execute("""
            INSERT INTO messages (ToClient, FromClient, Type, Content)
            VALUES (?, ?, ?, ?)
        """, (to_id, from_id, type, content))
        self.conn.commit()
        return cur.lastrowid
    
    def pull_messages(self, client_id):
        """ Pull all messages as an array of a client by id = client_id """
        cur = self.conn.cursor()
        cur.execute("SELECT * FROM messages WHERE ToClient = ?", (client_id,))
        rows = cur.fetchall()
        messages = [self._parse_message(row) for row in rows]
        
        # Delete the messages after pulling
        cur.execute("DELETE FROM messages WHERE ToClient = ?", (client_id,))
        self.conn.commit()

        return messages
    
    # Helpers
    def _parse_client(self, row):
        """ Parses a row by its values and returns a Client class """
        return Client(
            id=row["ID"],
            username=row["UserName"],
            pubkey=row["PublicKey"],
            last_seen=row["LastSeen"]
        )
    
    def _parse_message(self, row):
        """ Parses a row by its values and returns a Message class """
        return Message(
            id=row["ID"],
            to_id=row["ToClient"],
            from_id=row["FromClient"],
            type=row["Type"],
            content=row["Content"]
        )
    
    def _now(self):
        """ A helper to format the LastSeen parameter """
        return datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S")