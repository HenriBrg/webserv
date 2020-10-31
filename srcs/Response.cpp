# include "../inc/Webserv.hpp"

Response::Response(void) {
    reset();
}

Response::Response(Client *c) 
{
    reset();
    resClient = c;
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

    // resClient = nullptr; // Segfault on seccond request
    _errorFileName.clear();
    formatedResponse.clear();
    _bytesSent = 0;
    _sendStatus = Response::PREPARE;
    _resBody.clear();
    _resFile.clear();
    _cgiOutputBody.clear();
    contentLength = -1;
    _errorFileName.clear();
    _methodFctPtr = nullptr;
    _didCGIPassed = false;

}

Response::~Response() {
    reset();
}

void Response::setErrorParameters(int sendStatus, int code) {
    _sendStatus = sendStatus;
    _statusCode = code;
    _errorFileName = "./www/errors/error.html"; /* TODO : UPDATE AFTER PARSER DONE */
    _resFile = "./www/errors/error.html";
}

void Response::replaceErrorCode(const Server *server)
{
    size_t pos(0);

    pos = _resBody.find("_ERROR_");
    if (pos != std::string::npos)
        _resBody.replace(pos, 7, server->_errorStatus.at(_statusCode));
}


void Response::authControl(Request * req) {
    
    std::vector<std::string> tab;

    // TODO : modifier de façon à ce que s'il y a auth dans la loc, on passe quand même dans ce if, même si le req->authorization est vide

    if (!req->authorization.empty()) {
        tab = ft::split(req->authorization, ' ');
        if (tab.size() < 2) LOGPRINT(LOGERROR, this, ("Response::authControl() : Incomplete www-authenticate header"));
        if (tab[0] != "Basic" && tab[0] != "BASIC") LOGPRINT(LOGERROR, this, ("Response::authControl() : Unknow www-authenticate encoding"));
        LOGPRINT(INFO, this, ("Response::authControl() : Server credentials are : " + req->reqLocation->auth + " and given authorization header is : " + ft::decodeBase64(tab[1])));
        if (ft::decodeBase64(tab[1]) != req->reqLocation->auth) {
            setErrorParameters(Response::ERROR, UNAUTHORIZED_401);
            LOGPRINT(LOGERROR, this, ("Response::authControl() : Failed authentification"));
            return ;
        } else LOGPRINT(INFO, this, ("Response::authControl() : Successfull authentification"));
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
        setErrorParameters(Response::ERROR, METHOD_NOT_ALLOWED_405);
        LOGPRINT(LOGERROR, this, ("Response::methodControl() : Method " + req->method + " is not allowed on route " + req->reqLocation->uri));
    } else
        _methodFctPtr = serv->methodsTab[req->method];
}

void Response::resourceControl(Request * req)
{
    struct stat fileStat;
    int retStat;

    if (req->method == "DELETE")
    {
        if ((retStat = stat(req->resource.c_str(), &fileStat)) == -1)
            setErrorParameters(Response::ERROR, CONFLICT_409); 
    }
    else if (req->method != "PUT")
    {
        if ((retStat = stat(req->file.c_str(), &fileStat)) == -1)
            setErrorParameters(Response::ERROR, NOT_FOUND_404);
    }
    if (retStat == -1)
        LOGPRINT(REQERROR, this, ("Response::resourceControl() : Resource " + req->file + " not found"));
}

void Response::control(Request * req, Server * serv)
{
    // DON'T DEBUG THE FUNCTION methodControl, it causes LLDB crash !
    resourceControl(req);
    if (_sendStatus == Response::ERROR)
        return ;
    methodControl(req, serv);
    authControl(req);

    // ... TODO : Additionnal controls
        // 1) check http version 
        // 2) if empty headers meaningfull
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
    reason = responseUtils::getReasonPhrase(_statusCode);
    if (_statusCode == -1)
        _statusCode = INTERNAL_ERROR_500;
    
    // 2) Basic headers
    date = ft::getDate();
    server = "webserv";

    // 3) Error headers
    // Ok ---> Donc ici en fait on aura tous les hd qui auront pu être rempli au cours du traitement si erreur il y a 
    if (_sendStatus != Response::ERROR) {

        allow.clear();              // Unless Error 405
        wwwAuthenticate.clear(); // Unless an authorization was asked ?         --------------> POURQUOI UN CLEAR() SEG FAULT ?
        retryAfter.clear();         // Quid du status 301

    }

    // 4) Other headers
    // contentLanguage[0] = "fr";          // contentLanguage always to "fr" ---> finally, useless header if the file isnt explicitely fr 
    contentLanguage.clear();

    if (req->method == "PUT")
        contentLocation[0] = req->file;
    else
        contentLocation.clear();            // We don't care

    location.clear();                   // Use only with 300 status code

    // Encoding : do we need to handle multiple encoding ? gzip, ... or just chunked
    transfertEncoding.clear();
    // TODO : à confirmer mais le fait de recevoir un body chunked n'implique en rien de répondre avec un body chunked
    // if (req->transferEncoding.size() && req->transferEncoding[0] == "chunked" && !req->_reqBody.empty())
    //     transfertEncoding[0] = "chunked";

    // 5) Body headers cleared in case of no body in response
    if (contentType.empty()) // We set it in CGI
        contentType.clear();
    // lastModified.clear(); // Is here the right place to call ? --> moved into methods.cpp
    contentLength = -1;   // https://stackoverflow.com/questions/13821263/should-newline-be-included-in-http-response-content-length
}

void Response::setBody(const Server *server) {

    if (_didCGIPassed == true) {
        NOCLASSLOGPRINT(INFO, "Response::setBody() : _didCGIPassed == true - The body of response is now the cgi output stored in the variable _resBody ");
    } else NOCLASSLOGPRINT(INFO, ("Response::setBody() : _didCGIPassed == false - The body of response is the file _resFile, its path is : " + _resFile));

    if (!(_resFile.empty()))
    {
        char fileBuf[4096];
        int fileFd(0);
        int retRead(0);

        // stat() le file avant ?
        
        if ((fileFd = open(_resFile.c_str(), O_RDONLY)) != -1)
        {
            while ((retRead = read(fileFd, fileBuf, 4096)) != 0)
            {
                // TODO : Ajouter gestion d'erreur
                fileBuf[retRead] = '\0';
                _resBody.append(fileBuf);
            }
            close(fileFd);
        }
    }
    if (_sendStatus == Response::ERROR)
        replaceErrorCode(server);
    _didCGIPassed = false; // ---> Reset somewhere else ?

}

void Response::setBodyHeaders(void)
{
    if (!(_resBody.empty())) {
        if (contentType[0].empty() && !_resFile.empty())
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
    ret = "Response | Destination : Client from port " + std::to_string(resClient->port) + " (socket n°" + std::to_string(resClient->acceptFd) + ") | Method : " \
                                                       + resClient->req.method + " | Response Status : " + std::to_string(_sendStatus) + " | Response Code : " + std::to_string(_statusCode);
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
