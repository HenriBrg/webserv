#!/bin/sh

# TEST n°1 : CURL

if [ "$#" -eq 0 ]; then
    echo "Usage: sh runStressTests.sh <num test>"
    exit
fi

if [ "$1" -eq 1 ]; then 
    i=0;
    while (($i < 150)) ;do
        curl -I http://localhost:7777;
        i=$(($i+1));
    done
    exit
fi

# TEST n°2 : SIEGE

if [ "$1" -eq 2 ]; then 
    
    
    exit
fi




# ----------------

echo "Test n°$1 not found"
