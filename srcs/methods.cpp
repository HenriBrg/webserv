# include "../inc/Webserv.hpp"

void Response::getReq(Request * req) {

    LOGPRINT(INFO, this, ("Response::getReq() : Calling GET Request Function"));

    negotiateAcceptLanguage(req);
    negotiateAcceptCharset(req);
    
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
        sendStatus = Response::ERROR;
        statusCode = BAD_REQUEST_400;
        _errorFileName = "error.html"; /* TO UPDATE when we will have one html file per error */

    }

}
