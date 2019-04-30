from datetime import datetime
from Crypto.Hash import SHA256

class RequestObject:

    requester: str
    hashValue: SHA256
    createdOn: datetime

    def __init__(self, requester):
        self.requester = requester
        self.hashValue = None
        self.createdOn = datetime.now()