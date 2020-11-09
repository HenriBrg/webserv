#include "../inc/Webserv.hpp"

Server::Server(int port, std::string name, std::string error): port(port), name(name), error(error) {
    sockFd = -1;
    bzero(&addr, sizeof(addr));
}

Server::~Server() {}

int Server::start() {

    /* ---------- 1) SOCKET ---------- */

    /* AF_NET : Protocole TCP/IP with IPV4 */
    /* PF_INET wearable accross OS when using socket() */
    /* SOCK_STREAM : Connexion oriented (TCP) */
    if ((sockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        throw ServerException("Server::start : socket()", std::string(strerror(errno)));

    /* ---------- 2) SETSOCKOPT ---------- */

    /* https://stackoverflow.com/a/3233022 */
    /* SO_REUSEADDR is a boolean which allow the re-use of the current socket to avoid the error (when binding) : "Address already in use" */

    /* https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for */
    /* The second parameter indicates the "level" of the option that we want to set. */
    /* In this case, SOL_SOCKET indicates that the item we want to set refers to the socket itself */
    /* A TCP local socket address that has been bound is unavailable for */
    /* some time after closing, unless the SO_REUSEADDR flag has been set. */

    /* So this allow multiple connections, is it just a good habit, it will work without this */
    /* x value is the wanted value for the updated parameter */
    /* https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ */

    int x = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &x, sizeof(x)) == -1)
        throw ServerException("Server::start : setsockopt()", std::string(strerror(errno)));

    /* ---------- 3) BIND ---------- */

    /* https://www.it-swarm.dev/fr/c/comprendre-inaddr-any-pour-la-programmation-de-sockets/1073716001/ */
    /* https://www.commentcamarche.net/contents/1053-les-fonctions-de-l-api-socket */

    // When INADDR_ANY is specified in the bind call the socket will be bound to all local interfaces
    // Default behavior with INADDR_ANY : automatically be filled with current host's IP address

    /* Le numéro fictif INADDR_ANY signifie que le socket peut-être  */
    /* associé à n'importe quelle adresse IP de la machine locale (s'il en existe plusieurs).  */
    /* Dans les exemples on voit souvent inet_addr(127.0.0.1), c'est pour spécifier une adresse IP donnée à utiliser */
    /* Le socket peut être relié à un port libre quelconque en utilisant le numéro 0.  */
    /* "Network byte order" always means big endian */
    /* "Host byte order" depends on architecture of host. Depending on CPU, host byte order may be little endian or big endian */

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(LOCAL_IP);

    /* https://code.woboq.org/userspace/glibc/bits/byteswap.h.html#30 */
    
    int i = 0x00000001;
    if (((char *)&i)[0] ) /* Little */ addr.sin_port = ((__uint16_t) ((((port) >> 8) & 0xff) | (((port) & 0xff) << 8)));
    else /* Big */ addr.sin_port = static_cast<__uint16_t>(port);

    if ((bind(sockFd, (struct sockaddr*)&addr, sizeof(addr))) == -1)
        throw ServerException("Server::start : bind()", std::string(strerror(errno)));

    /* ---------- 4) LISTEN ---------- */

    /* This listen() call tells the socket to listen to the incoming connections. */
    /* The listen() function places all incoming connection into a backlog queue */
    /* until accept() call accepts the connection. */

    if (listen(sockFd, 128) == -1)
        throw ServerException("Server::start : listen()", std::string(strerror(errno)));

    /* ---------- 5) FCNTL ---------- */

    /* Full Answer about Blocking and non-blocking sockets : https://www.geeksforgeeks.org/non-blocking-io-with-pipes-in-c/ */
    /* And here too : https://www.scottklement.com/rpg/socktut/nonblocking.html */

    /* Linux’s select() can return “ready-to-read” and then not actually be ready to read, thus causing the subsequent read() call to block. */
	/* You can work around this bug by setting O_NONBLOCK flag on the receiving socket so it errors with EWOULDBLOCK, then ignoring this error if it occurs. */
    /* O_NONBLOCK flag is sometimes used in cases where one wants to open but does not necessarily have the intention to read or write */

    /*  Calling this on a socket causes all future read() and write() calls on */
    /*  that socket to do only as much as they can immediately, and return  */
    /*  without waiting. */
    /*  If no data can be read or written, they return -1 and set errno */
    /*  to EAGAIN (or EWOULDBLOCK). */

    if (fcntl(sockFd, F_SETFL, O_NONBLOCK) == -1)
        throw ServerException("Server::start : fcntl()", std::string(strerror(errno)));

    /* ---------- 6) FD_SET ---------- */
    /* On ajoute à la liste des FD le socket du serveur */
    /* Un descripteur de fichier est considéré comme prêt s'il est possible d'effectuer l'opération d'entrées-sorties correspondante (par exemple, un read(2)) sans bloquer. */
    /* On va lire la requête du client, qui aura été écrite dans la socket du serveur, donc on veut être alerté du caractère lisible du fd */

    FD_SET(sockFd, &gConfig.readSetBackup);
    gConfig.addFd(sockFd);

    /* ---------- 7) SET_METHOD_TAB ---------- */
	methodsTab["GET"]     = &Response::getReq;
	methodsTab["PUT"]     = &Response::putReq;
	methodsTab["POST"]    = &Response::postReq;
	methodsTab["HEAD"]    = &Response::headReq;
	methodsTab["DELETE"]  = &Response::deleteReq;
	methodsTab["PATCH"]   = &Response::patchReq;

    /* ---------- 8) SET_ERROR_STATUS MAP ---------- */
    _errorStatus[BAD_REQUEST_400] = "400 BAD RESQUEST";
    _errorStatus[UNAUTHORIZED_401] = "401 UNAUTHORIZED";
    _errorStatus[NOT_FOUND_404] = "404 NOT FOUND";
    _errorStatus[METHOD_NOT_ALLOWED_405] = "405 METHOD NOT ALLOWED";
    _errorStatus[NOT_ACCEPTABLE_406] = "406 NOT ACCEPTABLE";
    _errorStatus[CONFLICT_409] = "409 CONFLICT";
    _errorStatus[REQUEST_ENTITY_TOO_LARGE_413] = "413 REQUEST ENTITY TOO LARGE";
    _errorStatus[REQUEST_URI_TOO_LONG_414] = "414 BAD RESQUEST";
    _errorStatus[SSL_CERTIFICATE_ERROR_495] = "495 SSL CERTIFICATE ERROR";    
    _errorStatus[INTERNAL_ERROR_500] = "500 INTERNAL ERROR";
    _errorStatus[NOT_IMPLEMENTED_501] = "501 NOT IMPLEMENTED";
    _errorStatus[SERVICE_UNAVAILABLE_503] = "503 SERVICE UNAVAILABLE";
    _errorStatus[HTTP_VERSION_NOT_SUPPORTED_505] = "505 HTTP VERSION NOT SUPPORTED";
    
    return (EXIT_SUCCESS);
}

void Server::acceptNewClient(void) {

    int acceptFd = -1;
    struct sockaddr_in clientAddr;
    int addrSize = sizeof(clientAddr);

    bzero(&clientAddr, addrSize);
    LOGPRINT(INFO, this, ("Server::acceptNewClient : accept() " + std::string(strerror(errno))));
    if ((acceptFd = accept(sockFd, (struct sockaddr *)&clientAddr, (socklen_t*)&addrSize)) == -1) {
        LOGPRINT(LOGERROR, this, ("Server::acceptNewClient : accept() failed : " + std::string(strerror(errno))));
        return ;
    }

    if (fcntl(acceptFd, F_SETFL, O_NONBLOCK) == -1)
        LOGPRINT(LOGERROR, this, ("Server::acceptNewClient : fcntl() failed : " + std::string(strerror(errno))));
    int x = 1;
    if (setsockopt(acceptFd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x)) == -1)
        LOGPRINT(LOGERROR, this, ("Server::acceptNewClient : setsockopt() failed : " + std::string(strerror(errno))));

    Client *newClient = new Client(acceptFd, this, clientAddr);
    newClient->req.client = newClient;
    clients.push_back(newClient);

    if (gConfig._availableConnections <= 0)
    {
        newClient->res.setRefusedClient(this);
        sendClientResponse(newClient);
        if (newClient->res._sendStatus == Response::DONE) {
            FD_CLR(newClient->acceptFd, &gConfig.writeSetBackup);
            newClient->reset();
        }
        newClient->isConnected = false;
        newClient->_isAccepted = false;
    }
    else
        gConfig._availableConnections--;
    LOGPRINT(INFO, newClient, "Server::acceptNewClient() - New client !");
}

void Server::readClientRequest(Client *c) {

    char recvBuffer[BUFMAX + 1];
    int recvRet(-1);
    bool recvCheck(false);

    memset((void*)recvBuffer, 0, BUFMAX);
    c->resetTimeOut();
    while ((recvRet = recv(c->acceptFd, recvBuffer, BUFMAX, 0)) > 0)
    {
        recvBuffer[recvRet] = '\0';
        c->req._reqBody.append(recvBuffer);
        recvCheck = true;
    }

    /* À conserver */

    /* int i = 0; */
    // std::cout << RED << " ==================== REQBODY HEXA :" << END << std::endl;
    /* std::string::iterator it = c->req._reqBody.begin(); */
    /* for (; it < c->req._reqBody.end(); it++) { */
    /*     i++; */
    /*     std::cout << std::hex << (int) *it; */
    /*     if (i > 500) */
    /*         break ; */
    /* } */
    /* std::cout <<  std::endl << RED << " ====================" << END << std::endl; */

    if (!(recvCheck) || recvRet == 0)
    {
        c->isConnected = false;
        if (recvRet == 0)
        {
            LOGPRINT(DISCONNECT, c, ("Server::readClientRequest : recv() returned 0 : The client (port " + std::to_string(c->port) + ") has closed its connection"));
        }
        else if (recvCheck == false)
            LOGPRINT(LOGERROR, c, ("Server::readClientRequest : recv() returned -1 : Error : " + std::string(strerror(errno))));
        return ;
    }
    else {

        LOGPRINT(INFO, c, ("Server::readClientRequest() : recv() has read " + std::to_string(c->req._reqBody.size()) + " bytes"));
        if (c->recvStatus == Client::HEADER) {
            if (strstr(c->req._reqBody.c_str(), "\r\n\r\n") != NULL) {
                LOGPRINT(INFO, c, ("Server::readClientRequest() : Found closing pattern <CR><LF><CR><LF>"));
                c->req.parse(locations);
            }
            else {
                LOGPRINT(INFO, c, ("Server::readClientRequest() : Invalid request format, pattern <CR><LF><CR><LF> not found in headers"));
                return ;
            }
        }
        if (c->recvStatus == Client::BODY)
            c->req.parseBody();

        if (c->recvStatus == Client::COMPLETE) {
            LOGPRINT(INFO, c, ("Server::readClientRequest() : Request is completely received, we now handle response"));
            FD_SET(c->acceptFd, &gConfig.writeSetBackup);
            if (SILENTLOGS == 0)
                c->req.showReq();
        }
        if (c->recvStatus == Client::ERROR) {
            c->recvStatus = Client::COMPLETE;
            c->res.setErrorParameters(Response::ERROR, (c->res._statusCode == -1 ? BAD_REQUEST_400 : c->res._statusCode));
            LOGPRINT(INFO, c, ("Server::readClientRequest() : Client Request Error. We will directly respond to him with 400 BAD REQUEST"));
            FD_SET(c->acceptFd, &gConfig.writeSetBackup);
        }
    }
}

void Server::writeClientResponse(Client *c) {

    if (c->res._sendStatus == Response::PREPARE || c->res._sendStatus == Response::ERROR) {
        if (c->res._sendStatus == Response::ERROR)
            LOGPRINT(LOGERROR, c, ("Server::writeClientResponse() : sendStatus = ERROR - Code = " + std::to_string(c->res._statusCode)));
        setClientResponse(c);
    }
    sendClientResponse(c);
    if (c->res._sendStatus == Response::DONE) {
        FD_CLR(c->acceptFd, &gConfig.writeSetBackup);
        c->reset();
    }

}


void Server::setClientResponse(Client *c)
{
    LOGPRINT(INFO, &c->res, ("Server::setClientResponse() : L238 - Control"));
    c->res.control(&c->req, this); /* Control (+set) method & authorization */
    LOGPRINT(INFO, &c->res, ("Server::setClientResponse() : Calling " + c->req.method + " Handler"));
    c->res.callMethod(&c->req); /* Use requested method */
    LOGPRINT(INFO, &c->res, ("Server::setClientResponse() : Set and Format Header"));
    c->res.setHeaders(&c->req); /* Set headers */
    LOGPRINT(INFO, &c->res, ("Server::setClientResponse() : Set Body and Body Headers"));
    c->res.setBody(c->server); /* Set body */
    c->res.setBodyHeaders(); /* Set body headers to actual value (cleared in setHeaders()) */
    c->res.format(); /* Format response */
    c->res._sendStatus = Response::SENDING;
    if (SILENTLOGS == 0)
        c->res.showRes();
}


int Server::sendClientResponse(Client *c) {

    int retSendingBytes;

    if (c->res._sendStatus == Response::SENDING) {
        if (sendBytes(c, &(c->res.formatedResponse[0]), c->res.formatedResponse.size()) == EXIT_FAILURE)
            LOGPRINT(LOGERROR, c, ("Server::sendClientResponse() : send() headers failed"));
        if (c->res._resBody) {
            retSendingBytes = sendBytes(c, c->res._resBody, c->res.contentLength);
            if (retSendingBytes == EXIT_FAILURE) {
                LOGPRINT(LOGERROR, c, ("Server::sendClientResponse() : send() _resBody has failed - Error : " + std::string(strerror(errno))));
            }
        }
    }
    c->res._sendStatus = Response::DONE;
    return (EXIT_SUCCESS);
}

int Server::sendBytes(Client *c, char *toSend, long bytesToSend)
{
    long retSend(0);
    long bytesSent(0);
    long totalBytes = bytesToSend;

    while (bytesSent < totalBytes) {
        retSend = send(c->acceptFd, toSend, bytesToSend, 0);
        if (retSend == -1)
        {
            if (bytesToSend != 0)
                continue ;
            return (EXIT_FAILURE);
        } else if (retSend == 0)
            NOCLASSLOGPRINT(INFO, ("retSend = 0"));
        bytesSent += retSend;
        bytesToSend -= retSend;
        NOCLASSLOGPRINT(INFO, ("Cumulated bytes sent : " + std::to_string(bytesSent)));
    }
    if (bytesToSend != 0) {
        LOGPRINT(LOGERROR, c, ("Server::writeClientResponse() : send() not complete --> Bytes sent : "
        + std::to_string(bytesSent) + " (total = " + std::to_string(totalBytes) + ")"));
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}


void Server::handleClientRequest(Client *c) {

    if (FD_ISSET(c->acceptFd, &gConfig.readSet))
        readClientRequest(c);
    else LOGPRINT(INFO, c, ("Server::handleClientRequest() : Client socket isn't readable"));
    if (FD_ISSET(c->acceptFd, &gConfig.writeSet) && c->recvStatus == Client::COMPLETE)
        writeClientResponse(c);
    else LOGPRINT(INFO, c, ("Server::handleClientRequest() : Client socket isn't writable"));
    checkTimeOutClient(c);

}

void Server::checkTimeOutClient(Client *c) {
    if (c->_lastRequest && (ft::getTime() - c->_lastRequest) > 1000)
        c->isConnected = false;
}

/* **************************************************** */
/*                  LOGGER & EXCEPTIONS                 */
/* **************************************************** */

std::string const Server::logInfo(void) {
    std::string ret;
    ret = name + " | " + "Listening Port " + std::to_string(port) + " | Clients connected : " + std::to_string(clients.size());
    return (ret);
}

Server::ServerException::ServerException(std::string where, std::string error) {
    this->error = where + ": " + error;
}

Server::ServerException::~ServerException(void) throw() {

}

const char * Server::ServerException::what(void) const throw() {
    return this->error.c_str();
}