# include "../inc/Webserv.hpp"

Request::Request(void) {

    method.clear();
    uri.clear();
    httpVersion.clear();
    acceptCharset.clear();
    acceptLanguage.clear();
    authorization.clear();
    contentLanguage.clear();
    contentLength = -1;
    contentLocation.clear();
    contentType.clear();
    date.clear();
    host.clear();
    referer.clear();
    userAgent.clear();
    body.clear();
    transferEncoding.clear();
    bodyLength = -1;
}

Request::~Request() {

}

// Pour le moment, on suppoesera un requête GET très basique : 
// GET /index.html HTTP/1.0

void Request::parse(std::vector<Location*> locations) {
    
    std::vector<std::string> requestLine;

    // requestLine = split()

}
