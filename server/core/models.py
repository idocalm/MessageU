class Client: 
    def __init__(self, id, username, pubkey, last_seen):
        self.id = id
        self.username = username
        self.pubkey = pubkey
        self.last_seen = last_seen

class Message:
    def __init__(self, id, to_id, from_id, type, content):
        self.id = id
        self.to_id = to_id
        self.from_id = from_id
        self.type = type
        self.content = content