# include "../inc/Webserv.hpp"

void Response::getReq(Request * req) {

    int cgiUp = 0;
	struct stat	buffer;

    negotiateAcceptLanguage(req);
    negotiateAcceptCharset(req);
    if (stat(req->file.c_str(), &buffer) == 0)
        return setErrorParameters(req, Response::ERROR, NOT_FOUND_404);
   
    if (stat(req->reqLocation->cgiPath.c_str(), &buffer) == 0)
        if ((buffer.st_mode & S_IEXEC) != 0)
            cgiUp = 1;
    
    cgiUp = 0; // ---------------------------------> Temporary to test without CGI !
    if (cgiUp == 1) {



    } else {

        _statusCode = OK_200;

    }


    // LOGPRINT(INFO, this, ("Response::getReq() : CGI DOWN"));
    // LOGPRINT(INFO, this, ("Response::getReq() : CGI UP"));
    // LOGPRINT(INFO, this, ("Response::getReq() : Succesfull GET Request"));

}

void Response::headReq(Request * req) {
    // std::cout << "void Response::headReq(Request * req)" << std::endl;

}

void Response::putReq(Request * req) {
    // std::cout << "void Response::putReq(Request * req)" << std::endl;

}

void Response::postReq(Request * req) {
    // std::cout << "void Response::postReq(Request * req)" << std::endl;

}

void Response::connectReq(Request * req) {
    // std::cout << "void Response::connectReq(Request * req)" << std::endl;

}

void Response::traceReq(Request * req) {
    // std::cout << "void Response::traceReq(Request * req)" << std::endl;

}

void Response::optionsReq(Request * req) {
    // std::cout << "void Response::optionsReq(Request * req)" << std::endl;

}

void Response::deleteReq(Request * req) {
    // std::cout << "void Response::deleteReq(Request * req)" << std::endl;

}



/* https://developer.mozilla.org/fr/docs/Web/HTTP/Content_negotiation */

void Response::negotiateAcceptLanguage(Request * req) {

    int i = 0;
    size_t x;
    std::string tmp;
    std::string backup = req->file;

    while (i < req->acceptLanguage.size()) {
        if (req->acceptLanguage[i].empty())
            continue ;
        tmp = req->acceptLanguage[i];
        x = tmp.find(';');
        if (x != std::string::npos)
            tmp = tmp.substr(0, x);
        req->file = req->file + "." + tmp;
        std::ifstream tmpFile(req->file);
        if (tmpFile.good()) {
            tmpFile.close();
            return ;
        } else
            req->file = backup;
        i++;
    }
}

void Response::negotiateAcceptCharset(Request * req) {

    int i = 0;
    int isUTF8 = 0;

    while (i < req->acceptCharset.size()) {
        if (req->acceptLanguage[i].empty())
            if (req->acceptLanguage[i] == "utf-8")
                isUTF8 = 1;
        i++;
    }
    if (isUTF8)
        return ;
    else {
        LOGPRINT(LOGERROR, this, ("Response::negotiateAcceptCharset() : Unknow Charset"));
        _sendStatus = Response::ERROR;
        _statusCode = BAD_REQUEST_400;
        _errorFileName = "error.html"; /* TO UPDATE when we will have one html file per error */

    }

}
