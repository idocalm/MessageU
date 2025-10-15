class Client: 
    def __init__(self, id, username, pubkey, last_seen):
        """
        A client is just a wrapper to avoid working with raw dictionaries from DB. its a representation of a single row in db of the clients
        table.
        """

        self.id = id
        self.username = username
        self.pubkey = pubkey
        self.last_seen = last_seen

class Message:
    def __init__(self, id, to_id, from_id, type, content):
        """
        A single message object represents a row of the dbs messages table. 
        """
        self.id = id
        self.to_id = to_id
        self.from_id = from_id
        self.type = type
        self.content = content