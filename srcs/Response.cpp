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

void Response::get(Request * req) {

    // Pour l'instant on va au plus simple
    char res[41] = "Hello dear Client ! Welcome to WEBSERV \n";
    write(req->client->acceptFd, res, sizeof(res));
    return ;

}



void Response::handleResponse(Request * req) {

   if (req->method == "GET") {
       get(req);
   }
}


