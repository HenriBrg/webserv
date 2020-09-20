# include "../inc/Webserv.hpp"

Response::Response(void) {

    httpVersion.clear();
    signification.clear();
    allowedMethods.clear();
    contentLanguage.clear();
    contentLocation.clear();
    contentType.clear();
    lastModified.clear();
    location.clear();
    date.clear();
    retryAfter.clear();
    server.clear();
    transfertEncoding.clear();
    wwwAuthenticate.clear();
    statusCode = -1;
    contentLength = -1;
}

Response::~Response() {

}