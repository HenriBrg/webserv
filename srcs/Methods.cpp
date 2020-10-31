# include "../inc/Webserv.hpp"

void Response::getReq(Request * req) {

	struct stat	buffer;

    negotiateAcceptLanguage(req);
    negotiateAcceptCharset(req);
    NOCLASSLOGPRINT(INFO, "Language and Charset negotiation done");

    if (stat(req->file.c_str(), &buffer) == -1) {
        LOGPRINT(INFO, this, ("Response::getReq() : The requested file ( " + req->file + " ) doesn't exist, stat() has returned -1 on it"));
        return setErrorParameters(Response::ERROR, NOT_FOUND_404);
    }

    req->cgiType = getCGIType(req);
    if (req->cgiType) {
        LOGPRINT(INFO, req, ("Response::getReq() : One CGI is required to handle that request - Its type is " + std::to_string(req->cgiType) + " (1 = 42-CGI and 2 = PHP-CGI)"));
        execCGI(req);
        LOGPRINT(INFO, this, ("Response::getReq() : CGI has been performed !"));
    } else {
        LOGPRINT(INFO, this, ("Response::getReq() : No CGI required for this GET request, we handle the response by ourselve"));
        _resFile = req->file;
        _statusCode = OK_200;
        lastModified = ft::getLastModifDate(_resFile);
    }
}

void Response::headReq(Request * req) {
    getReq(req);
}

void Response::postReq(Request * req) {
    
    getReq(req);

}


void Response::putReq(Request * req)
{
    struct stat fileStat;
    int fileFd(0);
    bool isCreated(true);

    if (stat(req->resource.c_str(), &fileStat) == 0)
    {
        if (S_ISDIR(fileStat.st_mode))
            return (setErrorParameters(Response::ERROR, CONFLICT_409));
        isCreated = false;
    }
    if ((fileFd = open(req->file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0600)) != -1)
    {
        if (write(fileFd, req->_reqBody.c_str(), req->_reqBody.size()) != -1)
        {
            LOGPRINT(INFO, req, ("Response::putReq() : write() body in " + req->file + " DONE"));
            if (isCreated)
            {
                _statusCode = CREATED_201;
                _resFile = req->file; // On 201 reponse client waits for newly created ressource
            }
            else
                _statusCode = NO_CONTENT_204;
            close(fileFd);
            return ;
        }
    }
    setErrorParameters(Response::ERROR, INTERNAL_ERROR_500);
    LOGPRINT(LOGERROR, req, ("Response::putReq() : failed fileFd = " + std::to_string(fileFd)));
}

void Response::patchReq(Request * req) {

}

void Response::deleteReq(Request * req)
{
    struct stat fileStat;

    if (stat(req->resource.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode))
        deleteDir(req->resource);
    else
        unlink(req->file.c_str());
    
}

void Response::deleteDir(std::string directory)
{
    // Add error management on syscall
    DIR *dir;
    struct dirent *dirEntry;
    std::string entryName;

    if (!(dir = opendir(directory.c_str())))
    {
        LOGPRINT(LOGERROR, this, ("Request::deleteDir() : Open directory : " + directory + "failed"));
        return ;
    }
    while ((dirEntry = readdir(dir)))
    {
        entryName.clear();
        entryName.append(dirEntry->d_name);
        if (entryName == "." || entryName == "..")
            continue ;
        else if (dirEntry->d_type == DT_DIR)
            deleteDir((directory + entryName));
        else
            unlink((directory + "/" + entryName).c_str());
    }
    rmdir(directory.c_str());
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
    //setErrorParameters(Response::ERROR, BAD_REQUEST_400); => ERROR OR IGNORE ?
}