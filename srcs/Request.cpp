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

void	reqGetLine(std::string & buf, std::string & line) {
    
    size_t pos;

    pos = buf.find("\n");
    if (pos != std::string::npos) {
        line = std::string(buf, 0, pos++);
        buf = buf.substr(pos);
    }
    else {
        if (buf[buf.size() - 1] == '\n')
            buf = buf.substr(buf.size());
        else {
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

// À ce stade, on supposera une requête GET très basique : 
// GET /index.html HTTP/1.0

void Request::parse(std::vector<Location*> locations) {
    
    parseRequestLine();
}

void Request::showReq(void) {

    std::cout << "PARSE DONE" << std::endl;

    std::cout << "1) Method : " << method << std::endl;
    std::cout << "2) URI : " << uri << std::endl;
    std::cout << "3) HTTP Version : " << httpVersion << std::endl;

}