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
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Failed authentification"));
            sendStatus = Response::ERROR;
            statusCode = UNAUTHORIZED_401;
            _errorFileName = "error.html"; /* TO UPDATE when we will have one html file per error */
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
        _errorFileName = "error.html"; /* TO UPDATE when we will have one html file per error */
    } else
        _methodFctPtr = tab[req->method];
    tab.clear();
}

void Response::resDispatch(Request * req) {

    methodControl(req);
    authControl(req);
    // ... TODO : Additionnal controls
    (this->*_methodFctPtr)(req);
    if (sendStatus == Response::ERROR) {
        _errorFileName = "error.html"; /* TO UPDATE when we will have one html file per error */
        return ;
    }


}

std::string const Response::logInfo(void) {
    std::string ret;
    ret = "Response | To Client with Socket : " + std::to_string(resClient->acceptFd) + " | Method : " + resClient->req.method + " | Response Status : " + std::to_string(sendStatus);
    return (ret);
}