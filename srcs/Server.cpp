#include "../inc/Webserv.hpp"

Server::Server(std::string name, int port): name(name), port(port) {
    
    bzero(&addr, sizeof(addr));
    
    // Locations will be parsed later
    Location *newLoc1 = new Location("/", "./www", "index.html", "GET");
    Location *newLoc2 = new Location("/", "./www", "page.html", "GET");

    locations.push_back(newLoc1);
    locations.push_back(newLoc2);

}

Server::~Server() {}

int    Server::start() {

    // ---------- 1) SOCKET ----------

    // AF_NET : Protocole TCP/IP with IPV4
    // SOCK_STREAM : Connexion oriented (TCP)
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw ServerException("Server::start : socket()", std::string(strerror(errno)));

    // ---------- 2) SETSOCKOPT ----------

    // https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for
    // The second parameter indicates the "level" of the option that we want to set.
    // In this case, SOL_SOCKET indicates that the item we want to set refers to the socket itself
    // SO_REUSEADDR is a boolean which allow the re-use of the current socket to avoid that kind of error when binding : "Address already in use"

    // https://man7.org/linux/man-pages/man7/ip.7.html
    // A TCP local socket address that has been bound is unavailable for
    // some time after closing, unless the SO_REUSEADDR flag has been set.

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
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Adresse de l'hôte
    addr.sin_port = htons(port);
    if ((bind(sockFd, (struct sockaddr*)&addr, sizeof(addr))) == -1)
        throw ServerException("Server::start : bind()", std::string(strerror(errno)));

    //  printf("--> %" PRIu32 "\n", addr.sin_addr.s_addr);
    //  printf("--> %" PRIu16 "\n", addr.sin_port);

    // ---------- 4) LISTEN ----------

    // This listen() call tells the socket to listen to the incoming connections.
    // The listen() function places all incoming connection into a backlog queue
    // until accept() call accepts the connection.

    if (listen(sockFd, 42) == -1)
        throw ServerException("Server::start : listen()", std::string(strerror(errno)));

    // ---------- 5) FCNTL ----------

    // Fulll Answer about Blocking and non-blocking sockets : https://www.geeksforgeeks.org/non-blocking-io-with-pipes-in-c/
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
    
    FD_SET(sockFd, &gConfig.rFdsBackup);
    gConfig.setNfds(sockFd);

    return (EXIT_SUCCESS);
}

void Server::acceptNewClient(void) {

    int acceptFd;
    struct sockaddr_in clientAddr;
    int addrSize = sizeof(clientAddr);

    bzero(&clientAddr, addrSize);
    if ((acceptFd = accept(sockFd, (struct sockaddr *)&clientAddr, (socklen_t*)&addrSize)) == -1) {
        gConfig.run = 0;
        // LOGGER
        throw ServerException("Server::acceptNewClient : accept()", std::string(strerror(errno)));
    } else
    {
        // LOGGER
        Client *newClient = new Client(this, acceptFd, clientAddr);
        clients.push_back(newClient);

    }
}


int Server::readClientRequest(Client *c) {

    int ret;

    bzero(c->buf, sizeof(c->buf));
    if ((ret = recv(c->acceptFd, c->buf, sizeof(c->buf), 0)) == -1) {
        // LOGGER
        // throw ServerException("Server::readClientRequest : recv()", std::string(strerror(errno)));
        return (-1);
    }
    c->buf[ret] = 0;
    
    // LOGGER : We've read the following client request :

    c->req.buf = std::string(c->buf, sizeof(c->buf));
    c->req.parse(locations);
    FD_CLR(c->acceptFd, &gConfig.rFds);
    FD_SET(c->acceptFd, &gConfig.wFds);

    return (0);
}

int Server::writeClientResponse(Client *c) {

    return (0);
}

void Server::handleClientRequest(Client *c) {

    if (FD_ISSET(c->acceptFd, &gConfig.rFds)) {
        if (readClientRequest(c) != 0)
            return ;
    }

    if (FD_ISSET(c->acceptFd, &gConfig.wFds)) {
        if (writeClientResponse(c) != 0)
            return ;
    } else {
        // LOGGER
        std::cerr << "Server::handleClientRequest : write(): error" << std::endl;
    }

}

Server::ServerException::ServerException(std::string where, std::string error) {
    this->error = where + ": " + error;
}

Server::ServerException::~ServerException(void) throw() {
    
}

const char * Server::ServerException::what(void) const throw() {
    return this->error.c_str();
}