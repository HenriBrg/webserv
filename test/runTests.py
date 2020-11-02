import os
import sys
import requests
import json

# r.request.body
# r.request.headers
# res.headers
# res.text

class assertTypes:     
    BODY_CONTAIN_ASSERT = 1
    FILE_CONTAIN_ASSERT = 2
    
class bcolors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

# ------ ASSERTS FUNCTIONS ------

def bodyContain(r, str):
    if (str in r.text): return True
    else: return False

# print('File = ', file)
# print('STR =', strLookingFor)
def fileContain(r, strLookingFor, path):
    file = open(path, 'r').read()
    if (strLookingFor == file): return True
    else: return False

def moreAsserts(r, assertLevel, *args):
    indexArgsUsed = 0
    ret = False
    if (assertTypes.BODY_CONTAIN_ASSERT in assertLevel and bodyContain(r, args[0][indexArgsUsed]) == True):
        ret = True
        indexArgsUsed += 1
    if (assertTypes.FILE_CONTAIN_ASSERT in assertLevel and fileContain(r, args[0][indexArgsUsed], args[0][indexArgsUsed + 1]) == True):
        ret = True
        indexArgsUsed += 2 # str AND File Path so we increment by two the var-args tab
    return ret

def assertResponse(r, code, index, assertLevel = [], *args):

    ret = False
    if (len(assertLevel)):
        ret = moreAsserts(r, assertLevel, args)
    else:
        ret = True

    if (ret and r.status_code == code):
        info = bcolors.OKGREEN + "OK" + bcolors.ENDC + " - " + str(r.status_code) + " " + r.raw.reason
    else:
        info = bcolors.FAIL + "KO" + bcolors.ENDC + " - " + str(r.status_code) + " " + r.raw.reason + " - Received : " + str(code)
    url = "           • #" + str(index) + " : " + str(r.request.method) + " " + str(r.request.url)[16:]
    url = str(url).ljust(60, ' ')
    print(url + "   =   " + info)

# -----------------------------------------------------------------------------
# ------------------------------------ GET ------------------------------------
# -----------------------------------------------------------------------------

def GET_TESTS(testNum = 0):

    index = 0


    print("\n     ~ GET REQUESTS ------------------------> \n")
    # ------- GET : 200 without CGI
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

    # ------- GET : 200 - Request calling the CGI (ALL type of CGIs)
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/ftcgi/index.bla")
        assertResponse(r, 200, index)
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/php/uriQueries.php?user=bob")
        assertResponse(r, 200, index, [assertTypes.BODY_CONTAIN_ASSERT], "bob")

    # ------- GET : 400

    
    # ------- GET : 403 
    # r = requests.get("http://localhost:7777/..")
    # assertResponse(r, 404)
    # ------- GET : 404
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/inexting")
        assertResponse(r, 404, index)
    # ------- GET : 404
    index += 1
    if (testNum == 0 or index == int(testNum)):
        r = requests.get("http://localhost:7777/../webserv")
        assertResponse(r, 404, index)
    # ------- GET - 405

    # ------- GET with CGI
    # r = requests.get("http://localhost:7777/php/info.php", hd)
    # assertResponse(r, 200)

    #  ------- GET - 414 - REQUEST_URI_TOO_LONG_414 ---> OK - Just too long go be printed (> 1024)
    # index += 1
    # if (testNum == 0 or index == int(testNum)):
    #     r = requests.get("http://localhost:7777/?queries=zyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcbazyxwvutsrqponmlkjihgfedcba")
    #     assertResponse(r, 414, index)

# -----------------------------------------------------------------------------
# ----------------------------------- POST ------------------------------------
# -----------------------------------------------------------------------------

def POST_TESTS(testNum = 0):

    index = 0

    print("\n     ~ POST REQUESTS -----------------------> \n")

    # ------- POST - 201
    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/newFile"): os.remove("www/newFile")
        payload = "Hello ! I am a new file"
        r = requests.post('http://localhost:7777/newFile', data=payload, headers={})
        assertResponse(r, 201, index, [assertTypes.FILE_CONTAIN_ASSERT], "Hello ! I am a new file", "www/newFile")

    # ------- POST - 200
    index += 1
    if (testNum == 0 or index == int(testNum)):
        payload = ". I have been updated !"
        r = requests.post('http://localhost:7777/newFile', data=payload, headers={})
        assertResponse(r, 200, index, [assertTypes.FILE_CONTAIN_ASSERT], "Hello ! I am a new file. I have been updated !", "www/newFile")
    
    # ------- POST JSON - 201
    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/newFile"): os.remove("www/newFile")
        payload = {'hello': 'world'}
        hd = {"Content-Type": "application/json"}
        r = requests.post('http://localhost:7777/newFile',  data=json.dumps(payload), headers=hd)
        assertResponse(r, 201, index, [assertTypes.FILE_CONTAIN_ASSERT], "{\"hello\": \"world\"}", "www/newFile")
    
    # ------- POST CHUNKED - 201
    index += 1
    if (testNum == 0 or index == int(testNum)):
        if os.path.exists("www/newFile"): os.remove("www/newFile")
        payload = "14\r\nabcdefghijklmnopqrst\r\nA\r\n0123456789\r\n0\r\n\r\n"
        hd = {'Transfer-Encoding': 'chunked'}
        r = requests.post("http://localhost:7777/newFile", data=payload, headers=hd)
        assertResponse(r, 201, index, [assertTypes.FILE_CONTAIN_ASSERT], "abcdefghijklmnopqrst0123456789", "www/newFile")

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
        body = "abcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyz"
        r = requests.post("http://localhost:7777/newFile", body, headers=hd)
        assertResponse(r, 413, index)

# -----------------------------------------------------------------------------
# ----------------------------------- HEAD ------------------------------------
# -----------------------------------------------------------------------------

def HEAD_TESTS(testNum = 0):
    print("\n     ~ HEAD REQUESTS -----------------------> \n")


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
elif (len(sys.argv) == 3):
    if (sys.argv[1] == "GET"):      GET_TESTS(sys.argv[2])
    elif (sys.argv[1] == "HEAD"):   HEAD_TESTS(sys.argv[2])
    elif (sys.argv[1] == "POST"):   POST_TESTS(sys.argv[2])
    elif (sys.argv[1] == "PUT"):    PUT_TESTS(sys.argv[2])
    elif (sys.argv[1] == "DELETE"): DELETE_TESTS(sys.argv[2])

print()