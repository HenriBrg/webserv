# include "../inc/Webserv.hpp"

Request::Request(void) {
    reset();
}

// Keep Track after (no)successfull served response ?

Request::~Request() {
    reset();
}

void Request::reset(void) {

    client = nullptr;
    reqLocation = nullptr;
    reqBuf.clear();
    _reqBody.clear();

    _currentParsedReqBodyLength = -1;
    contentLength = -1;
    chunkLineBytesSize = -1;
    _optiChunkOffset = 0;
    file.clear();

    method.clear();
    uri.clear();
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
    return (0);
}

// The request line holds the location AND the params : http://www.domain.com/index.html?name1=value1
// We differenciate the storage of uri : http://www.domain.com/index.html
// and the storage of the query : ?name1=value1

void Request::parseUriQueries() {

    // TODO : gérer les failles de sécurités liées aux paramètres d'URL

    int i = 0;
    std::string tmp = uri;

    while (tmp[i] && tmp[i] != '?')
        i++;
    if (tmp[i] == '?') {
        uriQueries = uri.substr(i + 1, tmp.size());
        uri = tmp.substr(0, i);
    }
}


// Location NGinx : http://nginx.org/en/docs/beginners_guide.html
// Faille de sécurité à gérer ultérieurement : Directory Traversal    
// 1) If URI requested match directly one of server's locations, we match here
// TODO : à confirmer mais, si échec de location de la ressource, la location de base "/"" doit être utilisée pour réessayer l'association d'une location à la requête ?

void Request::assignLocation(std::vector<Location*> vecLocs) {

    Location * root;

    for (std::size_t i = 0; i < vecLocs.size(); i++) {

        NOCLASSLOGPRINT(DEBUG, ("Request::assignLocation() : URI = " + uri));
        NOCLASSLOGPRINT(DEBUG, ("Request::assignLocation() : vecLocs[i]->uri = " + vecLocs[i]->uri));
        if (vecLocs[i]->uri == uri) {
            reqLocation = vecLocs[i];
            LOGPRINT(INFO, this, ("Request::assignLocation() : Location directly assigned"));
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
		for (std::size_t x = 0; x < vecLocs.size(); ++x) // Recherche location
        {  
			if (vecLocs[x]->uri == tmpUri)
            {
				file = uri.substr(i + 1, uri.size());
				reqLocation = vecLocs[x];
                LOGPRINT(INFO, this, ("Request::assignLocation() : Location indirectly assigned"));
                return ;
			}
		}
	}

}

void Request::parseFile(std::vector<Location*> locations)
{
    std::string tmpFile;
    int         i;
    struct stat info;

    assignLocation(locations);

    if (reqLocation)
    {
        tmpFile = file;
        i = reqLocation->root.size() - 1;
        if (reqLocation->root[i] == '/')
            file = reqLocation->root;
        else
            file = reqLocation->root + "/";

        if (!(tmpFile.empty())) // Add for PUT REQUEST
            file += tmpFile;

        if (stat(file.c_str(), &info) == 0 && S_ISDIR(info.st_mode))
                file += reqLocation->index;
        else
            LOGPRINT(LOGERROR, this, ("Request::parseFile() : stat() on location failed : "));
        LOGPRINT(DEBUG, this, ("Request::parseFile() : File Assigned : " + file));
    }

}

// TODO : case insensitive string comparaison to handle multiple client type



/* Functions for filling headers value(s) into corresponding variables */

// Main function
void Request::fillHeader(std::string const key, std::string const value) {

    if (key == "Content-Language" || key == "Transfer-Encoding")
        fillMultiValHeaders(key, value);
    else if (key == "Accept-Charset" || key == "Accept-Language")
        fillMultiWeightValHeaders(key, value);
    else
        fillUniqueValHeaders(key, value);
}

// If the variable can have multiple values => map is used to stored them
// Map key is index after split and map value is request value
// Example => Content-Language: de-DE, en-CA
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

// If the variable can only have one value => value is set in corresponding std::string variable
void Request::fillUniqueValHeaders(std::string const key, std::string const value)
{
    if (key == "Keep-Alive")
        keepAlive = value;
    else if (key == "Content-Length")
        contentLength = std::stoi(value);
    // else if (key == "Content-Location")
    //     contentLocation = value;
    else if (key == "Content-Type" || key == "Content-type")
        contentType = value;
    else if (key == "Authorization")
        authorization = value;
    else if (key == "Date")
        date = value;
    else if (key == "Host")
        host = value;
    else if (key == "Referer")
        referer = value;
    else if (key == "User-Agent")
        userAgent = value;
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
            continue ; // Continue or break ? Or error 4XX
        value = ft::trim(line.substr(pos + 1));
        
        // if (value.empty())
        // Continue or break ? Or error 4XX

        utils::deleteCarriageReturn(value);
        fillHeader(key, value);
    }

}


// Really good article
// https://en.wikipedia.org/wiki/Chunked_transfer_encoding
// Example Body = "14\r\nabcdefghijklmnopqrst\r\nA\r\n0123456789\r\n0\r\n\r\n"
// Image illustration : https://doc.micrium.com/download/attachments/15714590/chunk_transfer.png?version=1&modificationDate=1424901030000&api=v2

void Request::parseChunkedBody() {

    // TODO : max body configuration REQUEST_ENTITY_TOO_LARGE_413
    // TODO : trigger error (set response flags) as soon as we see that body is longer that max_body, instead of parsing it entirely directly

    size_t      separator;
    std::string tmp;

    _reqBody.append(client->buf);
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
            if (separator == std::string::npos)
                break ; // ---> Error XXX ?
            _optiChunkOffset = separator;
            _reqBody.erase(separator, 2);
            chunkLineBytesSize = -1;
        } else if (chunkLineBytesSize == 0) {
            /* END of chunks */
            separator = _reqBody.find("\r\n", _optiChunkOffset);
            /* Delete the last "\r\n" which indicate the end */
            if (separator != std::string::npos)
                _reqBody.erase(separator, 2);
            // else ---> Error XXX ?
            client->recvStatus = Client::COMPLETE;
            break ;
        }
        
    }
    memset(client->buf, 0, BUFMAX + 1);
    LOGPRINT(INFO, this, ("Request::parseChunkedBody() : End chunked body parsing"));
}

void Request::parseSingleBody() {

    // TODO : max body configuration and error in response

    size_t      size;
    char        *newBodyRead = client->buf;

    _reqBody.append(newBodyRead);
    /* We check if body is fully received by comparing its length with the Content-Length header value */
    /* If parsing is perfect, size == contentLength, but for now, we keep a safety with >= */
    size = _reqBody.length();
    _currentParsedReqBodyLength = size;
    memset(newBodyRead, 0, BUFMAX + 1);
    if (size >= contentLength) {
        client->recvStatus = Client::COMPLETE;
        LOGPRINT(INFO, this, ("Request::parseSingleBody() : Body is fully received ! Header Content-Length = " \
         + std::to_string(contentLength) + " should be equal to Body length, which is : " + std::to_string(size)));
    } else
        LOGPRINT(INFO, this, ("Request::parseSingleBody() : Body not fully received yet"));

}

void Request::checkBody() {

    size_t bodyOffset;

    // A priori, le seul encoding à gérer pour webserv est le "chunked", mais à confirmer !
    // TODO : si plusieurs encoding, itérer dessus pour matcher "chunked", du moins retourner erreur si gzpi, deflate, car on gèrera pas à priori
    if (contentLength > 0 || (transferEncoding.size() && transferEncoding[0] == "chunked")) {
        client->recvStatus = Client::BODY;
        if (transferEncoding[0] == "chunked") {
            LOGPRINT(INFO, this, ("Request::checkBody() : Body sent in chunks"));
        } else
            LOGPRINT(INFO, this, ("Request::checkBody() : Body Content-Length = " + std::to_string(contentLength)));
        _reqBody = std::string(client->buf);
        bodyOffset = _reqBody.find("\r\n\r\n");
        _reqBody.erase(0, bodyOffset + 4);
        // Headers already parsed from that point, next body will be read within client->buf and appened in parseChunkedBody() or parseSingleBody()
        memset(client->buf, 0, BUFMAX + 1);
    } else
        client->recvStatus = Client::COMPLETE;

}

// TODO : Parsing Chunked

void Request::parse(std::vector<Location*> locations) {

    parseRequestLine();
    parseUriQueries();
    parseFile(locations);
    parseHeaders();
    checkBody();
    reqBuf.clear(); 
}

// UTILS

std::string const Request::logInfo(void) {
    std::string ret;
    ret = "Request | From Client with Socket : " + std::to_string(client->acceptFd) + " | Method : " + method + " | URI : " + uri + \
    " | Location Assigned : " + reqLocation->uri;
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

    std::cout << std::endl;
    std::cout << std::endl;

    //utils::displayHeaderMap(acceptCharset, (indent + "Accept-Charset"));
    //utils::displayHeaderMap(acceptLanguage, (indent + "Accept-Language"));
    utils::displayHeaderMap(contentLanguage, (indent + "Content-Language"));
    utils::displayHeaderMap(transferEncoding, (indent + "Transfer-Encoding"));


    //if (!uriQueries.empty())
    //    std::cout << indent << "Queries : " << uriQueries << std::endl;
    if (!authorization.empty())
        std::cout << indent << "Authorization : " << authorization << std::endl;
    if (!contentLocation.empty())
        std::cout << indent << "Content-Location : " << contentLocation << std::endl;
    if (!contentType.empty())
        std::cout << indent << "Content-Type : " << contentType << std::endl;
    if (!date.empty())
        std::cout << indent << "Date : " << date << std::endl;
    if (!host.empty())
        std::cout << indent << "Host : " << host << std::endl;
    if (!referer.empty())
        std::cout << indent << "Referer : " << referer << std::endl;
    if (!userAgent.empty())
        std::cout << indent << "User-Agent : " << userAgent << std::endl;
    if (!keepAlive.empty())
        std::cout << indent << "Keep-Alive : " << keepAlive << std::endl;
    
    std::cout << std::endl;

    std::cout << indent << "Content-Length : " << std::to_string(contentLength) << std::endl;
    std::cout << indent << "_currentParsedReqBodyLength = " << std::to_string(_currentParsedReqBodyLength) << std::endl;

    std::cout << indent << "Body : " << _reqBody << std::endl;

}

void Request::showBody() {
    std::string indent("    > ");
    std::cout << std::endl;
    // std::cout << indent << "(Server) Body Length : " << std::to_string(bodyLength) << std::endl;
}
