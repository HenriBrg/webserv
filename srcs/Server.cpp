#include "../inc/Webserv.hpp"

Server::Server(std::string name, int port, std::string error): name(name), port(port), error(error) {
    port = -1;
    sockFd = -1;
    bzero(&addr, sizeof(addr));
}

Server::~Server() {}

int Server::start() {

    // for (int j= 0; j < locations.size(); j++)
    // {
    //     std::cout << "\nlocation" << j << std::endl;
	// 	std::cout << locations[j]->uri << std::endl;
	// 	std::cout << locations[j]->root << std::endl;
	// 	std::cout << locations[j]->index << std::endl;
	// 	std::cout << locations[j]->methods << std::endl;
	// 	std::cout << locations[j]->max_body << std::endl;
	// 	std::cout << locations[j]->auth << std::endl;
	// 	std::cout << locations[j]->cgi << std::endl;
	// 	std::cout << locations[j]->phpcgi << std::endl;
	// 	std::cout << locations[j]->ext << std::endl;
    // }
    
    // ---------- 1) SOCKET ----------

    // AF_NET : Protocole TCP/IP with IPV4
    // PF_INET wearable accross OS when using socket()
    // SOCK_STREAM : Connexion oriented (TCP)
    if ((sockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        throw ServerException("Server::start : socket()", std::string(strerror(errno)));

    // ---------- 2) SETSOCKOPT ----------

    // https://stackoverflow.com/a/3233022
    // SO_REUSEADDR is a boolean which allow the re-use of the current socket to avoid the error (when binding) : "Address already in use"

    // https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for
    // The second parameter indicates the "level" of the option that we want to set.
    // In this case, SOL_SOCKET indicates that the item we want to set refers to the socket itself
    // A TCP local socket address that has been bound is unavailable for
    // some time after closing, unless the SO_REUSEADDR flag has been set.

    // So this allow multiple connections, is it just a good habit, it will work without this
    // x value is the wanted value for the updated parameter

    int x = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x)) == -1)
        throw ServerException("Server::start : setsockopt()", std::string(strerror(errno)));

    // ---------- 3) BIND ----------

    // https://www.it-swarm.dev/fr/c/comprendre-inaddr-any-pour-la-programmation-de-sockets/1073716001/
    // https://www.commentcamarche.net/contents/1053-les-fonctions-de-l-api-socket

    // htonl : convertit un entier non signé hostlong depuis l'ordre des octets de l'hôte vers celui du réseau.
    // htons : convertit un entier court non signé hostshort depuis l'ordre des octets de l'hôte vers celui du réseau.
   
    // When INADDR_ANY is specified in the bind call the socket will be bound to all local interfaces
    // Default behavior with INADDR_ANY : automatically be filled with current host's IP address

    // Le numéro fictif INADDR_ANY signifie que le socket peut-être 
    // associé à n'importe quelle adresse IP de la machine locale (s'il en existe plusieurs). 
    // Dans les exemples on voit souvent inet_addr(127.0.0.1), c'est pour spécifier une adresse IP donnée à utiliser
    // Le socket peut être relié à un port libre quelconque en utilisant le numéro 0. 

    // TODO : forbidden functions

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);              // --------------> Voir post Slack sel-melc 21/10/2020
    if ((bind(sockFd, (struct sockaddr*)&addr, sizeof(addr))) == -1)
        throw ServerException("Server::start : bind()", std::string(strerror(errno)));

    // ---------- 4) LISTEN ----------

    // This listen() call tells the socket to listen to the incoming connections.
    // The listen() function places all incoming connection into a backlog queue
    // until accept() call accepts the connection.

    if (listen(sockFd, 128) == -1)
        throw ServerException("Server::start : listen()", std::string(strerror(errno)));

    // ---------- 5) FCNTL ----------

    // Full Answer about Blocking and non-blocking sockets : https://www.geeksforgeeks.org/non-blocking-io-with-pipes-in-c/
    // And here too : https://www.scottklement.com/rpg/socktut/nonblocking.html

    // Linux’s select() can return “ready-to-read” and then not actually be ready to read, thus causing the subsequent read() call to block.
	// You can work around this bug by setting O_NONBLOCK flag on the receiving socket so it errors with EWOULDBLOCK, then ignoring this error if it occurs.
    // O_NONBLOCK flag is sometimes used in cases where one wants to open but does not necessarily have the intention to read or write

    //  Calling this on a socket causes all future read() and write() calls on
    //  that socket to do only as much as they can immediately, and return 
    //  without waiting.
    //  If no data can be read or written, they return -1 and set errno
    //  to EAGAIN (or EWOULDBLOCK).

    if (fcntl(sockFd, F_SETFL, O_NONBLOCK) == -1)
        throw ServerException("Server::start : fcntl()", std::string(strerror(errno)));

    // ---------- 6) FD_SET ----------
    // On ajoute à la liste des FD le socket du serveur
    // Un descripteur de fichier est considéré comme prêt s'il est possible d'effectuer l'opération d'entrées-sorties correspondante (par exemple, un read(2)) sans bloquer.
    // On va lire la requête du client, qui aura été écrite dans la socket du serveur, donc on veut être alerté du caractère lisible du fd

    FD_SET(sockFd, &gConfig.readSetBackup);
    gConfig.addFd(sockFd);

    // ---------- 7) SET_METHOD_TAB ----------
	methodsTab["GET"]     = &Response::getReq;
	methodsTab["PUT"]     = &Response::putReq;
	methodsTab["POST"]    = &Response::postReq;
	methodsTab["HEAD"]    = &Response::headReq;
	methodsTab["DELETE"]  = &Response::deleteReq;
	methodsTab["PATCH"]   = &Response::patchReq;


    // ---------- 8) SET_ERROR_STATUS MAP ----------
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

    // S'il n'y a pas de connexion en attente dans la file, et si la socket n'est pas marquée comme non bloquante, accept() se met en attente d'une connexion.
    // Si la socket est non bloquante, et qu'aucune connexion n'est présente dans la file, accept() échoue avec l'erreur EAGAIN.
    bzero(&clientAddr, addrSize);
    // Ici, accept() remplit clientAddr des infos du client qu'il aura passé à connect()
    // La requête en elle même est à lire chez le socket du client et non le socket du serveur
    if ((acceptFd = accept(sockFd, (struct sockaddr *)&clientAddr, (socklen_t*)&addrSize)) == -1) {
        LOGPRINT(LOGERROR, this, ("Server::acceptNewClient : accept() failed : " + std::string(strerror(errno))));
        return ;
    }
    // TODO : if too many client, reject new ones
    Client *newClient = new Client(this, acceptFd, clientAddr);
    newClient->req.client = newClient;
    clients.push_back(newClient);
    LOGPRINT(INFO, newClient, "Server::acceptNewClient() - New client !");
}

void Server::readClientRequest(Client *c) {

    int ret = -1;
    int error;

    c->req.client = c;
    ret = recv(c->acceptFd, c->buf, BUFMAX, 0);
    if (ret == -1 || ret == 0) {
        c->isConnected = false;
        if (ret == 0)
            LOGPRINT(DISCONNECT, c, ("Server::readClientRequest : recv() returned 0 : The client (port " + std::to_string(c->port) + ") has closed its connection. Its initial request was : " + c->req.uri));
        if (ret == -1)
            LOGPRINT(LOGERROR, c, ("Server::readClientRequest : recv() returned -1 : Error : " + std::string(strerror(errno))));
        return ;
    } else {
        c->buf[ret] = '\0';
        LOGPRINT(INFO, c, ("Server::readClientRequest() : recv() has read " + std::to_string(ret) + " bytes"));
        if (c->recvStatus == Client::HEADER) {
            if (strstr(c->buf, "\r\n\r\n") != NULL) {
                LOGPRINT(INFO, c, ("Server::readClientRequest() : Found closing pattern <CR><LF><CR><LF>"));
                // TODO : We should find a way to avoid buffer dupllication for optimization
                c->req.reqBuf = std::string(c->buf); 
                c->req.parse(locations); // TODO : set errors if invalid request format
            } else { 
                LOGPRINT(INFO, c, ("Server::readClientRequest() : Invalid request format, pattern <CR><LF><CR><LF> not found in headers - End of connection"));
                return ;
            }
        }
        if (c->recvStatus == Client::BODY) {
            if ((c->req.transferEncoding[0] == "chunked"))
                c->req.parseChunkedBody(); // TODO : set errors if invalid request format
            else if (c->req.contentLength > 0)
                c->req.parseSingleBody(); // TODO : set errors if invalid request format
            else 
                LOGPRINT(LOGERROR, c, ("Server::readClientRequest() : Anormal body"));
            if (c->req.reqLocation->max_body != -1 && c->req._reqBody.size() > c->req.reqLocation->max_body) {
                LOGPRINT(REQERROR, c, ("Server::readClientRequest() : Error : REQUEST_ENTITY_TOO_LARGE_413 - Max = " + std::to_string(c->req.reqLocation->max_body)));
                c->recvStatus = Client::ERROR;
                c->res.setErrorParameters(Response::ERROR, REQUEST_ENTITY_TOO_LARGE_413);
            } 
        }
        if (c->recvStatus == Client::COMPLETE) {
            LOGPRINT(INFO, c, ("Server::readClientRequest() : Request is completely received, we now handle response"));
            FD_SET(c->acceptFd, &gConfig.writeSetBackup); 
            c->req.showReq();
        }
        if (c->recvStatus == Client::ERROR) {
            // TODO : Passage à tester
            c->recvStatus = Client::COMPLETE; // Because We will respond even if we get error
            c->res.setErrorParameters(Response::ERROR, (c->res._statusCode == -1 ? BAD_REQUEST_400 : c->res._statusCode));
            LOGPRINT(LOGERROR, c, ("Server::readClientRequest() : Client Request Error. We will directly respond to him with 400 BAD REQUEST"));
            FD_SET(c->acceptFd, &gConfig.writeSetBackup);
        }
    }
}

void Server::writeClientResponse(Client *c) {

    if (c->res._sendStatus == Response::PREPARE)
        setClientResponse(c);

    if (c->res._sendStatus == Response::ERROR) {
        /* We might pass directly here (without passing through resDispatch() if parsing client request raised an error */
        LOGPRINT(LOGERROR, c, ("Server::writeClientResponse() : sendStatus = ERROR - Code = " + std::to_string(c->res._statusCode)));
        setClientResponse(c);
    }

    if (sendClientResponse(c) == EXIT_FAILURE)
        c->res._sendStatus = Response::DONE;

    if (c->res._bytesSent == c->res.formatedResponse.size())
    {
        LOGPRINT(INFO, c, ("Server::writeClientResponse() : send() complete ! --> Bytes to send : " + std::to_string(c->res.formatedResponse.size()) + ", bytes effectively sent : " + std::to_string(c->res._bytesSent)));
        c->res._sendStatus = Response::DONE;
    } 
    else
        LOGPRINT(INFO, c, ("Server::writeClientResponse() : send() not complete --> Bytes to send : " + std::to_string(c->res.formatedResponse.size()) + ", bytes effectively sent : " + std::to_string(c->res._bytesSent)));

    if (c->res._sendStatus == Response::DONE)
    {
        FD_CLR(c->acceptFd, &gConfig.writeSetBackup);
        c->reset();
    }
}

void Server::setClientResponse(Client *c)
{
    c->res.control(&c->req, this); // Control (+set) method & authorization
    c->res.callMethod(&c->req); // Use requested method

    c->res.setHeaders(&c->req); // Set headers
    c->res.setBody(&c->req, this); // Set body
    c->res.setBodyHeaders(); // Set body headers to actual value (cleared in setHeaders())
    c->res.format(); // Format response
    c->res._sendStatus = Response::SENDING;
    c->res.showRes();
}

int Server::sendClientResponse(Client *c)
{
    int retSend(0);
    int bytesSent(0);
    int bytesToSend(0);

    if (c->res._sendStatus == Response::SENDING)
    {
        bytesToSend = c->res.formatedResponse.size();
        while (bytesSent < c->res.formatedResponse.size())
        {
            retSend = send(c->acceptFd, c->res.formatedResponse.c_str(), bytesToSend, 0);
            if (retSend == -1)
            {
                LOGPRINT(LOGERROR, c, ("Server::writeClientResponse() : send() failed"));
                return (EXIT_FAILURE);
            }
            bytesSent += retSend;
            bytesToSend -= retSend;
        }
        c->res._bytesSent += bytesSent;
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

}


/*  -----------------------------------  LOGGER & EXCEPTIONS ---------------------------------------------------- */

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