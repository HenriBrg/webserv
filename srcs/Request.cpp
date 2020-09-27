# include "../inc/Webserv.hpp"

Request::Request(void) {

    method.clear();
    uri.clear();
    httpVersion.clear();
    acceptCharset.clear();
    acceptLanguage.clear();
    authorization.clear();
    contentLanguage.clear();
    contentLength = -1;
    contentLocation.clear();
    contentType.clear();
    date.clear();
    host.clear();
    referer.clear();
    userAgent.clear();
    body.clear();
    transferEncoding.clear();
    bodyLength = -1;
    
}

Request::~Request() {

}


// PARSING - No member methods
// TODO : handle REQUEST LINE wrote in several time (for example, with telnet, "GET" + hit enter + "/ HTTP/1.0" + double hit enter, must work)

void	reqGetLine(std::string & buf, std::string & line) {
    
    size_t pos;

    pos = buf.find("\n");
    if (pos != std::string::npos) {
        line = std::string(buf, 0, pos++);
        buf = buf.substr(pos);
    } else {
        if (buf[buf.size() - 1] == '\n')
            buf = buf.substr(buf.size());
        else
        {
            line = buf;
            buf = buf.substr(buf.size());
        }
    }
}

std::vector<std::string> split(const std::string & str, char delim) {

    size_t                      start;
    size_t                      end = 0;
    std::vector<std::string>    strings;

    // std::string::find_first_not_of searches the string for the first character that
    // does not match any of the characters specified in its arguments.

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
    }
    return strings;
}

int Request::parseRequestLine() {

    std::string                 line;
    std::vector<std::string>    tab;

    reqGetLine(reqBuf, line);
    tab = split(line, ' ');
    if (tab.size() != 3)
        return (-1);

    method      = tab[0];
    uri         = tab[1];
    httpVersion = tab[2];

    return (0);
}

// -----------

// The request line holds the location AND the params : http://www.domain.com/index.html?name1=value1
// We differenciate the storage of uri : http://www.domain.com/index.html
// and the storage of the query : ?name1=value1

void Request::parseQuery() {

    int i = 0;
    std::string tmp = uri;

    while (tmp[i] && tmp[i] != '?')
        i++;
    if (tmp[i] == '?') {
        query = uri.substr(i + 1, tmp.size());
        uri = tmp.substr(0, i);
    }

    // TODO : stocker les params dans un vector<pair> ou map
}


void Request::assignLocation(std::vector<Location*> vecLocs) {

    // Location NGinx : http://nginx.org/en/docs/beginners_guide.html
    // Faille de sécurité connue : Directory Traversal --> à gérer ultérieurement

    // 1) If URI requested match directly one of server's locations, we match here

    for (std::size_t i = 0; i < vecLocs.size(); i++) {
        if (vecLocs[i]->uri == uri) {
            reqLocation = vecLocs[i];
            // Point à éclaicir sur le le path de la ressource
            return ;
        }
    }

    // 2) Else, if no direct match, we iterate on every folder given by the URI (for example, "GET /tmp/data/site/ ..." 
    // and for each of them, we check if the server has one matching location
    // std::string tmp(uri);
    // int i = tmp.size() - 1;
    // i = (i < 0) ? 0 : i;
    // while (tmp.size() > 0) {
    //     while (i && tmp[i] != '/')
    //         i--;
    //     tmp = tmp.substr(0, i);
    //     if (tmp.empty())
    //         tmp = "/";
    //     for (std::size_t i = 0; i < vecLocs.size(); i++) {
    //         if (vecLocs[i]->uri == tmp) {
    //             file = tmp.substr(i + 1, tmp.size()); // BAD, need to assign the file pointed by index/root parameter in location
    //             reqLocation = vecLocs[i];
    //             return ;

    //         }
    //     }
    // }
    // TODO : à confirmer mais, si échec de location de la ressource, la location de base "/"" doit être utilisée pour réessayer l'association d'une location à la requête (voir lien ci-dessus)
}

int Request::parseFile(std::vector<Location*> locations) {

    // TMP Location : Location *newLoc1 = new Location("/", "./www", "index.html", "GET");

    // Location(std::string uri, std::string root, std::string index, std::string methods) {

    int         i;
    struct stat info;

    parseQuery();
    assignLocation(locations);

    if (reqLocation) {
        
        i = reqLocation->root.size() - 1;
        if (reqLocation->root[i] == '/')
            file = reqLocation->root + file;
        else
            file = reqLocation->root + "/" + file;
        if (stat(file.c_str(), &info) == 0) {
            if (S_ISDIR(info.st_mode)) {
                i = file.size() - 1;
                if (file[i] == '/')
                    file += reqLocation->index;
                else
                    file = file + "/" + reqLocation->index;
                }
        }

        LOGPRINT(DEBUG, this->client, ("Request::parseFile() : File Assignedd : " + file));

        return (0);
    }
    return (-1); // Erreur 4XX
}

// À ce stade, on supposera une requête GET très basique : 
// GET /index.html HTTP/1.0

void Request::parse(std::vector<Location*> locations) {
    
    parseRequestLine();
    parseFile(locations);
    // TODO 1 : On skip les headers pour le moment
    // TODO 2 : Parsing du body et gestion des requêtes chunked + tard
    showReq();

}

void Request::showReq(void) {

    std::cout << "ReqBuf : " << reqBuf << std::endl;
    std::cout << "Method : " << method << std::endl;
    std::cout << "URI : " << uri << std::endl;
    std::cout << "HTTP Version : " << httpVersion << std::endl;
    std::cout << "Query : " << query << std::endl;
    std::cout << "File : " << file << std::endl;

}