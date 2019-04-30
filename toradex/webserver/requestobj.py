#****************************************************************************
#
# Copyright (C) 2019 Toradex AG
# Contact: https://www.toradex.com/locations
#
# This file is part of the Toradex of the A71CH workshop demo.
#
# BSD License Usage
# Alternatively, you may use this file under the terms of the BSD license
# as follows:
#
# "Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Toradex Ag nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#****************************************************************************/

from datetime import datetime
from Crypto.Hash import SHA256

# Data class to store details for one request from a client
class RequestObject:

    requester: str
    hashValue: SHA256
    createdOn: datetime

    def __init__(self, requester):
        self.requester = requester
        self.hashValue = None
        self.createdOn = datetime.now()