# Tester dedicated to the 42 project webserv.
# 04/11/2020
# Reproduction of the "official" tester given in the project page (named "tester" or "ubuntu_tester")
# Thanks Wireshark

# ---------------------------------------------------------------------------
# ---------------------------- 1. CHECKS ------------------------------------
# ---------------------------------------------------------------------------

import os
import sys
import json
import requests

from requests.auth import HTTPBasicAuth
from sys import platform

global verbose
verbose = 0

if (len(sys.argv) == 3 and sys.argv[2] == '-v'):
    verbose = 1
if (os.system('lsof -c webserv > /dev/null') != 0):
    print("Webserv is not running")
    exit()

# ---------------------------------------------------------------------------
# ----------------------------- 2. CORE -------------------------------------
# ---------------------------------------------------------------------------

class assertTypes:     
    BODY_CONTAIN_ASSERT = 1
    FILE_CONTAIN_ASSERT = 2
    RES_HD_CONTAIN_ASSERT = 3
    
class bcolors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def printHdReqRes(r):
    print()
    indent = "          Request >  "
    for idA, reqKey in enumerate(r.request.headers):
        print(indent + reqKey + ": " + r.request.headers[reqKey])
    print()
    indent = "          Response > "
    for idB, resKey in enumerate(r.headers):
        print(indent + resKey + ": " + r.headers[resKey])
    print()

def bodyContain(r, str):
    if (str in r.text): return True
    else: return False

def fileContain(r, strLookingFor, path):
    file = open(path, 'r').read()
    if (strLookingFor == file): return True
    else: return False

def resHeadersKeyVal(r, hdKeyTab = [], hdValTab = []):
    validated = 0
    for idA, resKey in enumerate(r.headers):
        for idB, wantKey in enumerate(hdKeyTab):
            if (resKey == wantKey):
                if (r.headers[resKey] == hdValTab[idB]):
                    validated += 1
    if (validated == len(hdKeyTab)):
        return True
    return False

def moreAsserts(r, assertLevel, *args):
    indexArgsUsed = 0
    ret = False
    if (assertTypes.BODY_CONTAIN_ASSERT in assertLevel and bodyContain(r, args[0][indexArgsUsed]) == True):
        ret = True
        indexArgsUsed += 1
    if (assertTypes.FILE_CONTAIN_ASSERT in assertLevel and fileContain(r, args[0][indexArgsUsed], args[0][indexArgsUsed + 1]) == True):
        ret = True
        indexArgsUsed += 2
    if (assertTypes.RES_HD_CONTAIN_ASSERT in assertLevel and resHeadersKeyVal(r, args[0][indexArgsUsed], args[0][indexArgsUsed + 1]) == True):
        ret = True
        indexArgsUsed += 2
    return ret

def assertResponse(r, code, index, assertLevel = [], *args):
    ret = False
    if (len(assertLevel)): ret = moreAsserts(r, assertLevel, args)
    else: ret = True
    if (ret and r.status_code == code):
        info = bcolors.OKGREEN + "OK" + bcolors.ENDC + " - " + str(r.status_code) + " " + r.raw.reason
    else:
        info = bcolors.FAIL + "KO" + bcolors.ENDC + " - " + str(r.status_code) + " " + r.raw.reason + " - Should have been received : " + str(code)
    url = "           • #" + str(index).ljust(2, ' ') + " : " + str(r.request.method) + " "
    if (len(r.request.url) > 60):
        url += r.request.url[16:60] + " [..." + str(len(r.request.url)) + "]"
    else: url += str(r.request.url)[16:]
    url = str(url).ljust(80, ' ')
    print(url + "   =   " + info)
    if verbose == 1:
        printHdReqRes(r)

def run(sys):

    print("\n       Platform = " + platform)
    if (len(sys.argv) == 1):
        TESTS_42()
    elif (len(sys.argv) == 2):
            TESTS_42(sys.argv[1])
    print()
    



# -----------------------------------------------------------------------------
# -------------------------------- 3. TESTS -----------------------------------
# -----------------------------------------------------------------------------




def TESTS_42(testNum = 0):

    index = 0
    print("\n     ~ 42 TESTS ------------------------> \n")

# -----------------------------------------------------------------------------> #1 - STAGE 1

    # GET / HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/", headers=hd)
        assertResponse(r, 200, index)

# -----------------------------------------------------------------------------> #2 - STAGE 1
  
    # POST / HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Transfer-Encoding: chunked
    # Content-Type: test/file
    # Accepted-Encoding: gzip

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Transfer-Encoding": "chunked",
            "Content-Type": "test/file",
            "Accepted-Encoding": "gzip"
        }
        payload = "0\r\n\r\n" # --> Need confirmation about Request Python Library
        # The last \n seems to be send in another request ... idk why, may be to trool us but it may raise an error of invalid request
        r = requests.post("http://localhost:8888/", headers=hd, data=payload)
        assertResponse(r, 405, index)
        
# -----------------------------------------------------------------------------> #3 - STAGE 1

    # HEAD / HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1"
        }
        r = requests.head("http://localhost:8888/", headers=hd)
        assertResponse(r, 405, index)

# ----------------------------------------------------------------------------- #4 - STAGE 1

    # GET /directory HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    # Should return ./youpi.bad_extension

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory", headers=hd)
        assertResponse(r, 200, index)

# ----------------------------------------------------------------------------- #5 - STAGE 1
    
    # GET /directory/youpi.bad_extension HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    # Should return ./youpi.bad_extension
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/youpi.bad_extension", headers=hd)
        assertResponse(r, 200, index)

# ----------------------------------------------------------------------------- #6 - STAGE 1
    
    # GET /directory/youpi.bla HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/youpi.bla", headers=hd)
        assertResponse(r, 200, index)

# ----------------------------------------------------------------------------- #7 - STAGE 1
   
    # GET /directory/oulalala HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    # Should return Error

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/oulalala", headers=hd)
        assertResponse(r, 404, index)

# ----------------------------------------------------------------------------- #8 - STAGE 1


    # GET /directory/nop HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    # Should return youpi.bad_extension

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/nop", headers=hd)
        assertResponse(r, 200, index)
  
# ----------------------------------------------------------------------------- #9 - STAGE 1

    # The only difference here is the "/" at the end of the URL

    # GET /directory/nop/ HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    # Should return youpi.bad_extension

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/nop/", headers=hd)
        assertResponse(r, 200, index)
    
# ----------------------------------------------------------------------------- #10 - STAGE 1
    
    # GET /directory/nop/other.pouic HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    # Should return other.pouic

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/nop/other.pouic", headers=hd)
        assertResponse(r, 200, index)
    
# ----------------------------------------------------------------------------- #11 - STAGE 1
  
    # GET /directory/nop/other.pouac HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip
        
    # Should return Error 404
    
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/nop/other.pouac", headers=hd)
        assertResponse(r, 404, index)
  
# ----------------------------------------------------------------------------- #12 - STAGE 1

    # GET /directory/Yeah HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip
    
    # Should return Error 404

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/Yeah", headers=hd)
        assertResponse(r, 404, index)

# ----------------------------------------------------------------------------- #13 - STAGE 1

    # GET /directory/Yeah/not_happy.bad_extension HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Accept-Encoding: gzip

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {
            "Host": "localhost:8080",
            "User-Agent": "Go-http-client/1.1",
            "Accept-Encoding": "gzip"
        }
        r = requests.get("http://localhost:8888/directory/Yeah/not_happy.bad_extension", headers=hd)
        assertResponse(r, 200, index)

# ----------------------------------------------------------------------------- #14 - STAGE 1

    # PUT /put_test/file_should_exist_after HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Transfer-Encoding: chunked
    # Accepted-Encoding: gzip


    # File should exist after with a size of 1000
    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/test42/file_should_exist_after"): os.remove("www/test42/file_should_exist_after")
        payload = "3e8\r\n" # == 1000
        payload += "e" * 1000
        payload += "\r\n"
        payload += "0\r\n\r\n"
        # The last \n seems to be send in another request ... idk why, may be to trool. It may raise an error of invalid request
        r = requests.put("http://localhost:8888/put_test/file_should_exist_after", headers=hd, data=payload)
        assertResponse(r, 201, index)
    
# ----------------------------------------------------------------------------- #15 - STAGE 2

    # PUT /put_test/file_should_exist_after HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Transfer-Encoding: chunked
    # Accepted-Encoding: gzip

    # 32768 =    32 768 en HEX
    # 186A0 =   100 000 en HEX
    # F4240 = 1 000 000 en HEX

    # In this test, body is sent by chunk of 32 768 bytes or 8000 in hexa
    # File should exist after with a size of 10 000 000

    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/test42/file_should_exist_after"): os.remove("www/test42/file_should_exist_after")
        
        # TODO : Loop
        payload = "8000\r\n"
        payload += "z" * 32768
        payload += "\r\n"

        payload += "0\r\n\r\n"
        # The last \n seems to be send in another request ... idk why, may be to trool. It may raise an error of invalid request
        r = requests.put("http://localhost:8888/put_test/file_should_exist_after", headers=hd, data=payload)
        assertResponse(r, 201, index)
    
# ----------------------------------------------------------------------------- #16 - STAGE 2

    # POST /directoru/youpi.bla HTTP/1.1
    # Host: localhost:8080
    # User-Agent: Go-http-client/1.1
    # Transfer-Encoding: chunked
    # Content-Type: test/file
    # Accepted-Encoding: gzip

    # 8000 =     32 768 en HEX
    # 186A0 =   100 000 en HEX
    # F4240 = 1 000 000 en HEX

    # Also, in hexadecimal, 8000 is written : 38 30 30 30 (usefull for Wireshark)
    # In this test too, body is sent by chunk of 32 768 bytes or 8000 in hexa
    # File should exist after with a size of 100 000 000

    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/test42/file_should_exist_after"): os.remove("www/test42/file_should_exist_after")
        
        # TODO : Loop
        payload = "8000\r\n"
        payload += "n" * 32768
        payload += "\r\n"

        payload += "0\r\n\r\n"
        # The last \n seems to be send in another request ... idk why, may be to trool. It may raise an error of invalid request
        r = requests.put("http://localhost:8888/put_test/file_should_exist_after", headers=hd, data=payload)
        assertResponse(r, 201, index)


# -----------------------------------------------------------------------------
# ----------------------------------- MAIN ------------------------------------
# -----------------------------------------------------------------------------

run(sys)

# -----------------------------------------------------------------------------
# ------------------------------ WEBSERV CONFIG -------------------------------
# -----------------------------------------------------------------------------


# server {

# 	listen 	8888
# 	server_name     test42
# 	error   	    www/errors

# 	location / {
#         method      GET
# 		root 	    ./www/test42
#         index       index.html
#         ext			.bla
# 		cgi         ./www/cgi-bin/cgi_tester
# 	}

#     location /directory {
# 		method		GET,POST
# 		root 		./www/test42/YoupiBanane
# 		index 		youpi.bad_extension
# 		ext 		.bla
# 		cgi 		./www/cgi-bin/cgi_tester
# 	}

#     location /put_test {
#         method		PUT
#         root       ./www/test42/
#     }
    
#     location /post_body {
# 		methods 	POST
# 		root 		./www/test42
# 		index 		index.html
# 		max_body 	100
# 		ext			.bla

# 	}
# }