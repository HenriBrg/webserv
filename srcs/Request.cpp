# include "../inc/Webserv.hpp"

Request::Request(void) {
    reset();
}

Request::Request(Client * c) 
: client(c) 
{
    reset();
}

Request::~Request() {
    reset();
}

void Request::reset(void) {

    //client = nullptr // segfaut as on response 
    reqLocation = nullptr;
    reqBuf.clear();
    _reqBody.clear();

    _currentParsedReqBodyLength = -1;
    contentLength = -1;
    chunkLineBytesSize = -1;
    _optiChunkOffset = 0;
    file.clear();
    isolateFileName.clear();
    cgiType = NO_CGI;

    method.clear();
    uri.clear();
    resource.clear();
    httpVersion.clear();
    uriQueries.clear();
    acceptCharset.clear();
    acceptLanguage.clear();
    authorization.clear();
    contentLanguage.clear();
    contentLocation.clear();
    contentType.clear();
    date.clear();
    host.clear();
    referer.clear();
    userAgent.clear();
    uriQueries.clear();
    transferEncoding.clear();
    keepAlive.clear();

}

std::string mapToStr(std::map<int, std::string> map, char sep) {
    
    size_t i = 0;
    std::stringstream ret;

    ret.clear();    
    while (i < map.size()) {
        ret << map[int(i)];
        if ((i + 1) < map.size())
            ret << sep << " ";
        ++i;
    }
    return (ret.str());
}

 std::map<std::string, std::string> Request::mapReqHeaders(void) {
    
    std::map<std::string, std::string> ret;

    // ret["Accept-Charset"] = mapToStr(acceptCharset, ';');
    // ret["Accept-Language"] = mapToStr(acceptLanguage, ',');
    ret["Authorization"] = authorization;
    ret["Content-Language"] = mapToStr(contentLanguage, ',');
    ret["Content-Length"] = std::to_string(contentLength);
    ret["Content-Location"] = contentLocation;
    ret["Content-Type"] = contentType;
    ret["Date"] = date;
    ret["Host"] = host;
    ret["Referer"] = referer;
    ret["User-Agent"] = userAgent;
    ret["User-Agent"] = keepAlive;

    return (ret);
}


int Request::parseRequestLine() {

    std::string                 line;
    std::vector<std::string>    tab;

    ft::getLine(reqBuf, line);
    tab = ft::split(line, ' ');
    if (tab.size() != 3)
        return (-1);
    method      = tab[0];
    uri         = tab[1];
    httpVersion = tab[2];
    // if (httpVersion != "HTTP/1.1")
    //     client->res.setErrorParameters(Response::ERROR, BAD_REQUEST_400);
    return (0);
}

// The request line holds the location AND the params : http://www.domain.com/index.html?name1=value1
// We differenciate the storage of uri : http://www.domain.com/index.html
// and the storage of the query : ?name1=value1

void Request::parseUriQueries() {

    // TODO : gérer les failles de sécurités liées aux paramètres d'URL, injection js ...

    int i = 0;
    std::string tmp = uri;

    while (tmp[i] && tmp[i] != '?')
        i++;
    if (tmp[i] == '?') {
        uriQueries = uri.substr(i + 1, tmp.size());
        uri = tmp.substr(0, i);
    }
    if (uriQueries.size() > 1024 && client != nullptr)
        client->res.setErrorParameters(Response::ERROR, REQUEST_URI_TOO_LONG_414);
}


// Location NGinx : http://nginx.org/en/docs/beginners_guide.html
// Faille de sécurité à gérer ultérieurement : Directory Traversal    
// 1) If URI requested match directly one of server's locations, we match here
// TODO : à confirmer mais, si échec de location de la ressource, la location de base "/"" doit être utilisée pour réessayer l'association d'une location à la requête ?

void Request::assignLocation(std::vector<Location*> vecLocs) {

    Location * root;

    for (std::size_t i = 0; i < vecLocs.size(); i++) {

        if (vecLocs[i]->uri == uri) {
            reqLocation = vecLocs[i];
            LOGPRINT(INFO, this, ("Request::assignLocation() : Location directly assigned, the index of the location should be the resource to return"));
            return ;
        }
    }

    size_t i = uri.size() - 1;
    std::string tmpUri = uri;

    while (tmpUri.size() > 0) {                     // Parcours URI entier
		while (tmpUri[i] != '/' && i != 0)          // Si / alors arret
			i--;
		tmpUri = tmpUri.substr(0, i);               // tmpUri => partie droite URI
		if (tmpUri == "")                           // Si racine
			tmpUri = "/";
		for (std::size_t x = 0; x < vecLocs.size(); ++x) {
			if (vecLocs[x]->uri == tmpUri) {
                // For example, http://localhost/pouet --> file = pouet
				file = uri.substr(i + 1, uri.size());
				reqLocation = vecLocs[x];
                LOGPRINT(INFO, this, ("Request::assignLocation() : Location indirectly assigned. file = " + file));
                return ;
			}
		}
	}

}

/*
** We assign location
** If location doesn't end with '/', we add it
** 
** We check if the uri refers to a directory.
** If so, we check if autoindex is on, else, we refers to the index parameter of the location

*/

void Request::parseFile(std::vector<Location*> locations)
{
    std::string tmpFile;
    int         i;
    struct stat info;

    assignLocation(locations);

    if (reqLocation)
    {
        isolateFileName = file;
        i = reqLocation->root.size() - 1;
        if (reqLocation->root[i] == '/')
            file = reqLocation->root + file;
        else
            file = reqLocation->root + "/" + file;
        resource = file;
        if (stat(file.c_str(), &info) == 0 && S_ISDIR(info.st_mode)) {
            // TODO WHEN PARSER READY
            // if (reqLocation->autoindex == 1 && method == "GET")
            //     handleAutoIndex();
            // else {
                file = file + reqLocation->index;
            // }
        }
        LOGPRINT(INFO, this, ("Request::parseFile() : Autoindex = 0 and File Assignedd : " + file));
        
    }

}

/*
** Functions for filling headers value(s) into corresponding variables
*/

void Request::fillHeader(std::string const key, std::string const value)
{
    if (key == "Content-Language" || key == "Transfer-Encoding")
        fillMultiValHeaders(key, value);
    else if (key == "Accept-Charset" || key == "Accept-Language")
        fillMultiWeightValHeaders(key, value);
    else
        fillUniqueValHeaders(key, value);
}

/*
** If the variable can have multiple values => map is used to stored them
** Map key is index after split and map value is request value
** Example => Content-Language: de-DE, en-CA
*/

void Request::fillMultiValHeaders(std::string const key, std::string const value)
{
    std::vector<std::string> multiValues = ft::split(value, ',');

    for (size_t count = 0; count < multiValues.size(); count++)
    {
        if (key == "Content-Language")
            contentLanguage[count] = multiValues[count];
        else if (key == "Transfer-Encoding")
            transferEncoding[count] = multiValues[count];
    }
}

// If the variable can have multiple weighted values => multimap is used to stored them
// Map key is weight after split and map value is request value
// Accept-Language: fr-CH, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5
void Request::fillMultiWeightValHeaders(std::string const key, std::string const value)
{
    std::vector<std::string> multiValues = ft::split(value, ',');
    size_t weightPos(0);

    for (size_t count = 0; count < multiValues.size(); count++)
    {
        multiValues[count] = ft::trim(multiValues[count]);
        if ((weightPos = multiValues[count].find(";q=")) == std::string::npos) // If no weigth then key is 1.0
        {
            if (key == "Accept-Charset")
                acceptCharset.insert(std::make_pair(1.0, multiValues[count]));
            else if (key == "Accept-Language")
                acceptLanguage.insert(std::make_pair(1.0, multiValues[count]));
        }
        else // If weight then key is specified weight
        {
            multiValues[count].replace(weightPos, 3, " ");
            std::vector<std::string> weight = ft::split(multiValues[count], ' ');
            if (key == "Accept-Charset")
                acceptCharset.insert(std::make_pair(std::atof(weight[1].c_str()), weight[0]));
            else if (key == "Accept-Language")
                acceptLanguage.insert(std::make_pair(std::atof(weight[1].c_str()), weight[0]));
        }
    }
}

void Request::fillUniqueValHeaders(std::string const key, std::string const value)  {

    if (key == "Keep-Alive") keepAlive = value;
    else if (key == "Content-Length") contentLength = std::stoi(value);
    else if (key == "Content-Type" || key == "Content-type") contentType = value;
    else if (key == "Authorization") authorization = value;
    else if (key == "Date") date = value;
    else if (key == "Host") host = value;
    else if (key == "Referer") referer = value;
    else if (key == "User-Agent") userAgent = value;

}

void Request::parseHeaders() {

    std::size_t  pos;
    std::string  line;
    std::string  key;
    std::string  value;

    while (!reqBuf.empty()) {
        ft::getLine(reqBuf, line);
        pos = line.find(":");
        if (pos == std::string::npos)
            return ;
        key = ft::trim(line.substr(0, pos));
        utils::deleteCarriageReturn(key);
        if (key.empty())
            continue ; // TODO : continue or break ? Or error 4XX
        value = ft::trim(line.substr(pos + 1));
        // if (value.empty())
        // Continue or break ? Or error 4XX
        utils::deleteCarriageReturn(value);
        fillHeader(key, value);
    }

}



/*
**  Choix du parsing selon les headers recus et leurs valeurs 
**  1. Vérification présence d'un body
**  2. Appel de la fonction correspondante si chunked ou pas
**  3. Vérification taille du body ne dépasse pas la taille max défini dans le fichier de config
*/
void Request::parseBody()
{
    if (client->recvStatus == Client::BODY)
    {
        if (transferEncoding[0] == "chunked")
            parseChunkedBody();
        else if (contentLength > 0)
            parseSingleBody();
        else
            LOGPRINT(LOGERROR, client, ("Server::readClientRequest() : Anormal body"));
        if (reqLocation->max_body != -1 && _reqBody.size() > reqLocation->max_body)
        {
            LOGPRINT(REQERROR, client, ("Server::readClientRequest() : Error : REQUEST_ENTITY_TOO_LARGE_413 - Max = " + std::to_string(reqLocation->max_body)));
            client->recvStatus = Client::ERROR;
            client->res.setErrorParameters(Response::ERROR, REQUEST_ENTITY_TOO_LARGE_413);
        }
    }
}


// Really good article
// https://en.wikipedia.org/wiki/Chunked_transfer_encoding
// Example Body = "14\r\nabcdefghijklmnopqrst\r\nA\r\n0123456789\r\n0\r\n\r\n"
// Image illustration : https://doc.micrium.com/download/attachments/15714590/chunk_transfer.png?version=1&modificationDate=1424901030000&api=v2

// TODO : response chunked : https://www.codeproject.com/articles/648526/all-about-http-chunked-responses

void Request::parseChunkedBody() {

    size_t      separator = 0;
    std::string tmp;

    tmp.clear();
    //_reqBody.append(client->buf);
    LOGPRINT(INFO, this, ("Request::parseChunkedBody() : Starting chunked body parsing"));
    while (42) {

        separator = _reqBody.find("\r\n", _optiChunkOffset);
        if (separator == std::string::npos) {
            client->recvStatus = Client::ERROR;
            LOGPRINT(LOGERROR, this, ("Request::parseSingleBody() : no <CR><LF> in chunk - invalid request - disconnecting client"));
            break ;
        }
        if (chunkLineBytesSize == -1) {
            tmp = _reqBody.substr(_optiChunkOffset, separator - _optiChunkOffset);
            chunkLineBytesSize = utils::strHexaToDecimal(tmp);
            /* Delete hexa value + "\r\n" */
            _reqBody.erase(_optiChunkOffset, tmp.size() + 2);
            tmp.clear();
        }
        if (chunkLineBytesSize > 0) {
            separator = _reqBody.find("\r\n", _optiChunkOffset);
            if (separator == std::string::npos) {
                LOGPRINT(INFO, this, ("Request::parseChunkedBody() : invalid chunked request"));
                client->recvStatus = Client::ERROR;
                client->res.setErrorParameters(Response::ERROR, BAD_REQUEST_400);
                break ;
            }
            _optiChunkOffset = separator;
            _reqBody.erase(separator, 2);
            chunkLineBytesSize = -1;
        } else if (chunkLineBytesSize == 0) {
            /* END of chunks */
            separator = _reqBody.find("\r\n", _optiChunkOffset);
            /* Delete the last "\r\n" which indicate the end */
            if (separator != std::string::npos)
                _reqBody.erase(separator, 2);
            else {
                LOGPRINT(INFO, this, ("Request::parseChunkedBody() : invalid chunked request"));
                client->recvStatus = Client::ERROR;
                client->res.setErrorParameters(Response::ERROR, BAD_REQUEST_400);
                break ;
            }
            client->recvStatus = Client::COMPLETE;
            break ;
        }
        
    }
    //memset(client->buf, 0, BUFMAX + 1);
    LOGPRINT(INFO, this, ("Request::parseChunkedBody() : End chunked body parsing"));
}

/*
**  Vérification de la taille du body
**  1. Si contentLength est égale à la taille du body alors le parsing de la requete est complete
**  2. Autrement log + ???
*/
void Request::parseSingleBody() {

    size_t      bodySize;

    bodySize = _reqBody.length();
    if (bodySize == contentLength)
    {
        client->recvStatus = Client::COMPLETE;
        LOGPRINT(OK, this, ("Request::parseSingleBody() : Body is fully received ! Header Content-Length = " \
         + std::to_string(contentLength) + " should be equal to Body length, which is : " + std::to_string(bodySize)));
    }
    else // Do something about it ?
        LOGPRINT(INFO, this, ("Request::parseSingleBody() : Body not fully received yet"));
}


/*
**  Vérification de l'existence d'un body
**  1. Si contentLength > 0 ou transferEncoding[0] == "chunked", un body est présent
**  2. Si oui, on récupère toute la requête après le \r\n\r\n
**  3. Sinon, la requête est considérée comme complètement reçue / parsée.
*/
void Request::checkBody()
{
    size_t bodyOffset;

    if (contentLength > 0 || (transferEncoding.size() && transferEncoding[0] == "chunked"))
    {
        client->recvStatus = Client::BODY;
        if (transferEncoding[0] == "chunked")
        {
            LOGPRINT(INFO, this, ("Request::checkBody() : Body sent in chunks"));
        }
        else // Erreur sur le else si pas de {}
        {
            LOGPRINT(INFO, this, ("Request::checkBody() : Body Content-Length = " + std::to_string(contentLength)));
        }
        _reqBody.append(reqBuf);
        if ((bodyOffset = _reqBody.find("\r\n\r\n")) != std::string::npos)
            _reqBody.erase(0, bodyOffset + 1);
    }
    else
        client->recvStatus = Client::COMPLETE;
}

void Request::parse(std::vector<Location*> locations) {

    LOGPRINT(LOGERROR, this->client, "1 -------> " + std::to_string(this->client->res._statusCode));

    parseRequestLine();

    LOGPRINT(LOGERROR, this->client, "2 -------> " + std::to_string(this->client->res._statusCode));

    parseUriQueries();
    LOGPRINT(LOGERROR, this->client, "3 -------> " + std::to_string(this->client->res._statusCode));

    parseFile(locations);
    LOGPRINT(LOGERROR, this->client, "4 -------> " + std::to_string(this->client->res._statusCode));

    parseHeaders();
    LOGPRINT(LOGERROR, this->client, "5 -------> " + std::to_string(this->client->res._statusCode));

    checkBody();
    LOGPRINT(LOGERROR, this->client, "6 -------> " + std::to_string(this->client->res._statusCode));

    std::cout << BLUE << "====================================]\n" << END << std::endl;
    std::cout << _reqBody << std::endl;
    std::cout << BLUE << "====================================]\n" << END << std::endl;

    reqBuf.clear(); 
}

/* **************************************************** */
/*                        LOGGER                        */
/* **************************************************** */

std::string const Request::logInfo(void) {
    std::string ret;
    ret = "Request | From Client with Socket : " + std::to_string(client->acceptFd) + " | Method : " + method + " | URI : " + uri;
    return (ret);
}

void Request::showReq(void) {
    std::string indent("    > ");
    std::cout << std::endl << std::endl;
    std::cout << GREEN << "    REQUEST RECEIVED ----------------" << END;
    std::cout << std::endl << std::endl;
    std::cout << "    ~ Global \n\n";
    std::cout << indent << "Method : " << method << std::endl;
    std::cout << indent << "URI : " << uri << std::endl;
    std::cout << indent << "HTTP Version : " << httpVersion << std::endl;
    std::cout << indent << "Location Assigned : uri = " << reqLocation->uri << " and root = " << reqLocation->root << std::endl;
    showFullHeadersReq();
    std::cout << std::endl;
    std::cout << GREEN << "    ------------------------------- END" << END;
    std::cout << std::endl << std::endl;
}

void Request::showFullHeadersReq(void) {

    std::string indent("    > ");
    std::cout << std::endl;
    std::cout << "    ~ Details";
    std::cout << std::endl << std::endl;
    utils::displayHeaderMultiMap(acceptCharset, (indent + "Accept-Charset"));
    utils::displayHeaderMultiMap(acceptLanguage, (indent + "Accept-Language"));
    utils::displayHeaderMap(contentLanguage, (indent + "Content-Language"));
    utils::displayHeaderMap(transferEncoding, (indent + "Transfer-Encoding"));

    if (!uriQueries.empty())        std::cout << indent << "Queries : " << uriQueries << std::endl;
    if (!authorization.empty())     std::cout << indent << "Authorization : " << authorization << std::endl;
    if (!contentLocation.empty())   std::cout << indent << "Content-Location : " << contentLocation << std::endl;
    if (!contentType.empty())       std::cout << indent << "Content-Type : " << contentType << std::endl;
    if (!date.empty())              std::cout << indent << "Date : " << date << std::endl;
    if (!host.empty())              std::cout << indent << "Host : " << host << std::endl;
    if (!referer.empty())           std::cout << indent << "Referer : " << referer << std::endl;
    if (!userAgent.empty())         std::cout << indent << "User-Agent : " << userAgent << std::endl;
    if (!keepAlive.empty())         std::cout << indent << "Keep-Alive : " << keepAlive << std::endl;
    
    std::cout << std::endl;
    std::cout << indent << "Content-Length : " << std::to_string(contentLength) << std::endl;
    std::cout << indent << "_currentParsedReqBodyLength = " << std::to_string(_currentParsedReqBodyLength) << std::endl;
    std::cout << indent << "Body : " << _reqBody << std::endl;

}
