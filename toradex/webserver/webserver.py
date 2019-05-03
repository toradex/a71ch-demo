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

import sys, os, urllib, coloredlogs, logging, ssl, random, string
from http.server import BaseHTTPRequestHandler, HTTPServer
from requestobj import RequestObject
from datetime import datetime, timedelta
from Crypto.PublicKey import ECC
from Crypto.Signature import DSS
from Crypto.Hash import SHA256
from Crypto.Util.asn1 import DerSequence
from binascii import unhexlify

class HttpRequestHandler(BaseHTTPRequestHandler):

    _root_ca = '../certs/tls_rootca.cer'
    _server_cert = '../certs/tls_server.cer'
    _server_key = '../certs/tls_server_key.pem'
    _client_public_key = "../certs/tls_client_key_pub.pem" #static client public key
    _files_location = "../files/"
    protocol_version = 'HTTP/1.1' #Required to hold the session to the clients
    _request_authority_timout_sec = 60 #After this defined timeout (in seconds), the request is deprecated and will be deleted
    _activeRequesters = [] #Active ongoing requests, holding objects of class RequestObject

    def _generate_random_hash(self):
        letters = string.ascii_lowercase
        plainText = ''.join(random.choice(letters) for i in range(random.randint(1,100)))
        hashToSign = SHA256.new(plainText.encode("utf-8"))
        return hashToSign

    def _get_requester_from_active_requesters_by_address(self, sourceOfRequest):
        requestObj = None
        logging.debug("Search for requester with address \"%s\".", sourceOfRequest)
        for requester in HttpRequestHandler._activeRequesters:
            if requester.requester == sourceOfRequest:
                requestObj = requester
                requestObj.createdOn = datetime.now()
                logging.debug("Found existing requester in the list of active requesters.")
                break
        return requestObj

    def _clean_list_of_active_requests(self):
        total = len(HttpRequestHandler._activeRequesters)
        cnt = 0
        update = []
        deprecatedTime = datetime.now() - timedelta(hours=0, minutes=0, seconds=HttpRequestHandler._request_authority_timout_sec)
        logging.debug("Removing now all RequestObjects from list of active requesters which are older than %s.", deprecatedTime)
        for requester in HttpRequestHandler._activeRequesters:
            if requester.createdOn > deprecatedTime:  #only add RequestObject if the object is younger than the deprecatedTime
                update.append(requester)
            else:
                cnt = cnt + 1
        HttpRequestHandler._activeRequesters = update
        logging.debug("Removed %i of %i entries from the list of active requesters.", cnt, total)

    def _remove_requester_from_active_requests_by_address(self, sourceOfRequest):
        total = len(HttpRequestHandler._activeRequesters)
        cnt = 0
        update = []
        logging.debug("Remove requester with address \"%s\" from list of active requesters.", sourceOfRequest)
        for requester in HttpRequestHandler._activeRequesters:
            if requester.requester != sourceOfRequest:
                update.append(requester)
            else:
                cnt = cnt + 1
        HttpRequestHandler._activeRequesters = update
        logging.debug("Removed %i of %i entries from the list of active requesters.", cnt, total)

    def _check_signature(self, sourceOfRequest, signature):
        requestObj = self._get_requester_from_active_requesters_by_address(sourceOfRequest)
        if (requestObj == None):
            return

        f = open(self._client_public_key,'rt')
        key = ECC.import_key(f.read())
        verifier = DSS.new(key, 'deterministic-rfc6979')  #deterministic-rfc6979   fips-186-3

        # The following part where the bytes are parsed in a very specific way
        # is required, because the related Crypto.Signature library seem to have a bug.
        sigByte = unhexlify(signature)
        der_seq = DerSequence().decode(sigByte)
        rs = bytearray(64)
        rs[0:32] = der_seq[0].to_bytes(32, byteorder='big')
        rs[32:64] = der_seq[1].to_bytes(32, byteorder='big')
        try:
            verifier.verify(requestObj.hashValue, bytes(rs))
            logging.debug("The received signature from the client is valid.")
            return True
        except ValueError:
            logging.error("The received signature from the client is NOT valid!")
            return False

    def _send_file_to_client(self, filename):
        try:
            path = self._files_location + filename
            file = open(path, 'rb')
            content = file.read()
            file.close()
            sha256 = SHA256.new(content)
        except:
            content = "File not available.".encode('utf-8')
            logging.error("Requested file \"%s\" is not available. Sending a file with content \"File not available.\"", filename)
            sha256 = SHA256.new(str(random.randint(1,100)).encode('utf-8'))

        logging.debug("SHA256 = %s", sha256.hexdigest())
        logging.debug("File size is: %s", str(len(content)))
        body = (("Filesize=" + str(len(content)) + "\nFilename=" + filename + "\nSHA256=" + sha256.hexdigest() + "\n").encode('utf-8') + content)
        self.send_response(200)
        self.wfile.write(body)

    def do_AUTHHEAD(self, sourceOfRequest):
        requestObj = RequestObject(sourceOfRequest)
        requestObj.hashValue = self._generate_random_hash()
        logging.debug("Source: %s\nHash to sign: %s\nTimestamp: %s", requestObj.requester, requestObj.hashValue.hexdigest(), requestObj.createdOn)
        HttpRequestHandler._activeRequesters.append(requestObj)
        self.send_response(401)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(("hashToSign=" + requestObj.hashValue.hexdigest() + "\r\n").encode('utf-8'))

    #Entry point for post requests and main loop
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        logging.debug("New POST request:\n************************************\nPath: %s\nHeaders:\n%sBody:\n%s \n",
                str(self.path), str(self.headers), post_data.decode('utf-8'))
        postvars = urllib.parse.parse_qs(post_data.decode('utf-8'), keep_blank_values=0, errors='replace', max_num_fields=None)
        logging.debug("Postvars received from POST request: \"%s\".\nEnd of details ************\n", postvars)

        #Get IP of requester
        requesterIp = self.client_address[0]
        logging.info("Received new POST-request of client with address %s.", requesterIp)

        self._clean_list_of_active_requests()
        requestObj = self._get_requester_from_active_requesters_by_address(requesterIp)
        if requestObj == None:
            logging.info("Requester is not authorized. Starting authorization process.")
            self.do_AUTHHEAD(requesterIp)
            logging.info("Sent the hash to sign to the client. Ready for new requests...\n\n\n")

        else:
            logging.info("Requester already received a hash to sign. Checking his signature with his corresponding public key.")
            signature = postvars['signature'][0]
            filename = postvars['filename'][0]
            logging.debug("Received Signature: %s", signature)
            logging.debug("Received Filename: %s", filename)
            if(self._check_signature(requesterIp, signature)):
                logging.info("The signature of the client is valid. Now starting upload of requested file: \"%s\".", filename)
                self._send_file_to_client(filename)
                self._remove_requester_from_active_requests_by_address(requesterIp)
                logging.info("File transfer completed. Ready for new requests...\n\n\n")
            else:
                logging.error("The signature of the client is NOT valid. The requests file won't be sent. Ready for new requests...\n\n\n")

#Entry point for the application
def run(server_class=HTTPServer, handler_class=HttpRequestHandler, port=8080):
    coloredlogs.DEFAULT_LOG_FORMAT = '%(levelname)s %(message)s'
    coloredlogs.DEFAULT_LEVEL_STYLES = {'critical': {'color': 'red', 'bold': True}, 'debug': {'color': 'yellow', 'bold': True}, 'error': {'color': 'red'}, 'info': {'color': 'cyan', 'bold': True}, 'notice': {'color': 'magenta'}, 'spam': {'color': 'green', 'faint': True}, 'success': {'color': 'green', 'bold': True}, 'verbose': {'color': 'blue'}, 'warning': {'color': 'yellow'}}
    coloredlogs.install(level=logging.INFO)
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    httpd.socket = ssl.wrap_socket(httpd.socket, server_side=True,ca_certs=HttpRequestHandler._root_ca, certfile=HttpRequestHandler._server_cert, keyfile=HttpRequestHandler._server_key)
    logging.info('\n\n\n************* A71CH Toradex Demo Server *************\n\n\n')
    logging.info('HTTP server is ready now and listening for the first request...')

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    logging.info('Stopping httpd...\n')

run()
