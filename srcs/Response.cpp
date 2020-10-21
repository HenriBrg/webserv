# include "../inc/Webserv.hpp"

Response::Response(void) {
    reset();
}

void Response::reset() {

    httpVersion.clear();
    _statusCode = -1;
    reason.clear();
    allow.clear();
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


    resClient = nullptr;
    _errorFileName.clear();
    formatedResponse.clear();
    _bytesSent = 0;
    _sendStatus = Response::PREPARE;
    _resBody.clear();
    _resFile.clear();

    contentLength = -1;
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
            setErrorParameters(req, Response::ERROR, UNAUTHORIZED_401);
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
	tab["DELETE"]  = & Response::deleteReq;
	tab["PATCH"]   = & Response::patchReq;


    allowedMethods = ft::split(req->reqLocation->methods, ',');
    tmp = std::find(allowedMethods.begin(), allowedMethods.end(), req->method);

    if (tmp == allowedMethods.end()) {
        // TODO : set response header allow !
        setErrorParameters(req, Response::ERROR, METHOD_NOT_ALLOWED_405);
        LOGPRINT(LOGERROR, this, ("Response::methodControl() : Method " + req->method + " is not allowed on route " + req->reqLocation->uri));
    } else
        _methodFctPtr = tab[req->method];
    
    tab.clear();
}

void Response::resDispatch(Request * req) {

    // DON'T DEBUG THE FUNCTION methodControl, it causes LLDB crash !

    methodControl(req);
    authControl(req);
    // ... TODO : Additionnal controls
    
    if (_sendStatus == Response::ERROR)
        return ;
    (this->*_methodFctPtr)(req);
    if (_sendStatus == Response::ERROR)
        return ;

}

void Response::resBuild(Request * req) {
    

    // 1) Status Line
    
    httpVersion = "HTTP/1.1";
    reason = responseUtils::getReasonPhrase(this);

    // 2) Headers
    
    allow.clear();             // Unless Error 405

    contentLanguage[0] = "fr";          // contentLanguage always to "fr"


    contentLocation.clear();            // We don't care
    contentType[0] = responseUtils::getContentType(_resFile);
    lastModified = ft::getLastModifDate(_resFile); // Is here the right place to call ?
    location.clear();                   // Use only with 300 status code
    date = ft::getDate();
    retryAfter.clear();
    server = "webserv-42";              // Config file or hard-coded ?
    
    // Encoding : do we need to handle multiple encoding ? gzip, ... or just chunked
    transfertEncoding.clear();
    if (req->transferEncoding.size() && req->transferEncoding[0] == "chunked" && !req->_reqBody.empty())
        transfertEncoding[0] = "chunked";
    wwwAuthenticate.clear();            // Unless an authorization was asked ?

    // 3) Others
    if (req->method != "HEAD") {

        std::ifstream fd(_resFile);
        std::stringstream buffer;
        buffer << fd.rdbuf();
        _resBody = buffer.str();

    }
    
    contentLength = _resBody.size();    // https://stackoverflow.com/questions/13821263/should-newline-be-included-in-http-response-content-length
    contentLength = contentLength ? contentLength : -1;

}

void Response::resFormat(void) {

    formatedResponse.clear();

    // 1) Status Line
    formatedResponse.append(httpVersion);
    formatedResponse.append(" " + std::to_string(_statusCode) + " ");
    formatedResponse.append(reason);
    formatedResponse.append("\r\n");

    // 2) Headers

    // if (_statusCode == METHOD_NOT_ALLOWED_405)
    //     ...

    responseUtils::headerFormat(formatedResponse, "Allow", allow);
    responseUtils::headerFormat(formatedResponse, "Content-Language", contentLanguage);
    responseUtils::headerFormat(formatedResponse, "Content-Length", contentLength);
    responseUtils::headerFormat(formatedResponse, "Content-Location", contentLocation);
    responseUtils::headerFormat(formatedResponse, "Content-Type", contentType);
    responseUtils::headerFormat(formatedResponse, "Last-Modified", lastModified);
    responseUtils::headerFormat(formatedResponse, "Location", location);
    responseUtils::headerFormat(formatedResponse, "Date", date);
    responseUtils::headerFormat(formatedResponse, "Retry-After", retryAfter);
    responseUtils::headerFormat(formatedResponse, "Host", server);
    responseUtils::headerFormat(formatedResponse, "Transfer-Encoding", transfertEncoding);
    // QUID de www-authenticate ?
    formatedResponse.append("\r\n");
    if (contentLength > 0)
        formatedResponse.append(_resBody);

}

/* Body */

void Response::addBody(Request * req) {

    // TODO LATER

}


/* Errors Settings */

void Response::setErrorParameters(Request * req, int sendStatus, int code) {

    _sendStatus = sendStatus;
    _statusCode = code;
    _errorFileName = "./www/error/error.html"; /* TO UPDATE when we will have one html file per error */

}

/* Utils */

std::string const Response::logInfo(void) {
    std::string ret;
    
    ret = "Response | To Client with Socket : " + std::to_string(resClient->acceptFd) + " | Method : " + resClient->req.method + " | Response Status : " + std::to_string(_sendStatus);
    return (ret);
}