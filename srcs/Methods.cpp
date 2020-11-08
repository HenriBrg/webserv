# include "../inc/Webserv.hpp"

/*
**  GET Request handler 
**  1. Negotiate Accept-Language and Accept-Charset
**  2. We check if the requested file exist
**  3. We call the CGI defined it the location if the requested file extension match the "ext" location paremeter
**      3.1. We parse its output
*/

void Response::getReq(Request * req) {

	struct stat	buffer;

    negotiateAcceptLanguage(req);
    negotiateAcceptCharset(req);
    NOCLASSLOGPRINT(INFO, "GET - Language and Charset negotiation done");
    bzero(&buffer, sizeof(buffer));
    if (stat(req->file.c_str(), &buffer) == -1) {
        LOGPRINT(INFO, this, ("Response::getReq() : GET - The requested file ( " + req->file + " ) doesn't exist, stat() has returned -1 on it"));
        return setErrorParameters(Response::ERROR, NOT_FOUND_404);
    }
    req->cgiType = getCGIType(req);
    if (req->cgiType == TESTER_CGI || req->cgiType == PHP_CGI) {
        LOGPRINT(INFO, req, ("Response::getReq() : GET - CGI is required to handle that request - Its type is " + std::to_string(req->cgiType) + " (1 = 42-CGI and 2 = PHP-CGI)"));
        execCGI(req);
        LOGPRINT(INFO, this, ("Response::getReq() : GET - CGI has been performed !"));
    } else {
        LOGPRINT(INFO, this, ("Response::getReq() : GET - No CGI required for this GET request, we handle the response by ourselve"));
        _resFile = req->file;
        _statusCode = OK_200;
        lastModified = ft::getLastModifDate(_resFile);
    }
}

/*
**  HEAD Request handler
**  It's a simple GET, except that we will cut the body when formating the response
*/

void Response::headReq(Request * req) {
    getReq(req);
}

/*
**  POST Request handler
**  1. We call the CGI defined if needed (i.e. the requested file extension match the "ext" location parameter)
**  
**  2. Else, we handle by ourselve the POST request. We check if the requested file exist
**  3. If yes, the request will edit the file, else it create the file, both with the body given by the client
**
**  Usefull LOGs :
**  NOCLASSLOGPRINT(DEBUG, " isolateFileName --> " + req->isolateFileName + " and resource --> " + req->resource);
*/

void Response::postReq(Request * req) {

    std::string str;
    int ret;
    int fd = -1;
	struct stat	buffer;
    int action = 0;

    req->cgiType = getCGIType(req);
     if (req->cgiType == TESTER_CGI || req->cgiType == PHP_CGI) {
        LOGPRINT(INFO, req, ("Response::postReq() : POST - CGI is required to handle that request - Its type is " + std::to_string(req->cgiType) + " (1 = 42-CGI and 2 = PHP-CGI)"));
        execCGI(req);
        LOGPRINT(INFO, this, ("Response::postReq() : POST - CGI has been performed !"));
    } else if (req->cgiType == NO_CGI) {
        
        if (stat(req->file.c_str(), &buffer) == -1) {
            action = CREATE;
            LOGPRINT(INFO, this, ("Response::postReq() : POST - stat(" + req->file + ") has returned -1 meaning that the resource doesn't exist, thus, we create it."));
        } else {
            action = UPDATE;
            LOGPRINT(INFO, this, ("Response::postReq() : POST - stat(" + req->file + ") succeed, meaning that the resource already exist, thus, we update it."));
        }
        if ((fd = open(req->file.c_str(), (action == CREATE ? O_CREAT : O_APPEND) | O_WRONLY, 0666)) == -1) {
            LOGPRINT(INFO, this, ("Response::postReq() : POST - Could not open the file (" + req->file + "), open() has returned -1 on it. "));
            return setErrorParameters(Response::ERROR, INTERNAL_ERROR_500);
        }
        if ((ret = write(fd, req->_reqBody.c_str(), req->_reqBody.size())) <= 0)
            LOGPRINT(INFO, this, ("Response::postReq() : POST without CGI - write() _reqBody has returned ret = " + std::to_string(ret)));
        close(fd);
        _statusCode = action == CREATE ? CREATED_201 : OK_200;
        str = action == CREATE ? "201 - SUCCESSFULL POST REQUEST - CREATED FILE : " + req->file : "200 - SUCCESSFULL POST REQUEST - UPDATED FILE : " + req->file;
        _resBody = responseUtils::setBodyNoFile(str, str.size(), contentLength);
        lastModified = ft::getLastModifDate(req->file);
        LOGPRINT(INFO, this, ("Response::postReq() : POST - Successfull POST request"));
    } 

}

void Response::putReq(Request * req)
{
    struct stat fileStat;
    int fileFd(0);
    bool isCreated(true);

    if (stat(req->resource.c_str(), &fileStat) == 0)
        isCreated = false;
    if ((fileFd = open(req->file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0600)) != -1)
    {
        if (write(fileFd, req->_reqBody.c_str(), req->_reqBody.size()) != -1)
        {
            LOGPRINT(INFO, req, ("Response::putReq() : write() body in " + req->file + " DONE"));
            if (isCreated)
            {
                /* On 201 reponse client waits for newly created ressource */
                _statusCode = CREATED_201;
                _resFile = req->file;
            }
            else
                _statusCode = OK_200;
            close(fileFd);
            return ;
        }
    }
    setErrorParameters(Response::ERROR, INTERNAL_ERROR_500);
    LOGPRINT(LOGERROR, req, ("Response::putReq() : failed fileFd = " + std::to_string(fileFd)));
}

void Response::patchReq(Request * req) {
    (void)req;
}

/*
**  Delete method function
**  1. If target is a directory then call deleteDir()
**  2. Else unlink target
*/

void Response::deleteReq(Request * req) {
    
    struct stat fileStat;

    if (stat(req->resource.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode))
        deleteDir(req->resource);
    else
        unlink(req->file.c_str());
    _statusCode = NO_CONTENT_204;
    
}

/*
**  Delete method function
**  1. Open directory
**  2. Browse directory
**  3. Ignore current and previous directory
**  4. If entry is a directory call function on it
**  5. Else unlink entry
**  6. When no more entry in directory delete it
*/

void Response::deleteDir(std::string directory) {

    DIR *dir;
    struct dirent *dirEntry;
    std::string entryName;

    if (!(dir = opendir(directory.c_str()))) {
        LOGPRINT(LOGERROR, this, ("Request::deleteDir() : Open directory : " + directory + "failed"));
        return ;
    }
    while ((dirEntry = readdir(dir))) {
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
    closedir(dir);
}


/*
**  Negociation on languages
**  1. If header in request
**  2. Browse all value of Accept-Language header
**  3. Seek for language extension of file (example.html.fr or example.html.de)
**  4. if file then set as reponse file
*/

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
        _isLanguageNegociated = true;
        return ;
    }
    LOGPRINT(INFO, this, ("Response::negotiateAcceptLanguage() : Unknow Language"));
}

/*
**  Negociation on charset
**  1. If header in request
**  2. Seek for utf-8 or * (* is everything) in values of Accept-charset header
*/

void Response::negotiateAcceptCharset(Request * req)
{
    if (req->acceptCharset.empty())
        return ;

    std::multimap<float, std::string, std::greater<float> >::iterator it(req->acceptCharset.begin());
    std::multimap<float, std::string, std::greater<float> >::iterator ite(req->acceptCharset.end());
    for (; it != ite; it++)
    {
        if ((*it).second == "utf-8" || (*it).second == "*")
        {
            if ((*it).first == 0)
                setErrorParameters(Response::ERROR, NOT_ACCEPTABLE_406);
            return ;
        }
    }
    LOGPRINT(INFO, this, ("Response::negotiateAcceptCharset() : Unknow Charset"));
}