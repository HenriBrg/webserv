# include "../inc/Webserv.hpp"

Response::Response(void) {
    reset();
}

void Response::reset() {
    httpVersion.clear();
    reason.clear();
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
    sendStatus = Response::PREPARE;
    _methodFctPtr = nullptr;

}

Response::~Response() {
    reset();
}

void Response::authControl(Request * req) {

    /* Base 64 Deal ... */

}

void Response::methodControl(Request * req) {

    /* TODO : Avoid doing function mapping for each requests
    store it in servers instead */

    typedef void (Response::*ptr)(Request * req);
	std::map<std::string, ptr> tab;
    std::vector<std::string>    allowedMethods;
    std::vector<std::string>::iterator    tmp;

    tab["GET"] = &Response::getReq;
	tab["HEAD"] = &Response::headReq;
	tab["PUT"] = &Response::putReq;
	tab["POST"] = &Response::postReq;
	tab["CONNECT"] = &Response::connectReq;
	tab["TRACE"] = &Response::traceReq;
	tab["OPTIONS"] = &Response::optionsReq;
	tab["DELETE"] = &Response::deleteReq;

    allowedMethods = ft::split(req->reqLocation->methods, ',');
    tmp = std::find(allowedMethods.begin(), allowedMethods.end(), req->method);
    if (tmp == allowedMethods.end()) {
        LOGPRINT(LOGERROR, this, ("Response::methodControl() : Method " + req->method + " is not allowed on route " + req->reqLocation->uri));
        sendStatus = Response::ERROR;
        statusCode = METHOD_NOT_ALLOWED_405;
    } else
        _methodFctPtr = tab[req->method];
    tab.clear();
}

void Response::resDispatch(Request * req) {

    methodControl(req);
    authControl(req);
    /* Errors Checking Here */
    if (sendStatus == Response::ERROR) {
        LOGPRINT(LOGERROR, this, ("Response::resDispatch() : Error raised : " + std::to_string(statusCode) + " | Ending client connection"));
        return ;
    } else {
        (this->*_methodFctPtr)(req);
    }
    /* New Check - After CGI step */
    if (sendStatus == Response::ERROR) {

    }


}

std::string const Response::logInfo(void) {
    std::string ret;
    ret = "Response | To Client with Socket : " + std::to_string(resClient->acceptFd) + " | Method : " + resClient->req.method + " | Response Status : " + std::to_string(sendStatus);
    return (ret);
}