curl -iv "http://localhost:7777"
curl -iv "http://localhost:7777/?a=1&b=2" -H "Content-type: application/x-www-form-urlencoded"
curl -iv "http://localhost:7777/?a=1&b=2" -H "Accept-Language: da, en-gb;q=0.8, en;q=0.7" -H "Content-Type: text/html; charset=utf-8"


# Arrays (mapped to std::map) : 

# acceptCharset
# acceptLanguage
# contentLanguage
# transferEncoding

# Strings :

# authorization
# contentLength
# contentType
# date
# host
# referer
# userAgent
# keepAlive