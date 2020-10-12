# ---------------- GET ----------------

# curl -iv "http://localhost:7777"
# curl -iv "http://localhost:7777/?a=1&b=2" -H "Content-type: application/x-www-form-urlencoded"
# curl -iv "http://localhost:7777/?a=1&b=2" -H "Accept-Language: da, en-gb;q=0.8, en;q=0.7" \
#                                           -H "Content-Type: text/html; charset=utf-8" \
#                                           -H "Accept-Charset: utf-8, iso-8859-1;q=0.5" \
#                                           -H "Content-Type: application/x-www-form-urlencoded" \
#                                           -H "Transfer-Encoding: gzip, chunked" \
#                                           -H "Authorization: Basic YWxhZGRpbjpvcGVuc2VzYW1l" \
#                                           -H "Date: Wed, 21 Oct 2015 07:28:00 GMT" \
#                                           -H "Host: developer.cdn.mozilla.net" \
#                                           -H "Referer: https://developer.mozilla.org/fr/docs/Web/JavaScript" \
#                                           -H "User-Agent: PostmanRuntime/7.26.5" \

# 1/2/3 OK

# ---------------- POST ----------------

curl -ivd @payload http://localhost:7777
