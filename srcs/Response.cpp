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

void Response::setErrorParameters(Request * req, int sendStatus, int code) {
    _sendStatus = sendStatus;
    _statusCode = code;
    _errorFileName = "./www/errors/error.html"; /* TODO : UPDATE AFTER PARSER DONE */
}


/* https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Authorization */

void Response::authControl(Request * req) {
    
    std::vector<std::string> tab;

    if (!req->authorization.empty()) {

        tab = ft::split(req->authorization, ' ');
        if (tab.size() < 2)
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Incomplete www-authenticate header"));
        if (tab[0] != "Basic" && tab[0] != "BASIC")
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Unknow www-authenticate encoding"));

        LOGPRINT(INFO, this, ("Response::authControl() : Server credentials are : " + req->reqLocation->auth + " and given authorization header is : " + ft::decodeBase64(tab[1])));

        if (ft::decodeBase64(tab[1]) != req->reqLocation->auth) {
            setErrorParameters(req, Response::ERROR, UNAUTHORIZED_401);
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Failed authentification"));
            return ;
        } else
            LOGPRINT(INFO, this, ("Response::authControl() : Successfull authentification"));
    }

}

void Response::methodControl(Request * req) {

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
        setErrorParameters(req, Response::ERROR, METHOD_NOT_ALLOWED_405);
        LOGPRINT(LOGERROR, this, ("Response::methodControl() : Method " + req->method + " is not allowed on route " + req->reqLocation->uri));
    } else {
        LOGPRINT(INFO, this, ("Server::methodControl() : Method " + *tmp + " authorized"));
        _methodFctPtr = tab[req->method];
    }
    
    tab.clear();
}

void Response::resDispatch(Request * req) {

    methodControl(req);
    authControl(req);
    // TODO : + de contrôle 
    if (_sendStatus == Response::ERROR)
        return ;
    (this->*_methodFctPtr)(req);
    if (_sendStatus == Response::ERROR)
        return ; // TO HANDLE and set status sending

}

void Response::resBuild(Request * req) {
    
    // 1) Status Line
    httpVersion = "HTTP/1.1";
    reason = responseUtils::getReasonPhrase(this);
    // 2) Headers
    allow.clear();                                                   // TODO : not good (and handle error 405)
    contentLanguage[0] = "fr";                                       // contentLanguage always to "fr"
    contentLocation.clear();                                         // We don't care
    contentType[0] = responseUtils::getContentType(_resFile);
    lastModified = ft::getLastModifDate(_resFile);                   // Is here the right place to call ?
    location.clear();                                                // Use only with 300 status code
    date = ft::getDate();
    retryAfter.clear();
    server = "webserv";                                              // Config file or hard-coded ?
    transfertEncoding.clear();                                       // Encoding : do we need to handle multiple encoding ? gzip, ... or just chunked
    if (req->transferEncoding.size() &&
        req->transferEncoding[0] == "chunked" && !req->_reqBody.empty())
        transfertEncoding[0] = "chunked";
    wwwAuthenticate.clear();                                         // Unless an authorization was asked ?
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




/*  -----------------------------------  LOGGER ---------------------------------------------------- */





std::string const Response::logInfo(void) {
    std::string ret;
    ret = "Response | Destination : Client from port " + std::to_string(resClient->port) + " (socket n°" + std::to_string(resClient->acceptFd) + ") | Method : " + resClient->req.method + " | Response Status : " + std::to_string(_sendStatus);
    return (ret);
}


void Response::showRes(void) {

    std::string indent("    > ");
    std::cout << std::endl << std::endl;
    std::cout << MAGENTA << "    RESPONSE THAT WILL BE SENT ----------------" << END;
    std::cout << std::endl << std::endl;
    std::cout << "    ~ Global \n\n";
    std::cout << indent << "HTTP Version : " << httpVersion << std::endl;
    std::cout << indent << "Status Code : " << std::to_string(_statusCode) << std::endl;
    std::cout << indent << "Reason : " << reason << std::endl;
    showFullHeadersRes();    
    std::cout << std::endl;
    std::cout << MAGENTA << "    ------------------------------- END" << END;
    std::cout << std::endl << std::endl;
}



void Response::showFullHeadersRes(void) {

    std::string indent("    > ");
 
    std::cout << std::endl;
    std::cout << "    ~ Details";
    std::cout << std::endl;
    std::cout << std::endl;
    utils::displayHeaderMap(contentLocation, (indent + "Content-Location"));
    utils::displayHeaderMap(contentType, (indent + "Content-Type"));
    utils::displayHeaderMap(wwwAuthenticate, (indent + "WWW-Authenticate"));
    utils::displayHeaderMap(transfertEncoding, (indent + "Transfer-Encoding"));
    if (!lastModified.empty())
        std::cout << indent << "Last-Modified : " << lastModified << std::endl;
    if (!location.empty())
        std::cout << indent << "Location : " << location << std::endl;
    if (!date.empty())
        std::cout << indent << "Date : " << date << std::endl;
    if (!retryAfter.empty())
        std::cout << indent << "Retry-After: " << retryAfter << std::endl;
    if (!server.empty())
        std::cout << indent << "Server : " << server << std::endl;
    std::cout << std::endl;
    std::cout << indent << "Content-Length : " << std::to_string(contentLength) << std::endl;
    if (!_errorFileName.empty())
        std::cout << indent << "_errorFileName : " << _errorFileName << std::endl;
    if (!_resFile.empty())
        std::cout << indent << "_resFile : " << _resFile << std::endl;

}
