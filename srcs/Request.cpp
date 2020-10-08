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
    body.clear();
    transferEncoding.clear();
    bodyLength = -1;
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
        query = uri.substr(i + 1, tmp.size());
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

void Request::fillHeader(std::string const key, std::string const value) {

    size_t i = 0;
    std::vector<std::string> multiValues;

    if (key == "Accept-Charset" || key == "Accept-Language" || key == "Content-Language") {
        multiValues = ft::split(value, ',');
        while (!multiValues.empty() && i < multiValues.size()) {
            if (key == "Accept-Charset")
                acceptCharset[i] = multiValues[i];
            else if (key == "Accept-Language")
                acceptLanguage[i] = multiValues[i];
            else if (key == "Content-Language")
                contentLanguage[i] = multiValues[i];
            i++;
        }
    }
    else if (key == "Keep-Alive")
        keepAlive = value;
    else if (key == "Content-Length")
        contentLength = std::stoi(value);
    else if (key == "Content-Location")
        contentLocation = value;
    else if (key == "Content-Type")
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
    else if (key == "Transfer-Encoding")
        transferEncoding = value;

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
            continue ;
        value = ft::trim(line.substr(pos + 1));
        utils::deleteCarriageReturn(value);
        fillHeader(key, value);
    }

}

void Request::parseBody() {

}

void Request::checkBody() {

    size_t bodyOffset;

    // A priori, le seul encoding à gérer pour webserv est le "chunked", mais à confirmer !

    if (contentLength > 0 || transferEncoding == "chunked") {

        if (transferEncoding == "chunked") {
            LOGPRINT(INFO, this, ("Request::checkBody() : Body sent in ckunks"));
        } else
            LOGPRINT(INFO, this, ("Request::checkBody() : Body not chunked | Content-Length = " + std::to_string(contentLength)));
        
        client->recvStatus = Client::BODY;
        _reqBody = std::string(reqBuf);
        bodyOffset = _reqBody.find("\r\n\r\n");
        _reqBody.erase(0, bodyOffset + 4);
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
}

// UTILS

std::string const Request::logInfo(void) {
    std::string ret;
    ret = "Request Logger | From Client with Socket : " + std::to_string(client->acceptFd) + " | Method : " + method + " | URI : " + uri + \
    " | Location Assigned : " + reqLocation->uri;
    return (ret);
}

void Request::showReq(void) {

    std::cout << std::endl;
    std::cout << "ReqBuf : " << reqBuf << std::endl;
    std::cout << "Method : " << method << std::endl;
    std::cout << "URI : " << uri << std::endl;
    std::cout << "HTTP Version : " << httpVersion << std::endl;
    std::cout << "Query : " << query << std::endl;
    std::cout << "File : " << file << std::endl;
    std::cout << std::endl;

}

