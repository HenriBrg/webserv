import os
import sys
import requests
import logging
class bcolors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def assertResponse(r, code):
    if (r.status_code == code): info = bcolors.OKGREEN + "OK" + bcolors.ENDC + " - " + str(r.status_code) + " " + r.raw.reason
    else:                       info = bcolors.FAIL + "KO" + bcolors.ENDC + " - " + str(r.status_code) + " " + r.raw.reason + " - Should receive : " + str(code)
    url = "           • " + str(r.request.method) + " " + str(r.request.url)
    url = str(url).ljust(60, ' ')
    print(url + "   =   " + info)


# -----------------------------------------------------------------------------
# ------------------------------------ GET ------------------------------------
# -----------------------------------------------------------------------------

print("\n     ~ GET REQUESTS ------------------------> \n")

# ------- GET : 200
r = requests.get("http://localhost:7777")
assertResponse(r, 200)
r = requests.get("http://localhost:7777/index.html")
assertResponse(r, 200)
r = requests.get("http://localhost:7777/?a=1&b=2")
assertResponse(r, 200)
r = requests.get("http://localhost:7777/?a=z??a=1&b=2?")
assertResponse(r, 200)
r = requests.get("http://localhost:7777/php")
assertResponse(r, 200)
r = requests.get("http://localhost:7777/php/index.html")
assertResponse(r, 200)

# ------- GET : 400

# ------- GET : 403 

# r = requests.get("http://localhost:7777/..")
# assertResponse(r, 404)

# ------- GET : 404


r = requests.get("http://localhost:7777/inexting")
assertResponse(r, 404)

r = requests.get("http://localhost:7777/../webserv")
assertResponse(r, 404)

# - GET - 405



# - GET with CGI
# r = requests.get("http://localhost:7777/php/info.php", hd)
# assertResponse(r, 200)





# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

# print("\n     ~ POST REQUESTS ----------------------- ")



# -----------------------------------------------------------------------------


# body = "14\r\nabcdefghijklmnopqrst\r\nA\r\n0123456789\r\n0\r\n\r\n"
# hd = {'Transfer-Encoding': 'chunked'}
# r = requests.get("http://localhost:7777", data=body, headers=hd)

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


# -----------------------------------------------------------------------------

# def printReqRes(req):
#     # print (bcolors.WARNING, "\n    NEW REQUEST :", req.request.method, "|", uri, bcolors.ENDC)
#     print ("\n    Request : \n")
#     for h in req.request.headers:
#         print ("    > " + h + ":" + req.request.headers[h])
#     print ("\n    Response : \n")
#     print ("    Status :", req.status_code)
#     print ("    Reason :", req.raw.reason)
#     print ()
#     for h in req.headers:
#         print ("    > " + h + ":" + req.headers[h])
#     print ()
#     print ("    Body :", req.text)
#     print (bcolors.HEADER, "\n    ---------------------------\n", bcolors.ENDC)
