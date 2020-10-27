import os
import sys
import requests

# RUN : python3 client.py

class bcolors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

# -----------------------------------------------------------------------------

def printReqRes(req):

    # print (bcolors.WARNING, "\n    NEW REQUEST :", req.request.method, "|", uri, bcolors.ENDC)

    print ("\n    Request : \n")
    for h in req.request.headers:
        print ("    > " + h + ":" + req.request.headers[h])

    print ("\n    Response : \n")
    print ("    Status :", req.status_code)
    print ("    Reason :", req.raw.reason)
    print ()
    
    for h in req.headers:
        print ("    > " + h + ":" + req.headers[h])

    print ()
    print ("    Body :", req.text)
    print (bcolors.HEADER, "\n    ---------------------------\n", bcolors.ENDC)

# GET 1

# hd = {}
# r = requests.get("http://localhost:7777", hd)
# printReqRes(r)

# hd = {"Accept-Language": "fr", "Accept-Charset": "utf8"}
# r = requests.get("http://localhost:7777/?a=1&b=2", hd)
# printReqRes(r)

# hd = {"Accept-Language": "fr", "Accept-Charset": "utf8", "Content-Type": "application/x-www-form-urlencoded"}
# r = requests.get("http://localhost:7777", hd)
# printReqRes(r)


body = "14\r\nabcdefghijklmnopqrst\r\nA\r\n0123456789\r\n0\r\n\r\n"
hd = {'Transfer-Encoding': 'chunked'}
r = requests.get("http://localhost:7777", data=body, headers=hd)


# -----------------------------------------------------------------------------

# POST 1

# hd = { "Content-Type": "text/plain" }
# body = "abcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyzabcdefghijklmnopqrstuvwyz"
# res = requests.post("http://localhost:8082/text", body, headers=hd)
# print ()
# print ("Status code:", res.status_code)
# print ("Reason:", res.raw.reason)
# print ()

# print (res.request.method, "on", res.request.url)
# print ()
# for h in res.request.headers:
#     print (h + ": " + res.request.headers[h])
# print ()
# print ("Body:", res.request.body)

# print ()
# print ()
# print ()

# for h in res.headers:
# 	print (h + ": " + res.headers[h])
# 	print ()
# 	print ("Body:", res.text)


