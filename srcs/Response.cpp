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
    _isLanguageNegociated = false;
    contentLocation.clear();
    contentType.clear();
    lastModified.clear();
    location.clear();
    date.clear();
    retryAfter = -1;
    server.clear();
    transfertEncoding.clear();
    wwwAuthenticate.clear();

    // resClient = nullptr; // Segfault on seccond request
    _errorFileName.clear();
    formatedResponse.clear();
    _bytesSent = 0;
    _sendStatus = Response::PREPARE;

    _resBody = nullptr;

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

/*
**  Replace tag _ERROR_ in error.html by response error status
**  1. Get index of _ERROR_ tag
**  2. Create new char* of final size
**  3. Fullfil new char* with pre-tag chars
**  4. Fullfil new char* with response error status
**  5. Fullfil new char* with post-tag chars
**  6. Free _resBody and attribuate new char* to it
*/
void Response::replaceErrorCode(const Server *server)
{
    int index(0);

    index = ft::ft_strstr(_resBody, "_ERROR_", responseUtils::getContentLength(_resFile));
    if (index >= 0)
    {
        int fileLength = responseUtils::getContentLength(_resFile) - 7;
        int replaceLength = server->_errorStatus.at(_statusCode).size();
        int totalLength = fileLength + replaceLength;
        char *tmp = (char*)malloc(sizeof(char) * totalLength);

        responseUtils::copyBytes(tmp, _resBody, index, 0);
        responseUtils::copyBytes(tmp, server->_errorStatus.at(_statusCode).c_str(), replaceLength, index);
        responseUtils::copyBytes(tmp, &(_resBody[index + 7]), (fileLength - index),(replaceLength + index));

        free(_resBody);
        _resBody = tmp;
        contentLength = totalLength;
    }
}


void Response::authControl(Request * req) {
    
    std::vector<std::string> tab;

    if (!req->reqLocation->auth.empty() && !req->authorization.empty()) {
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

/*
**  Verify if received METHOD is valid
**  1. Get all allowed methods of location
**  2. Compare them to received method
**  3. If no one match then error 405
**  4. Else attribuate corresponding method function
*/
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


/*
**  Verify if received version is valid
**  1. Verify total size (segfault)
**  2. Verify HTTP 
**  3. Verify http version
*/
void Response::versionControl(Request *req)
{
    if (req->httpVersion.size() < 8
    || req->httpVersion.substr(0, 5) != "HTTP/")
        setErrorParameters(Response::ERROR, BAD_REQUEST_400);
    else if (req->httpVersion.at(5) != '1')
        setErrorParameters(Response::ERROR, HTTP_VERSION_NOT_SUPPORTED_505);
}


/*
**  Verify if requested resource is valid
**  1. If Delete -> verify if resource exists
**  2. If PUT || POST -> verify if resource is a directory (if true then error)
**  3. Else verify if file exists
*/
void Response::resourceControl(Request * req)
{
    struct stat fileStat;
    int retStat;

    if (req->method == "DELETE") {
        if ((retStat = stat(req->resource.c_str(), &fileStat)) == -1)
            setErrorParameters(Response::ERROR, CONFLICT_409); 
    }
    else if (req->method == "PUT" || req->method == "POST")
    {
        if (req->resource.back() == '/')
            setErrorParameters(Response::ERROR, CONFLICT_409);
         if (req->method == "POST" && req->isolateFileName.empty()) {
            LOGPRINT(INFO, this, ("Response::resourceControl() : POST - isolateFileName is empty, so there is nothing to create/update. Invalid Request"));
            setErrorParameters(Response::ERROR, BAD_REQUEST_400);
        }
    }
    else
    {
        if ((retStat = stat(req->file.c_str(), &fileStat)) == -1)
            setErrorParameters(Response::ERROR, NOT_FOUND_404);
    }
    if (retStat == -1)
        NOCLASSLOGPRINT(REQERROR, ("Response::resourceControl() : Resource " + req->file + " not found"));
}


/*
**  Dispatching function for controlling request
*/
void Response::control(Request * req, Server * serv) {
    
    if (_sendStatus != Response::ERROR)
        versionControl(req);
    if (_sendStatus != Response::ERROR)
        resourceControl(req);
    if (_sendStatus != Response::ERROR)
        methodControl(req, serv);
    if (_sendStatus != Response::ERROR)
        authControl(req);

    // TODO : be sure that we dont forget meaningfull headers
}


/*
**  Call for corresponding method function set in methodControl()
*/
void Response::callMethod(Request * req) {
    if (_sendStatus != Response::ERROR)
        (this->*_methodFctPtr)(req);
}


/*
**  SET ALL HEADERS TODO
*/
void Response::setHeaders(Request * req) {

    // 1) Status Line
    httpVersion = "HTTP/1.1";
    reason = responseUtils::getReasonPhrase(_statusCode);
    if (_statusCode == -1) _statusCode = INTERNAL_ERROR_500;
    
    // 2) Basic headers
    date = ft::getDate();
    server = "webserv";

    // 3) Error headers
    // Ok ---> Donc ici en fait on aura tous les hd qui auront pu être rempli au cours du traitement si erreur il y a 

    if (_sendStatus != Response::ERROR) {
        allow.clear();              // Unless Error 405
        wwwAuthenticate.clear();
        retryAfter = -1;         // Quid du status 301
    }

    if (_statusCode == UNAUTHORIZED_401) wwwAuthenticate[0] = "Basic";

    // 4) Other headers
    // contentLanguage[0] = "fr";          // TODO : si la négotiation à réussi, ce header doit le prendre en compte
    // contentLanguage[0] = "fr";          // contentLanguage always to "fr" ---> finally, useless header if the file isnt explicitely fr 
    // if (_isLanguageNegociated)
    //     contentLanguage = _resFile. // Set Content Language -> What if multiple tag (de-DE, en-CA - > file.html.de-DE.en-CA) ?

    contentLanguage.clear();

    if (req->method == "PUT") contentLocation[0] = req->file;
    else contentLocation.clear();

    if (_statusCode == CREATED_201) location = req->uri;
    else location.clear();

    // TODO : à confirmer mais le fait de recevoir un body chunked n'implique en rien de répondre avec un body chunked
    // if (req->transferEncoding.size() && req->transferEncoding[0] == "chunked" && !req->_reqBody.empty())
    //     transfertEncoding[0] = "chunked";
    transfertEncoding.clear();

    // 5) Body headers cleared in case of no body in response
    if (contentType.empty()) // We set it in CGI
        contentType.clear();
    // lastModified.clear(); // Is here the right place to call ? --> moved into methods.cpp
    contentLength = -1;   // https://stackoverflow.com/questions/13821263/should-newline-be-included-in-http-response-content-length
}

/*
**  Function to fullfit body (bytes array) with file content
**  1. Allocate char* of file size
**  2. Read file to retreive bytes
**  3. If error html page then contextualize body with corresponding error status
*/
void Response::setBody(const Server *server) {

    if (_didCGIPassed == true) {
        NOCLASSLOGPRINT(INFO, "Response::setBody() : _didCGIPassed == true - The body of response is now the cgi output stored in the variable _resBody ");
    } else NOCLASSLOGPRINT(INFO, ("Response::setBody() : _didCGIPassed == false - The body of response is the file _resFile, its path is : " + _resFile));

    if (!(_resFile.empty()))
    {
        char fileBuf[4096];
        int fileFd(0);
        int retRead(0);
        int saveOffset(0);

        if (responseUtils::setupBytesArray(this) == -1)
            return ;
        if ((fileFd = open(_resFile.c_str(), O_RDONLY)) != -1)
        {
            while ((retRead = read(fileFd, fileBuf, 4096)) != 0)
            {
                if (retRead == -1)
                {
                    LOGPRINT(LOGERROR, this, ("Response::setBody() : read() body file failed | Bytes copied = " + std::to_string(saveOffset)));
                    break ;
                }
                responseUtils::copyBytes(_resBody, fileBuf, retRead, saveOffset);
                saveOffset += retRead;
            }
            contentLength = saveOffset;
            close(fileFd);
        }
        else
            LOGPRINT(LOGERROR, this, ("Response::setBody() : open() body file failed"));
        
    }
    if (_resBody && _sendStatus == Response::ERROR)
        replaceErrorCode(server);
    _didCGIPassed = false; // ---> Reset somewhere else ?

}


/*
**  Set body headers values
*/
void Response::setBodyHeaders(void)
{
    if (_resBody)
    {
        if (contentType[0].empty() && !_resFile.empty())
            contentType[0] = responseUtils::getContentType(_resFile);
        lastModified = ft::getLastModifDate(_resFile); // Is here the right place to call ?
    }
}


void Response::format(void) {

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
}

/* **************************************************** */
/*                        LOGGER                        */
/* **************************************************** */

std::string const Response::logInfo(void) {

    std::string ret;
    if (resClient == NULL) return "LOG WOULD SEGFAULT - We dont print it";
    ret = "Response | Destination : Client from port " + std::to_string(resClient->port) + " (socket n°" + std::to_string(resClient->acceptFd) + ") | Method : " + resClient->req.method + " | Response Status : " + std::to_string(_sendStatus) + " | Response Code : " + std::to_string(_statusCode);
    return (ret);

}

void Response::showRes(void) {

    std::string indent("    > ");
    std::cout << std::endl << std::endl;
    std::cout << ORANGE << "    RESPONSE THAT WILL BE SENT ----------------" << END;
    std::cout << std::endl << std::endl;
    std::cout << "    ~ Global \n\n";
    std::cout << indent << "HTTP Version : " << httpVersion << std::endl;
    std::cout << indent << "Status Code : " << std::to_string(_statusCode) << std::endl;
    std::cout << indent << "Reason : " << reason << std::endl;
    showFullHeadersRes();    
    std::cout << std::endl;
    std::cout << ORANGE << "    ------------------------------- END" << END;
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
    if (!lastModified.empty())  std::cout << indent << "Last-Modified : " << lastModified << std::endl;
    if (!location.empty())      std::cout << indent << "Location : " << location << std::endl;
    if (!date.empty())          std::cout << indent << "Date : " << date << std::endl;
    if (retryAfter != -1)      std::cout << indent << "Retry-After: " << retryAfter << std::endl;
    if (!server.empty())        std::cout << indent << "Server : " << server << std::endl;
    std::cout << std::endl;
    std::cout << indent << "Content-Length : " << std::to_string(contentLength) << std::endl;
    if (!_errorFileName.empty())    std::cout << indent << "_errorFileName : " << _errorFileName << std::endl;
    if (!_resFile.empty())          std::cout << indent << "_resFile : " << _resFile << std::endl;

}
