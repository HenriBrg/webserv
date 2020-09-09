# Simple Python function that issues an HTTP request
# http://www.garshol.priv.no/download/text/http-tut.html

from socket import *

def http_req(server, path):

    # Creating a socket to connect and read from
    s=socket(AF_INET,SOCK_STREAM)

    # Finding server address (assuming port 80)
    adr=(gethostbyname(server),80)

    # Connecting to server
    s.connect(adr)

    # Sending request
    s.send("GET "+path+" HTTP/1.0\n\n")

    # Printing response
    resp=s.recv(1024)
    while resp!="":
	print resp
	resp=s.recv(1024)
