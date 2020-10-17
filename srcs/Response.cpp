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
    _resBody.clear();

    _statusCode = -1;
    contentLength = -1;
    _sendStatus = Response::PREPARE;
    _errorFileName.clear();
    _methodFctPtr = nullptr;

}

Response::~Response() {
    reset();
}

/* https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Authorization */

void Response::authControl(Request * req) {
    
    std::vector<std::string> tab;

    if (!req->authorization.empty()) {

        tab = ft::split(req->authorization, ' ');
        if (tab.size() < 2)
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Incomplete www-authenticate header"));
        if (tab[0] != "Basic" && tab[1] != "BASIC")
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Unknow www-authenticate encoding"));
        if (ft::decodeBase64(tab[1]) != req->authorization) {
            _sendStatus = Response::ERROR;
            _statusCode = UNAUTHORIZED_401;
            _errorFileName = "error.html"; /* TO UPDATE when we will have one html file per error */
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Failed authentification"));
            return ;
        } else
            LOGPRINT(INFO, this, ("Response::authControl() : Successfull authentification"));
    }

}

void Response::methodControl(Request * req) {

    /* TODO : Avoid doing function mapping for each requests
    store it in servers instead */

    typedef void (Response::*ptr)(Request * req);
	std::map<std::string, ptr> tab;
    std::vector<std::string>    allowedMethods;
    std::vector<std::string>::iterator    tmp;

    tab["GET"]     = & Response::getReq;
	tab["PUT"]     = & Response::putReq;
	tab["POST"]    = & Response::postReq;
	tab["HEAD"]    = & Response::headReq;
	tab["TRACE"]   = & Response::traceReq;
	tab["DELETE"]  = & Response::deleteReq;
	tab["CONNECT"] = & Response::connectReq;
	tab["OPTIONS"] = & Response::optionsReq; 

    allowedMethods = ft::split(req->reqLocation->methods, ',');
    tmp = std::find(allowedMethods.begin(), allowedMethods.end(), req->method);
    if (tmp == allowedMethods.end()) {
        setErrorParameters(req, Response::ERROR, METHOD_NOT_ALLOWED_405);
        LOGPRINT(LOGERROR, this, ("Response::methodControl() : Method " + req->method + " is not allowed on route " + req->reqLocation->uri));
    } else
        _methodFctPtr = tab[req->method];
    tab.clear();
}

void Response::resDispatch(Request * req) {

    methodControl(req);
    authControl(req);
    // ... TODO : Additionnal controls
    (this->*_methodFctPtr)(req);
    if (_sendStatus == Response::ERROR) {
        _errorFileName = "error.html"; /* TO UPDATE when we will have one html file per error */
        return ;
    }

}

void Response::resBuild(Request * req) {
    
    std::map<int, std::string> reasonMap;
    
    reasonMap[200] = "OK";
    reasonMap[201] = "Created";
    reasonMap[202] = "Accepted";
    reasonMap[204] = "No Content";
    reasonMap[305] = "Use Proxy";
    reasonMap[400] = "Bad Request";
    reasonMap[401] = "Unauthorized";
    reasonMap[404] = "Not Found";
    reasonMap[405] = "Method Not Allowed";
    reasonMap[413] = "Request Entity Too Large";
    reasonMap[414] = "Request-URI Too Long";
    reasonMap[495] = "SSL Certificate Error";
    reasonMap[500] = "Internal Server Error";
    reasonMap[501] = "Not Implemented";
    reasonMap[503] = "Service Unavailable";

    httpVersion = "HTTP/1.1";
    reason = reasonMap[_statusCode];
    server = "webserv-42";
    date = ft::getDate();

    if (!_resBody.empty())
        contentLength = _resBody.size();



}

/* Body */

void Response::addBody(Request * req) {

    // TODO LATER

}


/* Errors Settings */

void Response::setErrorParameters(Request * req, int sendStatus, int code) {

    _sendStatus = sendStatus;
    _statusCode = code;
    _errorFileName = "error.html";

}

/* Utils */

std::string const Response::logInfo(void) {
    std::string ret;
    ret = "Response | To Client with Socket : " + std::to_string(resClient->acceptFd) + " | Method : " + resClient->req.method + " | Response Status : " + std::to_string(_sendStatus);
    return (ret);
}