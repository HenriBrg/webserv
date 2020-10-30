# include "../inc/Webserv.hpp"

Client::Client(Server *server, int acceptFd, struct sockaddr_in clientAddr):
    server(server), 
    acceptFd(acceptFd),
    res(this)
    {

    isConnected = true; // TODO : handle client-side de-connection

    ip = inet_ntoa(clientAddr.sin_addr);
    port = htons(clientAddr.sin_port);
    req.client = this;
    // On "lira" le client au prochain passage de select() après le call resetFds() de la config
    // L'enjeu c'est qu'on ne peut pas modifier les FD_SET ayant déclencé select() pendant qu'on est dans la loop du server
    // D'ailleurs, c'est un point à approfondir + tard
 	FD_SET(acceptFd, &gConfig.readSetBackup); 
    gConfig.addFd(acceptFd);
    recvStatus = Client::HEADER;
    buf = (char*)malloc(sizeof(char) * (BUFMAX + 1));
    memset((void*)buf, 0, BUFMAX + 1);

    cgipid = -1; 
    cgiWriteFd = -1;
    cgiReadFd = -1;

}

Client::~Client() {

    LOGPRINT(DISCONNECT, this, ("Request::~Client() : Client destructor is called"));

    free(buf);
    FD_CLR(acceptFd, &gConfig.readSetBackup);
    FD_CLR(acceptFd, &gConfig.writeSetBackup);
    
    gConfig.removeFd(cgiWriteFd);
    gConfig.removeFd(cgiReadFd);
    gConfig.removeFd(acceptFd);

    close(cgiWriteFd);
    close(cgiReadFd);
    close(acceptFd);

    cgiWriteFd = -1;
    cgiReadFd = -1;
    acceptFd = -1;

}

void Client::reset() {

    // We reset everything, except port, acceptFd, server, ip

    req.client = this;
    memset((void*)buf, 0, BUFMAX + 1);
    strbuf.clear();

    isConnected = true;
	recvStatus = HEADER;

    req.reset();
    res.reset();

	// FD_SET(acceptFd, &gConfig.readSetBackup);
	FD_CLR(acceptFd, &gConfig.writeSetBackup);

}

std::string const Client::logInfo(void) {
    std::string ret;
    ret = "Client | Connected to port " + std::to_string(this->server->port) + \
          " from port " + std::to_string(this->port) + " | recvStatus : " +  std::to_string(this->recvStatus) + " | isConnected : " +  std::to_string(this->isConnected) ;
    return (ret);
}
