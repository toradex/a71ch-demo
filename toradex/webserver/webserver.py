import sys, os, urllib, logging, ssl, random, string
from http.server import BaseHTTPRequestHandler, HTTPServer
from requestobj import RequestObject
from datetime import datetime, timedelta
from Crypto.PublicKey import ECC
from Crypto.Signature import DSS
from Crypto.Hash import SHA256
from Crypto.Util.asn1 import DerSequence
from binascii import unhexlify

class HttpRequestHandler(BaseHTTPRequestHandler):

    _client_public_key = "../certs/tls_client_key_pub.pem"
    _files_location = "../files/"
    protocol_version = 'HTTP/1.1'
    _request_authority_timout_sec = 60
    _activeRequesters = [] #Active ongoing requests, holding objects of class RequestObject

    def _set_response(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def _generate_random_hash(self):
        letters = string.ascii_lowercase
        plainText = ''.join(random.choice(letters) for i in range(random.randint(1,100)))
        hashToSign = SHA256.new(plainText.encode("utf-8"))
        return hashToSign

    def _get_requester(self, sourceOfRequest):
        requestObj = None
        logging.debug("Search for requester with address %s.", sourceOfRequest)
        for requester in HttpRequestHandler._activeRequesters:
            if requester.requester == sourceOfRequest:
                requestObj = requester
                requestObj.createdOn = datetime.now()
                logging.debug("Found existing requester.")
                break
        return requestObj

    def _clean_list_of_requests(self):
        total = len(HttpRequestHandler._activeRequesters)
        cnt = 0
        update = []
        deprecatedTime = datetime.now() - timedelta(hours=0, minutes=0, seconds=HttpRequestHandler._request_authority_timout_sec)
        logging.debug("Removing now all RequestObjects which are older than %s.", deprecatedTime)
        for requester in HttpRequestHandler._activeRequesters:
            if requester.createdOn > deprecatedTime:  #only add RequestObject if the object is younger than the deprecatedTime
                update.append(requester)
            else:
                cnt = cnt + 1
        HttpRequestHandler._activeRequesters = update
        logging.debug("Removed %i of %i entries.", cnt, total)
    
    def _remove_entry_from_list_of_requests(self, sourceOfRequest):
        total = len(HttpRequestHandler._activeRequesters)
        cnt = 0
        update = []
        logging.debug("Remove requester with address \"%s\" from list of requesters.", sourceOfRequest)
        for requester in HttpRequestHandler._activeRequesters:
            if requester.requester != sourceOfRequest:
                update.append(requester)
            else:
                cnt = cnt + 1
        HttpRequestHandler._activeRequesters = update
        logging.debug("Removed %i of %i entries.", cnt, total)

    def _check_signature(self, sourceOfRequest, signature):
        requestObj = self._get_requester(sourceOfRequest)
        if (requestObj == None):
            return

        f = open(self._client_public_key,'rt')
        key = ECC.import_key(f.read())
        verifier = DSS.new(key, 'deterministic-rfc6979')  #deterministic-rfc6979   fips-186-3

        # This parsing is required because the related Crypto.Signature library has a bug. 
        sigByte = unhexlify(signature)
        der_seq = DerSequence().decode(sigByte)
        rs = bytearray(64)
        rs[0:32] = der_seq[0].to_bytes(32, byteorder='big')
        rs[32:64] = der_seq[1].to_bytes(32, byteorder='big')
        try:
            verifier.verify(requestObj.hashValue, bytes(rs))
            logging.debug("The signature is valid.")
            return True
        except ValueError:
            logging.error("The signature is not valid!")
            return False

    def _provide_file_download(self, filename):
        try:
            path = self._files_location + filename
            file = open(path, 'rb')
            content = file.read()
            file.close()
        except:
            content = "File not available.".encode('utf-8')
            logging.error("Requested file \"%s\" is not available.", filename)
        sha256 = SHA256.new(content)
        logging.debug("SHA256 = %s", sha256.hexdigest())
        logging.debug("File size is: %s", str(len(content)))
        body = (("Filesize=" + str(len(content)) + "\nFilename=" + filename + "\nSHA256=" + sha256.hexdigest() + "\n").encode('utf-8') + content)
        self.send_response(200)
        self.wfile.write(body)
        logging.info("File transfer completed.")

    def do_AUTHHEAD(self, sourceOfRequest):
        logging.info("Requester is not authorized. Starting authorization process.")
        requestObj = RequestObject(sourceOfRequest)
        requestObj.hashValue = self._generate_random_hash()
        logging.debug("Source: %s\nHash to sign: %s\nTimestamp: %s", requestObj.requester, requestObj.hashValue.hexdigest(), requestObj.createdOn)
        HttpRequestHandler._activeRequesters.append(requestObj)
        self.send_response(401)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(("hashToSign=" + requestObj.hashValue.hexdigest() + "\r\n").encode('utf-8'))
        logging.info("Sent hash for signature process to the client. Waiting for new requests...")

    def do_GET(self):
        logging.info("Received GET request. Nothing to do...")
        logging.debug("GET request,\nPath: %s\nHeaders:\n%s\n", str(self.path), str(self.headers))
        self._set_response()
        self.wfile.write("Demo trusted file server.".encode('utf-8'))

    def do_POST(self):
        content_length = int(self.headers['Content-Length']) # <--- Gets the size of data
        post_data = self.rfile.read(content_length) # <--- Gets the data itself
        logging.debug("New POST request:\n************************************\nPath: %s\nHeaders:\n%sBody:\n%s \n",
                str(self.path), str(self.headers), post_data.decode('utf-8'))
        postvars = urllib.parse.parse_qs(post_data.decode('utf-8'), keep_blank_values=0, errors='replace', max_num_fields=None)
        logging.debug("Postvars received from POST request: \"%s\".\nEnd of details ************\n", postvars)

        #Get IP of requester
        requesterIp = self.client_address[0]
        logging.info("Received POST-request of client with address %s.", requesterIp)

        self._clean_list_of_requests()
        requestObj = self._get_requester(requesterIp)
        if requestObj == None:
            self.do_AUTHHEAD(requesterIp)
        else:
            logging.info("Requester is authorized. Checking his signature with the corresponding public key.")
            signature = postvars['signature'][0]
            filename = postvars['filename'][0]
            logging.debug("Received Signature: %s", signature)
            logging.debug("Received Filename: %s", filename)
            if(self._check_signature(requesterIp, signature)):
                logging.info("Signature is valid. Starting upload of requested file.")
                self._provide_file_download(filename)
                self._remove_entry_from_list_of_requests(requesterIp)
            else:
                #error
                logging.error("Do an error routine")

def run(server_class=HTTPServer, handler_class=HttpRequestHandler, port=8080):
    logging.basicConfig(level=logging.INFO)
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    httpd.socket = ssl.wrap_socket(httpd.socket, server_side=True,ca_certs='../certs/tls_rootca.cer',certfile='../certs/tls_server.cer',keyfile='../certs/tls_server_key.pem')
    logging.info('*************  *************\n')
    logging.info('HTTP server is ready now and listening for the first request...')

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    logging.info('Stopping httpd...\n')

run()
