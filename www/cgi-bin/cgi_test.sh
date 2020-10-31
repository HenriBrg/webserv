#! /bin/sh

# For POST ONLY

echo 'Content-type: text/html'
echo
echo "QS=$QUERY_STRING"
read DATA
echo "Data=$DATA"