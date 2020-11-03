# Tester dedicated to the 42 project webserv.
# 01/11/2020
# Feel free to use it, just be sure that configuration file is properly set

# Usage : runTests.py [METHOD] [TEST NUM] [VERBOSE]
# python3 runTests.py
# python3 runTests.py GET
# python3 runTests.py PUT
# python3 runTests.py GET 2
# python3 runTests.py GET 7
# python3 runTests.py GET 7 -v

import os
import sys
import json
import requests

from requests.auth import HTTPBasicAuth
from sys import platform

global verbose
verbose = 0
if (len(sys.argv) == 4 and sys.argv[3] == '-v'):
    verbose = 1

if (os.system('lsof -c webserv > /dev/null') != 0):
    print("Webserv is not running")
    exit()
if (os.system('type php-cgi > /dev/null') != 0):
    print("The executable php-cgi is required to run theses tests.\nBe sure that the php-cgi path inside the configuration file is correct")
    exit()

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


# ------ ASSERTS FUNCTIONS ------

# r.request.body
# r.request.headers
# res.headers
# res.text

def bodyContain(r, str):
    if (str in r.text): return True
    else: return False

def fileContain(r, strLookingFor, path):
    file = open(path, 'r').read()
    if (strLookingFor == file): return True
    else: return False

# print(r.headers["Content-Length"])
# print(resKey + " == " + wantKey)
# print(hdValTab[idB])
# print(r.headers[resKey])
def resHeadersKeyVal(r, hdKeyTab = [], hdValTab = []):
    validated = 0
    for idA, resKey in enumerate(r.headers):
        for idB, wantKey in enumerate(hdKeyTab):
            if (resKey == wantKey):
                # print(resKey + " == " + wantKey)
                # print(r.headers[resKey] + " == " + hdValTab[idB])
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
    if (len(r.request.url) > 50):
        url += r.request.url[16:50] + " [..." + str(len(r.request.url)) + "]"
    else: url += str(r.request.url)[16:]
    url = str(url).ljust(70, ' ')
    print(url + "   =   " + info)
    
    if verbose == 1:
        printHdReqRes(r)

# -----------------------------------------------------------------------------
# ------------------------------------ GET ------------------------------------
# -----------------------------------------------------------------------------

def GET_TESTS(testNum = 0):

    index = 0
    print("\n     ~ GET REQUESTS ------------------------> \n")
    
    # ------- GET : 200 (without CGI)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/index.html")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/?a=1&b=2")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/?a=z??a=1&b=2?")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/php")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/php/index.html")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/ftcgi/index.html")
        assertResponse(r, 200, index)

    # ------- GET : 200 (with CGI)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/index.bla")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/ftcgi/index.bla")
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/php/uriQueries.php?user=bob")
        assertResponse(r, 200, index, [assertTypes.BODY_CONTAIN_ASSERT], "bob")
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/php/info.php")
        assertResponse(r, 200, index)

    # ------- GET : Authentification (root:pass = cm9vdDpwYXNz)

    index += 1
    if (testNum == 0 or index == int(testNum)):
        # hd = {"Authorization": "Basic cm9vdDpwYXNz"} --> Test without the Authorization header
        r = requests.get("http://localhost:7777/auth", headers={})
        assertResponse(r, 401, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {"Authorization": "Basic cm9vdDpwYXNz"}
        r = requests.get("http://localhost:7777/auth/index.html", headers=hd)
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {"Authorization": "Digest cm9vdDpwYXNz"} 
        r = requests.get("http://localhost:7777/auth", headers=hd)
        assertResponse(r, 401, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        # hd = {"Authorization": "Basic cm9vdDpwYXNz"} 
        r = requests.get("http://localhost:7777/auth", auth=HTTPBasicAuth('root', 'pass'), headers={})
        assertResponse(r, 200, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {"Authorization": "Basic zagGFDSsdfjAC0"}
        r = requests.get("http://localhost:7777/auth", headers=hd)
        assertResponse(r, 401, index)



    # ------- GET : Negotiation

    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {"Accept-Language": "fr"}
        r = requests.get("http://localhost:7777/nego", headers=hd)
        assertResponse(r, 200, index, [assertTypes.BODY_CONTAIN_ASSERT], "FRANCE")
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {"Accept-Language": "en"}
        r = requests.get("http://localhost:7777/nego", headers=hd)
        assertResponse(r, 200, index, [assertTypes.BODY_CONTAIN_ASSERT], "ENGLAND")
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {"Accept-Language": "de"}
        r = requests.get("http://localhost:7777/nego", headers=hd)
        assertResponse(r, 200, index, [assertTypes.BODY_CONTAIN_ASSERT], "NO_LANGUAGE_NEGOTIATED")

    # ------- GET : 400
    
    # ---> Working but Python doesn't sendd the HOST header if it is empty or full of space
    # ---> Test with CURL
    index += 1
    if (testNum == 0 or index == int(testNum)):
        hd = {"Host": ''}
        r = requests.get("http://localhost:7777/", headers=hd)
        assertResponse(r, 400, index)

    # ------- GET : 404
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/inexisting")
        assertResponse(r, 404, index)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/...")
        assertResponse(r, 404, index)
        
    #  ------- GET - 414 - REQUEST_URI_TOO_LONG_414 (> 1024)
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/?queries=zyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcba")
        assertResponse(r, 414, index)


    # TO TEST : AUTOINDEX

# -----------------------------------------------------------------------------
# ----------------------------------- POST ------------------------------------
# -----------------------------------------------------------------------------

# ATTENTION : Run indépendament des autres les POST peut en faire rater certains

def POST_TESTS(testNum = 0):

    index = 0

    print("\n     ~ POST REQUESTS -----------------------> \n")

    # ------- POST - 200/2001 - NO CGI
    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/newFile"): os.remove("www/newFile")
        payload = "Hello ! I am a new file"
        r = requests.post('http://localhost:7777/newFile', data=payload, headers={})
        assertResponse(r, 201, index, [assertTypes.FILE_CONTAIN_ASSERT], "Hello ! I am a new file", "www/newFile")
    index += 1
    if (testNum == 0 or index == int(testNum)):
        payload = ". I have been updated !"
        r = requests.post('http://localhost:7777/newFile', data=payload, headers={})
        assertResponse(r, 200, index, [assertTypes.FILE_CONTAIN_ASSERT], "Hello ! I am a new file. I have been updated !", "www/newFile")
    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/newFile"): os.remove("www/newFile")
        payload = {'hello': 'world'}
        hd = {"Content-Type": "application/json"}
        r = requests.post('http://localhost:7777/newFile',  data=json.dumps(payload), headers=hd)
        assertResponse(r, 201, index, [assertTypes.FILE_CONTAIN_ASSERT], "{\"hello\": \"world\"}", "www/newFile")
    
    # ------- POST CHUNKED - 200/201 - NO CGI
    index += 1
    if (platform == "darwin" and (testNum == 0 or index == int(testNum))):
        if os.path.exists("www/newFile"): os.remove("www/newFile")
        payload = "14\r\nabcdefghijklmnopqrst\r\nA\r\n0123456789\r\n0\r\n\r\n"
        hd = {'Transfer-Encoding': 'chunked'}
        r = requests.post("http://localhost:7777/newFile", data=payload, headers=hd)
        assertResponse(r, 201, index, [assertTypes.FILE_CONTAIN_ASSERT], "abcdefghijklmnopqrst0123456789", "www/newFile")
    index += 1
    if (testNum == 0 or index == int(testNum)):
        payload = "14\r\nabcdefghijklmnopqrst\r\nA\r\n0123456789\r\n0\r\n\r\n"
        hd = {'Transfer-Encoding': 'chunked'}
        r = requests.post("http://localhost:7777/newFile", data=payload, headers=hd)
        assertResponse(r, 200, index, [assertTypes.FILE_CONTAIN_ASSERT], "abcdefghijklmnopqrst0123456789abcdefghijklmnopqrst0123456789", "www/newFile")


    # ------- POST - 200/2001 - 42 CGI
    index += 1
    if (testNum == 0 or index == int(testNum)):
        payload = "I am a payload"
        r = requests.post('http://localhost:7777/ftcgi/index.bla', data=payload, headers={})
        assertResponse(r, 200, index, [assertTypes.BODY_CONTAIN_ASSERT], "I AM A PAYLOAD")

    # FREEZE ERROR
    
    # ------- POST IMGAGE CHUNKED - 201
    # index += 1
    # if (testNum == 0 or index == int(testNum)):
    #     if os.path.exists("www/42.png"): os.remove("www/42.png")
    #     img = {'file': open('test/payloads/42.png', 'rb')}
    #     hd = {"Content-Type": "image/png", "Transfer-Encoding": "chunked"}
    #     r = requests.post('http://localhost:7777/youpi/pouet/42.png', files=img,  headers=hd)
    #     assertResponse(r, 201, index)



    # ------- POST - REQUEST_ENTITY_TOO_LARGE_413

    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/newFile"): os.remove("www/newFile")
        hd = { "Content-Type": "text/plain" }
        body = "abcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyz"
        r = requests.post("http://localhost:7777/newFile", body, headers=hd)
        assertResponse(r, 413, index)

# -----------------------------------------------------------------------------
# ----------------------------------- HEAD ------------------------------------
# -----------------------------------------------------------------------------

def HEAD_TESTS(testNum = 0):

    index = 0
    print("\n     ~ HEAD REQUESTS -----------------------> \n")
    
    # ------- HEAD - 200
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.head("http://localhost:7777/")
        assertResponse(r, 200, index, [assertTypes.RES_HD_CONTAIN_ASSERT], ["Content-Length"], ["860"])

    # ------- HEAD - 405
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.head("http://localhost:7777/ftcgi")
        assertResponse(r, 405, index, [assertTypes.RES_HD_CONTAIN_ASSERT], ["Allow"], ["GET,POST"])

# -----------------------------------------------------------------------------
# ----------------------------------- PUT -------------------------------------
# -----------------------------------------------------------------------------

def PUT_TESTS(testNum = 0):
    print("\n     ~ PUT REQUESTS -----------------------> \n")


# -----------------------------------------------------------------------------
# ---------------------------------- DELETE -----------------------------------
# -----------------------------------------------------------------------------

def DELETE_TESTS(testNum = 0):
    print("\n     ~ DELETE REQUESTS -----------------------> \n")


# -----------------------------------------------------------------------------
# ----------------------------------- MAIN ------------------------------------
# -----------------------------------------------------------------------------


print("\n       Platform = " + platform)

if (len(sys.argv) == 1):
    GET_TESTS()
    POST_TESTS()
    PUT_TESTS
    DELETE_TESTS
    HEAD_TESTS
elif (len(sys.argv) == 2):
    if (sys.argv[1] == "GET"):      GET_TESTS()
    elif (sys.argv[1] == "HEAD"):   HEAD_TESTS()
    elif (sys.argv[1] == "POST"):   POST_TESTS()
    elif (sys.argv[1] == "PUT"):    PUT_TESTS()
    elif (sys.argv[1] == "DELETE"): DELETE_TESTS()
elif (len(sys.argv) >= 3):
    if (sys.argv[1] == "GET"):      GET_TESTS(sys.argv[2])
    elif (sys.argv[1] == "HEAD"):   HEAD_TESTS(sys.argv[2])
    elif (sys.argv[1] == "POST"):   POST_TESTS(sys.argv[2])
    elif (sys.argv[1] == "PUT"):    PUT_TESTS(sys.argv[2])
    elif (sys.argv[1] == "DELETE"): DELETE_TESTS(sys.argv[2])

print()