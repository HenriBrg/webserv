# include "../inc/Webserv.hpp"

int Response::getCGIType(Request * req) {
    if (!req->reqLocation->ext.empty() && !req->reqLocation->cgi.empty() && req->uri.find(req->reqLocation->ext) != std::string::npos)
        return (TESTER_CGI);
    else if (!req->reqLocation->php.empty() && utils::isExtension(req->file, ".php"))
        return (PHP_CGI);
    else
        return (NO_CGI);
}

void Response::getReq(Request * req) {

    int cgiType = NO_CGI;
	struct stat	buffer;

    // 1) Client/Server Negotiation
    negotiateAcceptLanguage(req);
    negotiateAcceptCharset(req);

    // 2) Check if requested file exist (after having negotiate on it) 
    if (stat(req->file.c_str(), &buffer) == -1) {
        LOGPRINT(INFO, this, ("Response::getReq() : The requested file ( " + req->file + " ) doesn't exist, stat() has returned -1 on it"));
        return setErrorParameters(req, Response::ERROR, NOT_FOUND_404);
    }

    // 3) Determine CGI type (no cgi = 0, 42 cgi = 1, php-cgi = 2) and perform it
    cgiType = getCGIType(req);
    if (cgiType) {
        LOGPRINT(INFO, req, ("Response::getReq() : CGI is required to handle that request - Its type is " + std::to_string(cgiType) + " (1 = 42-CGI and 2 = PHP-CGI)"));
        execCGI(req);
        parseCGIHeadersOutput(req);
        std::ifstream tmpFile("./www/tmpFile");
		std::string buffer((std::istreambuf_iterator<char>(tmpFile)), std::istreambuf_iterator<char>());
        _cgiOutputBody = buffer;
        remove("./www/tmpFile");
        LOGPRINT(INFO, this, ("Response::getReq() : CGI has been performed ! to handle that request - Its type is " + std::to_string(cgiType) + " (1 = 42-CGI and 2 = PHP-CGI)"));
    } else {
        LOGPRINT(INFO, this, ("Response::getReq() : No CGI required for this GET request, we handle the response by ourselve"));
        _resFile = req->file;
        _statusCode = OK_200;
        lastModified = ft::getLastModifDate(_resFile);
    }
}


void Response::headReq(Request * req) {

}

void Response::putReq(Request * req)
{
    int fileFd(0);

    if ((fileFd = open(req->file.c_str(), O_TRUNC | O_CREAT, 0777)) != -1)
    {
        write(fileFd, req->_reqBody.c_str(), req->_reqBody.size());
    }
    // Else manage error

}

void Response::postReq(Request * req) {

    // tmp for test
    getReq(req);

}

void Response::patchReq(Request * req) {

}

void Response::deleteReq(Request * req) {

}



/* https://developer.mozilla.org/fr/docs/Web/HTTP/Content_negotiation */

void Response::negotiateAcceptLanguage(Request * req)
{
    if (req->acceptLanguage.empty())
        return ;

    struct stat fileStat;
    std::string path;

    std::multimap<float, std::string, std::greater<float> >::iterator it(req->acceptLanguage.begin());
    std::multimap<float, std::string, std::greater<float> >::iterator ite(req->acceptLanguage.end());
    for (; it != ite; it++)
    {
        if ((*it).second != "*")
            path = req->file + "." + (*it).second;
        else
            path = req->file;
        if (stat(path.c_str(), &fileStat) == -1)
            continue;
        req->file = path;
        return ;
    }
    LOGPRINT(INFO, this, ("Response::negotiateAcceptLanguage() : Unknow Language"));
    //setErrorParameters(req, Response::ERROR, BAD_REQUEST_400); => ERROR OR IGNORE ?
}

void Response::negotiateAcceptCharset(Request * req)
{
    if (req->acceptCharset.empty())
        return ;

    std::multimap<float, std::string, std::greater<float> >::iterator it(req->acceptCharset.begin());
    std::multimap<float, std::string, std::greater<float> >::iterator ite(req->acceptCharset.end());
    for (; it != ite; it++)
    {
        if ((*it).second == "utf-8" || (*it).second == "*")
            return ;
    }
    LOGPRINT(INFO, this, ("Response::negotiateAcceptCharset() : Unknow Charset"));
    //setErrorParameters(req, Response::ERROR, BAD_REQUEST_400); => ERROR OR IGNORE ?
}