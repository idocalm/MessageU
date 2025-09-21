import sqlite3 
import uuid
from datetime import datetime
from core.models import Client, Message

class Database:
    def __init__(self, db_path):
        self.conn = sqlite3.connect(db_path, check_same_thread=False)
        self.conn.row_factory = sqlite3.Row
        self._init_db()

    def _init_db(self):
        cur = self.conn.cursor()
        cur.execute("""
            CREATE TABLE IF NOT EXISTS clients (           
                ID BLOB(16) PRIMARY KEY,
                UserName TEXT UNIQUE NOT NULL,
                PublicKey BLOB NOT NULL,
                LastSeen TEXT NOT NULL      
            )
        """)
        
        cur.execute("""
            CREATE TABLE IF NOT EXISTS messages (
                    ID INTEGER PRIMARY KEY AUTOINCREMENT,
                    ToClient BLOB(16) NOT NULL,
                    FromClient BLOB(16) NOT NULL,
                    Type INTEGER NOT NULL,
                    Content BLOB NOT NULL
                )
        """)

        self.conn.commit()
    
    # Client operations
    def add_client(self, username, pubkey):
        '''
        Adds a new client to the DB, and returns its 
        client_id (16 bytes), or raises an exception if the username exists. 
        '''
        client_id = uuid.uuid4().bytes
        cur = self.conn.cursor()

        cur.execute("""
            INSERT INTO clients (ID, UserName, PublicKey, LastSeen)
            VALUES (?, ?, ?, ?)
        """, (client_id, username, pubkey, self._now()))

        self.conn.commit()

        return client_id
    
    def get_client_by_name(self, username):
        cur = self.conn.cursor()
        cur.execute("SELECT * FROM clients WHERE UserName = ?", (username,))
        row = cur.fetchone()
        return self._parse_client(row) if row else None
    
    def get_all_clients(self):
        cur = self.conn.cursor()
        cur.execute("SELECT * FROM clients")
        rows = cur.fetchall()
        return [self._parse_client(row) for row in rows]
    
    def get_pubkey(self, client_id):
        cur = self.conn.cursor()
        cur.execute("SELECT PublicKey FROM clients WHERE ID = ?", (client_id,))
        row = cur.fetchone()
        return row["PublicKey"] if row else None
    
    def upade_last_seen(self, client_id):
        cur = self.conn.cursor()
        cur.execute("UPDATE clients SET LastSeen = ? WHERE ID = ?", (self._now(), client_id))
        self.conn.commit()

    # Messages
    def add_message(self, to_id, from_id, type, content):
        cur = self.conn.cursor()
        cur.execute("""
            INSERT INTO messages (ToClient, FromClient, Type, Content)
            VALUES (?, ?, ?, ?)
        """, (to_id, from_id, type, content))
        self.conn.commit()
        return cur.lastrowid
    
    def pull_messages(self, client_id):
        cur = self.conn.cursor()
        cur.execute("SELECT * FROM messages WHERE ToClient = ?", (client_id,))
        rows = cur.fetchall()
        messages = [self._parse_message(row) for row in rows]
        
        # Delete after pulling
        cur.execute("DELETE FROM messages WHERE ToClient = ?", (client_id,))
        self.conn.commit()

        return messages
    
    # Helpers

    def _parse_client(self, row):
        return Client(
            id=row["ID"],
            username=row["UserName"],
            pubkey=row["PublicKey"],
            last_seen=row["LastSeen"]
        )
    
    def _parse_message(self, row):
        return Message(
            id=row["ID"],
            to_id=row["ToClient"],
            from_id=row["FromClient"],
            type=row["Type"],
            content=row["Content"]
        )
    
    def _now(self):
        return datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S")