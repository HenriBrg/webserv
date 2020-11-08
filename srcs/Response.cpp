# include "../inc/Webserv.hpp"

Response::Response(void) : _resBody(nullptr) {
	reset();
}

Response::Response(Client *c) : _resBody(nullptr) {
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

    _errorFileName.clear();
    formatedResponse.clear();
    _bytesSent = 0;
    _sendStatus = Response::PREPARE;

    if (_resBody)
        free(_resBody);
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
	_errorFileName = resClient->server->error + "/error.html";
	_resFile =  resClient->server->error + "/error.html";
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

/*
**  Contrôle de l'authentification
**  Le header authorization est décodé en supposant un encodage Basic (Base64)
**  Si différences il y a, l'erreur UNAUTHORIZED_401 est retournée au client
**  if (!req->reqLocation->auth.empty())
**  NOCLASSLOGPRINT(LOGERROR, ("req->reqLocation = " + req->reqLocation->uri));
**  NOCLASSLOGPRINT(LOGERROR, ("req->reqLocation auth = " + req->reqLocation->auth));
*/

void Response::authControl(Request * req) {
	
	std::vector<std::string> tab;

	if (!req->reqLocation->auth.empty() && req->authorization.empty()) {
		setErrorParameters(Response::ERROR, UNAUTHORIZED_401);
		LOGPRINT(LOGERROR, this, ("Response::authControl() : Header Authentification is not sent in the request"));
		return ;
	}
	if (!req->reqLocation->auth.empty()) {
		tab = ft::split(req->authorization, ' ');
		if (tab.size() < 2) LOGPRINT(LOGERROR, this, ("Response::authControl() : Incomplete www-authenticate header"));
		if (tab[0] != "Basic" && tab[0] != "BASIC") {
			setErrorParameters(Response::ERROR, UNAUTHORIZED_401);
			LOGPRINT(LOGERROR, this, ("Response::authControl() : Unknow www-authenticate encoding"));
			return ;
		}
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
    int retStat = -1;

    if (req->uri.back() == '.')
        setErrorParameters(Response::ERROR, BAD_REQUEST_400); 
    else if (req->method == "DELETE") {
        if ((retStat = stat(req->resource.c_str(), &fileStat)) == -1)
            setErrorParameters(Response::ERROR, CONFLICT_409); 
    } else if (req->method == "PUT" || req->method == "POST") {
        if (req->method == "PUT" && req->resource.back() == '/')
            setErrorParameters(Response::ERROR, CONFLICT_409);
    } else {
        if ((retStat = stat(req->file.c_str(), &fileStat)) == -1)
            setErrorParameters(Response::ERROR, NOT_FOUND_404);
        else if ((retStat = stat(req->file.c_str(), &fileStat)) == 0 && S_ISDIR(fileStat.st_mode) && req->reqLocation->autoindex == true)
            handleAutoIndex(req);
    }
    if (retStat == -1) NOCLASSLOGPRINT(INFO, ("Response::resourceControl() : Resource " + req->file + " not found - It might be an error or a request to create the resource"));
}

void    Response::handleAutoIndex(Request * req)
{

	DIR			*dir = opendir((req->file).c_str());
	std::string	htmlPage;

	htmlPage = "<!DOCTYPE html>\n \
	<html>\n \
	<head><title>Index of " + req->file + "</title></head>\n \
	<body bgcolor=\"white\">\n \
	<h1>Index of " + req->file + "</h1>\n \
	<hr><pre>\n";
	if (dir != NULL) {
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
			htmlPage += std::string(ent->d_name) + "\n";
		closedir(dir);
	}
	htmlPage += "\t</pre><hr>\n \
	</body>\n \
	</html>";
    _resBody = strdup(htmlPage.c_str());
    contentType[0] = "text/html";
    contentLength = strlen(_resBody);
    LOGPRINT(INFO, this, ("Response::handleAutoIndex() : autoindex generated"));
}

void Response::reqHeadersControl(Request * req) {

	if (req->host.empty()) {
		LOGPRINT(INFO, this, ("Response::reqHeadersControl() : Host header is absent of the request headers. 400 BAD REQUEST"));
		return setErrorParameters(Response::ERROR, BAD_REQUEST_400);
	}
}

/*
**  Dispatching function for controlling request
*/

void Response::control(Request * req, Server * serv) {
    
    int _statusCodeBackup;

    _statusCodeBackup = _statusCode;
    versionControl(req);
    if (_statusCode != _statusCodeBackup)
        NOCLASSLOGPRINT(LOGERROR, "An error has been detected in  versionControl() - _statusCode is now : " + std::to_string(_statusCode))
    _statusCodeBackup = _statusCode;
    reqHeadersControl(req);
    if (_statusCode != _statusCodeBackup)
        NOCLASSLOGPRINT(LOGERROR, "An error has been detected in  reqHeadersControl() - _statusCode is now : " + std::to_string(_statusCode))
    _statusCodeBackup = _statusCode;
    resourceControl(req);
    if (_statusCode != _statusCodeBackup)
        NOCLASSLOGPRINT(LOGERROR, "An error has been detected in  resourceControl() - _statusCode is now : " + std::to_string(_statusCode))
    _statusCodeBackup = _statusCode;
    methodControl(req, serv);
    if (_statusCode != _statusCodeBackup)
        NOCLASSLOGPRINT(LOGERROR, "An error has been detected in  methodControl(req, () - _statusCode is now : " + std::to_string(_statusCode))
    _statusCodeBackup = _statusCode;
    authControl(req);
    if (_statusCode != _statusCodeBackup)
        NOCLASSLOGPRINT(LOGERROR, "An error has been detected in  authControl() - _statusCode is now : " + std::to_string(_statusCode))

}

/*
**  Call for corresponding method function set in methodControl()
*/

void Response::callMethod(Request * req) {
	if (_sendStatus != Response::ERROR)
		(this->*_methodFctPtr)(req);
}

/*
**  SET ALL HEADERS
*/

void Response::setHeaders(Request * req) {

	/* 1) Status Line */
	httpVersion = "HTTP/1.1";
	reason = responseUtils::getReasonPhrase(_statusCode);
	if (_statusCode == -1) _statusCode = INTERNAL_ERROR_500;
	/* 2) Basic headers */
	date = ft::getDate();
	server = "webserv";
	/* 3) Error headers */
	if (_sendStatus != Response::ERROR) {
		allow.clear();
		retryAfter = -1;
	}

    // Set wwwAuthenticate header if auth error
	wwwAuthenticate.clear();
    if (_statusCode == UNAUTHORIZED_401)
        wwwAuthenticate.append("Basic");
	/* 4) Other headers */
	if (_isLanguageNegociated == true && !_resFile.empty())
        contentLanguage[0] = _resFile.substr(_resFile.rfind("."), _resFile.size() - 1);

    // Set Content Language -> What if multiple tag (de-DE, en-CA - > file.html.de-DE.en-CA) ? --> à voir ensemble
    if (_isLanguageNegociated == true)
	     contentLocation[0] = _resFile;
	else contentLocation.clear();
	
	if (_statusCode == CREATED_201) location = req->uri;
	else location.clear();
	transfertEncoding.clear();
	/* 5) Body headers cleared in case of no body in response */
	if (contentType.empty()) // We set it in CGI
		contentType.clear();
	/* lastModified.clear(); // Is here the right place to call ? --> moved into methods.cpp */
	if (_resFile.empty() && !(_resBody)) // ---> à voir
		contentLength = -1;

}

/*
** Fonction en cours
** Que le body soit chunked ou pas, send() va fail ... à la limite faut call send() chunk par chunk, ou faire des plus petit call de send()
** Idéalement : open, lire par 32 768 ou moins, le chunké et le send
*/

void Response::intoChunk() {

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


    if ((_statusCode == OK_200 || _statusCode == CREATED_201) && resClient->req.method == "POST" && _didCGIPassed == true) {
        LOGPRINT(INFO, this, "Response::setBody() : POST && _didCGIPassed == true - We enchunk the body");
        intoChunk();
        NOCLASSLOGPRINT(INFO, "Response::setBody() : Enchunking done");
        return ;
    }

    if (!(_resBody) && !(_resFile.empty()))
    {
        char fileBuf[4096];
        int fileFd(0);
        int retRead(0);
        int saveOffset(0);

        if (responseUtils::setupBytesArray(this) == -1)
            return ;
        else if ((fileFd = open(_resFile.c_str(), O_RDONLY)) != -1)
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
            if (retRead == 0)
                LOGPRINT(INFO, this, ("Response::setBody() : read() body file returned 0 | Bytes copied = " + std::to_string(saveOffset)));
            contentLength = saveOffset;
            close(fileFd);
        }
        else
            LOGPRINT(LOGERROR, this, ("Response::setBody() : open() body file failed"));
    }

    if (_resBody && _sendStatus == Response::ERROR)
        replaceErrorCode(server);
}


/*
**  Set body headers values
*/

void Response::setBodyHeaders(void) {


    if (_resBody) {
        
		if (contentType[0].empty() && !_resFile.empty())
			contentType[0] = responseUtils::getContentType(_resFile);
		if (resClient->req.method == "GET" || resClient->req.method == "HEAD")
			lastModified = ft::getLastModifDate(_resFile);
	}

    /*   The purpose here is to cut the reponse body in case of PUT since it is useless to send back the full updated (200) created (201)
    **   resource to the client (https://httpstatuses.com/201 - RFC7231) */

    if ((_statusCode == OK_200 || _statusCode == CREATED_201) && resClient->req.method == "PUT")
		    contentLength = 0;
    else if ((_statusCode == OK_200 || _statusCode == CREATED_201) && resClient->req.method == "POST") {
        /*
        if (_didCGIPassed) {
            transfertEncoding[0] = "chunked"; --> TO ADD when chunk res done
            contentLength = -1;
        }
        else */ 
        contentLength = strlen(_resBody);
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
    responseUtils::headerFormat(formatedResponse, "WWW-Authenticate", wwwAuthenticate);
    formatedResponse.append("\r\n");

}

void Response::setRefusedClient(const Server *serv)
{
    setErrorParameters(Response::ERROR, SERVICE_UNAVAILABLE_503);
	httpVersion = "HTTP/1.1";
	reason = responseUtils::getReasonPhrase(_statusCode);
	date = ft::getDate();
	server = "webserv";
    retryAfter = 10;
    setBody(serv);
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
	if (SILENTLOGS == 0)
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
    utils::displayHeaderMap(transfertEncoding, (indent + "Transfer-Encoding"));
    if (!wwwAuthenticate.empty())  std::cout << indent << "WWW-Authenticate : " << wwwAuthenticate << std::endl;
    if (!lastModified.empty())  std::cout << indent << "Last-Modified : " << lastModified << std::endl;
    if (!location.empty())      std::cout << indent << "Location : " << location << std::endl;
    if (!date.empty())          std::cout << indent << "Date : " << date << std::endl;
    if (retryAfter != -1)      std::cout << indent << "Retry-After: " << retryAfter << std::endl;
    if (!server.empty())        std::cout << indent << "Server : " << server << std::endl;
    std::cout << std::endl;
    std::cout << indent << "Content-Length : " << std::to_string(contentLength) << std::endl;
    if (!_errorFileName.empty())    std::cout << indent << "_errorFileName : " << _errorFileName << std::endl;
    if (!_resFile.empty())          std::cout << indent << "_resFile : " << _resFile << std::endl;
    int x =  contentLength;
    std::cout << indent << "_resBody Size : " << std::to_string(x) << std::endl;
	/* std::cout << indent << "_resBody content : " << ( x < 500 ? _resBody : "_resBody too big") << std::endl; */
}
