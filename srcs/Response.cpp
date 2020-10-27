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

void Response::methodControl(Request * req, Server * serv)
{
    std::vector<std::string>    allowedMethods;
    std::vector<std::string>::iterator    tmp;

    allowedMethods = ft::split(req->reqLocation->methods, ',');
    tmp = std::find(allowedMethods.begin(), allowedMethods.end(), req->method);
    if (tmp == allowedMethods.end()) {
        allow = req->reqLocation->methods;
        setErrorParameters(req, Response::ERROR, METHOD_NOT_ALLOWED_405);
        LOGPRINT(LOGERROR, this, ("Response::methodControl() : Method " + req->method + " is not allowed on route " + req->reqLocation->uri));
    } else
        _methodFctPtr = serv->methodsTab[req->method];
}

void Response::control(Request * req, Server * serv)
{
    // DON'T DEBUG THE FUNCTION methodControl, it causes LLDB crash !

    methodControl(req, serv);
    authControl(req);
    // ... TODO : Additionnal controls
}

void Response::callMethod(Request * req)
{
    if (_sendStatus != Response::ERROR)
        (this->*_methodFctPtr)(req);
}

void Response::setHeaders(Request * req)
{
    // 1) Status Line 
    httpVersion = "HTTP/1.1";
    reason = responseUtils::getReasonPhrase(this);
    
    // 2) Basic headers
    date = ft::getDate();
    server = "webserv-42";

    // 3) Error headers
    if (_sendStatus != Response::ERROR)
    {
        allow.clear();             // Unless Error 405
        wwwAuthenticate.clear();            // Unless an authorization was asked ?
        retryAfter.clear();  // Quid du status 301
    }

    // 4) Other headers
    contentLanguage[0] = "fr";          // contentLanguage always to "fr"
    contentLocation.clear();            // We don't care
    location.clear();                   // Use only with 300 status code

    // Encoding : do we need to handle multiple encoding ? gzip, ... or just chunked
    transfertEncoding.clear();
    if (req->transferEncoding.size() && req->transferEncoding[0] == "chunked" && !req->_reqBody.empty())
        transfertEncoding[0] = "chunked";

    // 5) Body headers cleared in case of no body in response
    contentType.clear();
    lastModified.clear(); // Is here the right place to call ?
    contentLength = -1;   // https://stackoverflow.com/questions/13821263/should-newline-be-included-in-http-response-content-length
}

void Response::setBody(void) {

    if (!(_resFile.empty()))
    {
        char fileBuf[4096];
        int fileFd(0);
        int retRead(0);
        
        if ((fileFd = open(_resFile.c_str(), O_RDONLY)) != -1)
        {
            while ((retRead = read(fileFd, fileBuf, 4096)) != 0)
            {
                // Ajouter gestion d'erreur
                fileBuf[retRead] = '\0';
                _resBody.append(fileBuf);
            }
            close(fileFd);
        }
    }
}

void Response::setBodyHeaders(void)
{
    if (!(_resBody.empty()))
    {
        contentType[0] = responseUtils::getContentType(_resFile);
        lastModified = ft::getLastModifDate(_resFile); // Is here the right place to call ?
        contentLength = _resBody.size();    // https://stackoverflow.com/questions/13821263/should-newline-be-included-in-http-response-content-length
    }
}


void Response::format(void)
{

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
    if (contentLength > 0) // TODO or chunked 
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
