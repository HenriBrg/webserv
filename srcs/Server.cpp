#include "../inc/Webserv.hpp"

Server::Server(std::string name, int port): name(name), port(port) {
    
    port = -1;
    sockFd = -1;

    bzero(&addr, sizeof(addr));
    
    // Locations will be parsed later
    Location *newLoc1 = new Location("/", "./www", "index.html", "GET,TRACE,HEAD", "root:pass", "./www/cgi_tester");
    Location *newLoc2 = new Location("/tmp", "./www", "index.html", "GET,POST,HEAD", "root:pass", "./www/cgi_tester");
    locations.push_back(newLoc1);
    locations.push_back(newLoc2);

}

Server::~Server() {

    std::vector<Client*>::iterator itb;
    std::vector<Client*>::iterator ite = clients.end();

    for (itb = clients.begin(); itb < ite; itb++) {
        delete *itb;
    }
    clients.clear();

    LOGPRINT(INFO, this, "Server - Closed");
    gConfig.removeFd(sockFd);
    FD_CLR(sockFd, &gConfig.readSet);

}

int Server::start() {

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

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Adresse de l'hôte // 127.0.0.1:7777
    addr.sin_port = htons(port);
    if ((bind(sockFd, (struct sockaddr*)&addr, sizeof(addr))) == -1)
        throw ServerException("Server::start : bind()", std::string(strerror(errno)));

    //  printf("--> %" PRIu32 "\n", addr.sin_addr.s_addr);
    //  printf("--> %" PRIu16 "\n", addr.sin_port);

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

    LOGPRINT(INFO, this, "");
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
        LOGPRINT(LOGERROR, this, ("Server::acceptNewClient : accept()" + std::string(strerror(errno))));
        return ;
    }
    Client *newClient = new Client(this, acceptFd, clientAddr);
    clients.push_back(newClient);
    LOGPRINT(INFO, newClient, "Server::acceptNewClient() - New client !");
}

int Server::readClientRequest(Client *c) {

    int ret = -1;

    // En temps normal, le nombre d'octets reçus est retourné
    // Si aucun message n'est disponible sur la socket, la valeur -1 est renvoyée. En fait il faut juste réessayer + tard
    // La valeur de retour sera 0 si le pair a effectué un arrêt normal.  
   
    // OLD WAY
    // int x = strlen(c->buf);
    // ret = recv(c->acceptFd, c->buf + x, BUFMAX - x, 0);
    // x += ret;
    // c->buf[x] = '\0';
    // c->req.reqBuf = std::string(c->buf, x);


    // NEW WAY
    // int x = strlen(c->buf);
    ret = recv(c->acceptFd, c->buf, BUFMAX, 0);
    // x += ret;


    if (ret == -1 || ret == 0) {
        c->isConnected = false;
        LOGPRINT(LOGERROR, c, ("Server::readClientRequest : recv() returned " + std::to_string(ret) + " : Error : " + std::string(strerror(errno))));
        return (EXIT_FAILURE);
    } else {
        
        c->buf[ret] = '\0';
        LOGPRINT(INFO, c, ("Server::readClientRequest() : recv() has read " + std::to_string(ret) + " bytes"));
        if (c->recvStatus == Client::HEADER) {

            // If a payload / body is sent, we'll see it AFTER "\r\n\r\n" and Content-Length will be set, or encoding will be "chunked"
            if (strstr(c->buf, "\r\n\r\n") != NULL) {
                LOGPRINT(INFO, c, ("Server::readClientRequest() : Found closing pattern : \\r\\n\\r\\n"));
                // An HTTP request has to end with "\r\n\r\n"
                // If we pass here, it means that the request is fully received
                
                // TO DELETE/AVOID so that we don't duplicate too much buffers
                c->req.reqBuf = std::string(c->buf, ret);
                
                c->req.parse(locations);

            } else { 
                // If we pass here, it means that the request isn't fully received, so we'll have to recall recv
                LOGPRINT(INFO, c, ("Server::readClientRequest() : Incomplete Request (pattern \\r\\n\\r\\r not found yet) - We wait until its completion"));
                return (EXIT_FAILURE);
            }
        }
        if (c->recvStatus == Client::BODY) {
            // Body start after the \r\n\r\n of headers
            // https://en.wikipedia.org/wiki/Chunked_transfer_encoding
            if ((c->req.transferEncoding[0] == "chunked"))
                c->req.parseChunkedBody();
            else if (c->req.contentLength > 0)
                c->req.parseSingleBody();
            // else -> error 4XX ?
        }
        if (c->recvStatus == Client::COMPLETE) {
            LOGPRINT(INFO, c, ("Server::readClientRequest() : Request is completely received, we know handle response"));
            FD_SET(c->acceptFd, &gConfig.writeSetBackup); 
            c->req.showReq();
        }
    }

    return (EXIT_SUCCESS);
}

// OLD WAY
// Pour l'instant on va au plus simple
// char res[36] = "Hello Client ! Welcome to WEBSERV \n";
// write(c->acceptFd, res, sizeof(res));

// c->res.handleResponse(&c->req);
// // if ((ret = send(c->acceptFd, c->res.finalResponse.c_str(), c->res.finalResponse.length(), 0)) == -1) {
// //     c->isConnected = 0; 
// //     LOGPRINT(ERROR, c, ("Server::writeClientResponse : send() returned -1 : Error : " + std::string(strerror(errno))));
// //     return (ret);
// // }
// FD_CLR(c->acceptFd, &gConfig.readSetBackup);
// FD_CLR(c->acceptFd, &gConfig.writeSetBackup); 

// exit(0);


int Server::writeClientResponse(Client *c) {

    int ret = 0;

    if (c->res._sendStatus == Response::PREPARE) {

        c->res.resDispatch(&c->req);
        c->res.resBuild(&c->req);
        c->res._sendStatus = Response::SENDING;

        // if (!c->res._resBody.empty())
        //     addBody(c->res);


    }



    exit(0); // Tempo
    return (EXIT_SUCCESS);
}

void Server::handleClientRequest(Client *c) {

    if (c->acceptFd == -1)
        return ;
    if (FD_ISSET(c->acceptFd, &gConfig.readSet)) {
        if (readClientRequest(c) != 0)
            return ;
    } else
        LOGPRINT(INFO, c, ("Server::handleClientRequest() : Client socket isn't yet readable"));
    if (FD_ISSET(c->acceptFd, &gConfig.writeSet)) {
        if (c->recvStatus != Client::COMPLETE) {
            LOGPRINT(LOGERROR, c, ("Server::handleClientRequest() : Client Request isn't fully received yet"));
            return ;
        }
        if (writeClientResponse(c) != 0)
            return ;
    } else
        LOGPRINT(INFO, c, ("Server::handleClientRequest() : Client socket isn't yet writable"));
}

// LOGGER

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