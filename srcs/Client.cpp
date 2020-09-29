# include "../inc/Webserv.hpp"

Client::Client(Server *server, int acceptFd, struct sockaddr_in clientAddr):
    server(server), 
    acceptFd(acceptFd) {

    isConnected = true;
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

}

Client::~Client() {
    FD_CLR(acceptFd, &gConfig.readSetBackup);
    FD_CLR(acceptFd, &gConfig.writeSetBackup);
    gConfig.removeFd(acceptFd);
    close(acceptFd);
}

void Client::reset() {
    // The client must remain connected to the server
    isConnected = true;
    // The server is now ready for a new request from this client
    // TODO : add a status telling the client got response and stay connected ?
	recvStatus = HEADER;
	memset((void *)buf, 0, BUFMAX + 1);
	FD_SET(acceptFd, &gConfig.readSetBackup);
	FD_CLR(acceptFd, &gConfig.writeSetBackup);
}

std::string const Client::logInfo(void) {
    std::string ret;
    ret = "Client | Connected to port " + std::to_string(this->server->port) + \
          " from port " + std::to_string(this->port) + " | recvStatus : " +  std::to_string(this->recvStatus) + " | isConnected : " +  std::to_string(this->isConnected) ;
    return (ret);
}
