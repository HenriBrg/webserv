# include "../inc/Webserv.hpp"

void Response::getReq(Request * req) {

    int cgiUp = 0;
	struct stat	buffer;

    negotiateAcceptLanguage(req);
    negotiateAcceptCharset(req);
    if (stat(req->file.c_str(), &buffer) == -1) {
        LOGPRINT(LOGERROR, this, ("Response::getReq() : The function stat() has returned -1 on the requested file which is "  + req->file));
        return setErrorParameters(req, Response::ERROR, NOT_FOUND_404);
    }

    if ((!req->reqLocation->cgi.empty() && utils::isExtension(req->file, ".cgi")) || (!req->reqLocation->phpcgi.empty() && utils::isExtension(req->file, ".php")))
        cgiUp = 1;

    // TMP
    cgiUp = 0;

    if (cgiUp) {
        execCGI(req);
        _cgiOutputFile = "./www/tmpFile";
        _resFile = req->file;
        _statusCode = OK_200;

    } else {

        LOGPRINT(INFO, this, ("Response::getReq() : No CGI required for this GET request, we handle by ourselves the response"));
        // For testing in advance, not the right place
        _resFile = req->file;
        _statusCode = OK_200;

        // TODO : ouvrir fichier

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