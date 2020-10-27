# include "../inc/Webserv.hpp"

void Response::getReq(Request * req) {

    int cgiUp = 0;
	struct stat	buffer;

    negotiateAcceptLanguage(req);
    negotiateAcceptCharset(req);
    if (stat(req->file.c_str(), &buffer) == -1)
        return setErrorParameters(req, Response::ERROR, NOT_FOUND_404);
  
    if (stat(req->reqLocation->cgiPath.c_str(), &buffer) == 0)
        if ((buffer.st_mode & S_IEXEC) != 0)
            cgiUp = 1;
    
    cgiUp = 0; // ---------------------------------> Temporary to test without CGI !
    if (cgiUp == 1) {



    } else {

        // For testing in advance, not the right place
        _resFile = req->file;
        _statusCode = OK_200;

    }


    // LOGPRINT(INFO, this, ("Response::getReq() : CGI DOWN"));
    // LOGPRINT(INFO, this, ("Response::getReq() : CGI UP"));
    // LOGPRINT(INFO, this, ("Response::getReq() : Succesfull GET Request"));

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
    LOGPRINT(LOGERROR, this, ("Response::negotiateAcceptLanguage() : Unknow Language"));
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
    LOGPRINT(LOGERROR, this, ("Response::negotiateAcceptCharset() : Unknow Charset"));
    //setErrorParameters(req, Response::ERROR, BAD_REQUEST_400); => ERROR OR IGNORE ?
}