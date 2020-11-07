# Tester dedicated to the 42 project webserv.
# 07/11/2020

# ---------------------------------------------------------------------------
# ---------------------------- 1. LIBS --------------------------------------
# ---------------------------------------------------------------------------

import os
import sys
import json
import math
import requests
from sys import platform
from requests.auth import HTTPBasicAuth

import time
import random
from threading import Thread

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
    RES_BODY_SIZE_ASSERT = 4

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

def resBodySize(r, size):
    if (r.text and str(len(r.text)) >= size and str(r.headers["Content-Length"]) == size): return True
    else: return False

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
    if (assertTypes.RES_BODY_SIZE_ASSERT in assertLevel and resBodySize(r, args[0][indexArgsUsed]) == True ):
        ret = True
        indexArgsUsed += 1

    return ret

def assertResponse(r, threadID, code, index, assertLevel = [], *args):
    ret = True
    # if (len(assertLevel)): ret = moreAsserts(r, assertLevel, args)
    # else: ret = True
    if (ret and r.status_code == code):
        info = bcolors.OKGREEN + "OK" + bcolors.ENDC + " - " + str(r.status_code)
    else:
        info = bcolors.FAIL + "KO" + bcolors.ENDC + " - " + str(r.status_code) + " - Should have been received : " + str(code)
    url = "           • TID : " + str(threadID) + " - #" + str(index).ljust(2, ' ') + " : " + str(r.request.method) + " "
    if (len(r.request.url) > 60):
        url += r.request.url[16:60] + " [..." + str(len(r.request.url)) + "]"
    else: url += str(r.request.url)[16:]
    url = str(url).ljust(80, ' ')
    
    bodyInfo = "REQ | RES BODY = "
    if r.request.body:
        bodyInfo += (str(len(r.request.body)) + " | ")
    else:
        bodyInfo += "Empty | "
  
    if r.text:
        bodyInfo += str(len(r.text))
    else:
        bodyInfo += "Empty"
    print(url + "   =   " + info + "    ==>    " + bodyInfo)
    if verbose == 1:
        printHdReqRes(r)

def run(sys):

    print("\n       Platform = " + platform)
    if (len(sys.argv) == 1):
        TESTS_42()
    elif (len(sys.argv) >= 2):
            TESTS_42(sys.argv[1])
    print()

# ---------------------------------------------------------------------------
# ---------------------------- 3. TRHEADS -----------------------------------
# ---------------------------------------------------------------------------

class Client(Thread):
	def __init__(self, threadID, method, uri, payloadSizeToSend, repeat):
	    Thread.__init__(self)
	    self.threadID = threadID
	    self.method = method
	    self.uri = uri
	    self.payloadSizeToSend = payloadSizeToSend
	    self.repeat = repeat

	def run(self):
        hd = {}
        if (payloadSizeToSend != -1):
            payload = "x" * payloadSizeToSend
            payload += "0\r\n\r\n"
        else:
            payload = ""

        if (self.method == "GET"):
            hd = {
                "Host": "localhost:8080",
                "User-Agent": "Go-http-client/1.1",
                "Accept-Encoding": "gzip"
            }
        elif (self.method == "PUT"):
            hd = {
                "Host": "localhost:8080",
                "User-Agent": "Go-http-client/1.1",
                "Accept-Encoding": "gzip"
            }
        elif (self.method == "POST"):
            hd = {
                "Host": "localhost:8080",
                "User-Agent": "Go-http-client/1.1",
                "Accept-Encoding": "gzip"
            }

        # À voir
		# headers += {
		# 	"Connection" : "Keep-Alive",
		#   "Keep-Alive" : "timeout=10, max=10"
		# }
        
        index = 1
		max = repeat
		for x in range(max):
			try:
                if (self.method == "GET"):
				    r = requests.get(uri, headers=hd, payload=payload)
                    assertResponse(r, 200, index)
                    index += 1
                elif (self.method == "PUT"):
				    r = requests.put(uri, headers=hd, payload=payload)
                    assertResponse(r, 200, index)
                    index += 1
                elif (self.method == "POST"):
				    r = requests.post(uri, headers=hd, payload=payload)
                    assertResponse(r, 200, index)
                    index += 1
			except:
				print("An exception has been raised")


# Pour informations :

    # Les GET :
    
        # GET / HTTP/1.1
        # Host: localhost:8080
        # User-Agent: Go-http-client/1.1
        # Accept-Encoding: gzip

    # Les PUT :

    # Les POST :


# -----------------------------------------------------------------------------
# -------------------------------- 4. TESTS -----------------------------------
# -----------------------------------------------------------------------------

def TESTS_42(testNum = 0):

    index = 0
    print("\n     ~ 42 TESTS ------------------------> \n")
    # -----------------------------------------------------------------------------> #1 - STAGE 1

    # 1. Création
    threads = []
    name = "Thread n°"
    for x in range(5):
        threads.append(Client((name + str(x)), "GET", "http://localhost:8888", -1, 15))
    # 2. Lancement
    for x in threads:
        x.start()
    for x in threads:
        x.join()


run(sys)