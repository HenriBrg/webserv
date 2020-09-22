import sys
import requests
from socket import *

def http_req(server, path):

    # Creating a socket to connect and read from
    s=socket(AF_INET,SOCK_STREAM)
    # Finding server address
    adr=(gethostbyname(server), 3000)
    # # Connecting to server
    s.connect(adr)

    # # Sending request
    s.send("GET "+path+" HTTP/1.0\r\n\r\n")

    # Printing response
    # resp=s.recv(1024)
    # while resp!="":
	# print resp
	# resp=s.recv(1024)

http_req(sys.argv[1], sys.argv[2])
