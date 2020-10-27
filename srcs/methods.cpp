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

void Response::putReq(Request * req) {

}

void Response::postReq(Request * req) {

}

void Response::patchReq(Request * req) {

}

void Response::deleteReq(Request * req) {

}



/* https://developer.mozilla.org/fr/docs/Web/HTTP/Content_negotiation */

void Response::negotiateAcceptLanguage(Request * req) {

    int i = 0;
    size_t x;
    std::string tmp;
    std::string backup = req->file;

    if (req->acceptCharset.empty())
        return ;
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

    if (req->acceptCharset.empty())
        return ;
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
        setErrorParameters(req, Response::ERROR, BAD_REQUEST_400);

    }

}
