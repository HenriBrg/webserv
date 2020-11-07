# include "../inc/Webserv.hpp"

Client::Client(int acceptFd, Server *server, struct sockaddr_in clientAddr): acceptFd(acceptFd), server(server), req(this), res(this) {
    
    isConnected = true;
    /* By default we consider we are able to accept an user */
    _isAccepted = true; 
    ip = inet_ntoa(clientAddr.sin_addr);
    port = htons(clientAddr.sin_port);
    req.client = this;
 	FD_SET(acceptFd, &gConfig.readSetBackup); 
    gConfig.addFd(acceptFd);
    recvStatus = Client::HEADER;
    buf = (char*)malloc(sizeof(char) * (BUFMAX + 1));
    memset((void*)buf, 0, BUFMAX + 1);
    _lastRequest = 0;
    cgipid = -1;
}

Client::~Client() {

    free(buf);
    FD_CLR(acceptFd, &gConfig.readSetBackup);
    FD_CLR(acceptFd, &gConfig.writeSetBackup);
    gConfig.removeFd(acceptFd);
    close(acceptFd);
    acceptFd = -1;
    if (_isAccepted)
        gConfig._availableConnections++;
}

/*
**  Reset client after response
**  We reset everything except variables which persist between requests
*/

void Client::reset() {

    req.client = this;
    memset((void*)buf, 0, BUFMAX + 1);
    strbuf.clear();
    isConnected = true;
	recvStatus = Client::HEADER;
    req.reset();
    res.reset();
	FD_CLR(acceptFd, &gConfig.writeSetBackup);

}

void Client::resetTimeOut(void) {
    _lastRequest = ft::getTime();
}

std::string const Client::logInfo(void) {
    std::string ret;
    ret = "Client | Connected to port " + std::to_string(this->server->port) + " from port " + std::to_string(this->port) + " | recvStatus : " +  std::to_string(this->recvStatus) + " | isConnected : " +  std::to_string(this->isConnected) ;
    return (ret);
}
