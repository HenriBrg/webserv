#!/bin/sh

END='\e[0m'
RED='\e[31m'
GREEN='\e[32m'

if [ "$?" -ne 0 ]; then
    echo "Webserv isn't running"
    exit
fi
if [ "$#" -eq 0 ]; then
    echo "Usage: sh runStressTests.sh <num test>"
    exit
fi
if [ ! -f "webserv" ]; then
	make run
fi
echo
echo

# TEST n°1 : CURL

if [ "$1" -eq 1 ]; then 
    printf "  Test 1) Running 250 GET requests to http://localhost:8888\n"
    i=0;
    while (($i < 250)) ;do
        curl -s http://localhost:8888 > /dev/null &
        i=$(($i+1));
    done
    printf "  Test 1) ${GREEN}250 HEAD requests to http://localhost:8888 done${END}\n"
    echo
    exit
fi

# ----------------

echo "Test n°$1 not found"
echo
