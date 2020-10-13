# include "../inc/Webserv.hpp"

Request::Request(void) {
    reset();
}

// Keep Track after (no)successfull served response ?

Request::~Request() {
    reset();
}

void Request::reset(void) {

    method.clear();
    uri.clear();
    httpVersion.clear();
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
    _currentParsedReqBodyLength = -1;
    contentLength = -1;
    _reqBody.clear();

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

    for (std::size_t i = 0; i < vecLocs.size(); i++) {
        if (vecLocs[i]->uri == uri) {
            reqLocation = vecLocs[i];
            LOGPRINT(INFO, this, ("Request::assignLocation() : Location assigned"));
            return ;
        }
    }
}

// Example : 
// TMP Location : Location *newLoc1 = new Location("/", "./www", "index.html", "GET");
// Location(std::string uri, std::string root, std::string index, std::string methods) {

void Request::parseFile(std::vector<Location*> locations) {
    int         i;
    struct stat info;

    assignLocation(locations);
    if (reqLocation) {
        i = reqLocation->root.size() - 1;
        if (reqLocation->root[i] == '/')
            file = reqLocation->root;
        else
            file = reqLocation->root + "/";
        if (stat(file.c_str(), &info) == 0) {
            if (S_ISDIR(info.st_mode)) {
                file += reqLocation->index;
            }
        LOGPRINT(INFO, this, ("Request::parseFile() : File Assignedd : " + file));
        }
    }

}

// TODO : case insensitive string comparaison to handle multiple client type

void Request::fillHeader(std::string const key, std::string const value) {

    size_t i = 0;
    std::vector<std::string> multiValues;

    if (key == "Accept-Charset" || key == "Accept-Language" || key == "Content-Language"  || key == "Transfer-Encoding") {
        multiValues = ft::split(value, ',');
        while (!multiValues.empty() && i < multiValues.size()) {
            if (key == "Accept-Charset")
                acceptCharset[i] = multiValues[i];
            else if (key == "Accept-Language")
                acceptLanguage[i] = multiValues[i];
            else if (key == "Content-Language")
                contentLanguage[i] = multiValues[i];
            else if (key == "Transfer-Encoding")
                transferEncoding[i] = multiValues[i];
            i++;
        }
    }
    // else if (key == "Keep-Alive")
    //     keepAlive = value;
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

    std::size_t pos;
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

void Request::parseChunkedBody() {

}

void Request::parseSingleBody() {

    char        *newBodyRead = client->buf;
    size_t      size;

    _reqBody.append(newBodyRead);
    /* We check if body is fully received by comparing its length with the Content-Length header value */
    /* If parsing is perfect, size == contentLength, but for now, we keep a safety with >= */
    size = _reqBody.length();
    _currentParsedReqBodyLength = size;
    if (size >= contentLength) {
        client->recvStatus = Client::COMPLETE;
        LOGPRINT(INFO, this, ("Request::parseSingleBody() : Body is fully received ! Header Content-Length = " \
         + std::to_string(contentLength) + " should be equal to Body length, which is : " + std::to_string(size)));
        memset(newBodyRead, 0, BUFMAX + 1);
    } else {
        LOGPRINT(INFO, this, ("Request::parseSingleBody() : Body not fully received yet"));
        memset(newBodyRead, 0, BUFMAX + 1);
    }

}


void Request::checkBody() {

    size_t bodyOffset;

    // A priori, le seul encoding à gérer pour webserv est le "chunked", mais à confirmer !
    if (contentLength > 0 || transferEncoding[0] == "chunked") {
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


void Request::showFullHeadersReq(void) {

    std::string indent("    > ");
 
    std::cout << std::endl;
    std::cout << "    ~ Details";

    std::cout << std::endl;
    std::cout << std::endl;

    utils::displayHeaderMap(acceptCharset, (indent + "Accept-Charset"));
    utils::displayHeaderMap(acceptLanguage, (indent + "Accept-Language"));
    utils::displayHeaderMap(contentLanguage, (indent + "Content-Language"));
    utils::displayHeaderMap(transferEncoding, (indent + "Transfer-Encoding"));


    if (!uriQueries.empty())
        std::cout << indent << "Queries : " << uriQueries << std::endl;
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
    showBody();
    std::cout << std::endl;
    std::cout << GREEN << "    ------------------------------- END" << END;
    std::cout << std::endl << std::endl;
}

