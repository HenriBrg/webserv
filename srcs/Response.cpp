# include "../inc/Webserv.hpp"

Response::Response(void) {
    reset();
}

void Response::reset() {
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
    finalResponse.clear();

    statusCode = -1;
    contentLength = -1;
}

Response::~Response() {

}

void Response::get(Request * req) {

    statusCode = OK_200;
    

}



void Response::handleResponse(Request * req) {

   if (req->method == "GET") {
       get(req);
   }
}


