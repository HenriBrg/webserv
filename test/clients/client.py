import requests
import sys
import os

# RUN : python3 get.py

class bcolors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def getReq(uri, hd):
    req = requests.get(uri, headers=hd)
    print (bcolors.WARNING, "\n    NEW REQUEST :", req.request.method, "|", uri, bcolors.ENDC)
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

# -------------------------------

hd = {}
getReq('http://localhost:8888', hd)

