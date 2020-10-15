# include "../inc/Webserv.hpp"

Response::Response(void) {
    reset();
}

void Response::reset() {
    httpVersion.clear();
    signification.clear();
    allowedMethods.clear();
    contentLanguage.clear();
    contentLocation.clear();
    contentType.clear();
    lastModified.clear();
    location.clear();
    date.clear();
    retryAfter.clear();
    server.clear();
    transfertEncoding.clear();
    wwwAuthenticate.clear();
    finalResponse.clear();

    statusCode = -1;
    contentLength = -1;
    sendStatus = Response::PREPARE;
    methodFctPtr = nullptr;

}

Response::~Response() {

}

// TODO : avoid doing function mapping for each requests

int Response::requestValidition(Request * req) {

    std::vector<std::string>    allowedMethods;
    std::vector<std::string>::iterator    tmp;

    typedef void	(Response::*ptr)(Request * req);
	std::map<std::string, ptr> tab;

    tab["GET"] = &Response::getReq;
	tab["HEAD"] = &Response::headReq;
	tab["PUT"] = &Response::putReq;
	tab["POST"] = &Response::postReq;
	tab["CONNECT"] = &Response::connectReq;
	tab["TRACE"] = &Response::traceReq;
	tab["OPTIONS"] = &Response::optionsReq;
	tab["DELETE"] = &Response::deleteReq;

    allowedMethods = ft::split(req->reqLocation->methods, ',');
    tmp = std::find(allowedMethods.begin(), allowedMethods.end(), req->method);
    if (tmp == allowedMethods.end()) {
        sendStatus = Response::ERROR;
        LOGPRINT(LOGERROR, this, ("Response::controlRequestValidity() : Method " + req->method + " is not allowed on route " + req->reqLocation->uri));
        return (-1);
    } else
        methodFctPtr = tab[req->method];
    if (methodFctPtr != nullptr)

    tab.clear();
    return (0);
}

void Response::resDispatch(Request * req) {

    if (requestValidition(req) == -1)
        return ;
    (this->*methodFctPtr)(req);

}

std::string const Response::logInfo(void) {
    std::string ret;
    ret = "Response | To Client with Socket : " + std::to_string(resClient->acceptFd) + " | Method : " + resClient->req.method + " | Response Status : " + std::to_string(sendStatus);
    return (ret);
}