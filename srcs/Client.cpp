# include "../inc/Webserv.hpp"

Client::Client(Server *server, int acceptFd, struct sockaddr_in clientAddr): server(server), acceptFd(acceptFd), res(this) {
    
    isConnected = true;
    ip = inet_ntoa(clientAddr.sin_addr);
    port = htons(clientAddr.sin_port);
    req.client = this;
 	FD_SET(acceptFd, &gConfig.readSetBackup); 
    gConfig.addFd(acceptFd);
    recvStatus = Client::HEADER;
    buf = (char*)malloc(sizeof(char) * (BUFMAX + 1));
    memset((void*)buf, 0, BUFMAX + 1);
    cgipid = -1; 

}

Client::~Client() {

    free(buf);
    FD_CLR(acceptFd, &gConfig.readSetBackup);
    FD_CLR(acceptFd, &gConfig.writeSetBackup);
    gConfig.removeFd(acceptFd);
    close(acceptFd);
    acceptFd = -1;

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
	recvStatus = HEADER;
    req.reset();
    res.reset();
	FD_CLR(acceptFd, &gConfig.writeSetBackup);

}

std::string const Client::logInfo(void) {
    std::string ret;
    ret = "Client | Connected to port " + std::to_string(this->server->port) + " from port " + std::to_string(this->port) + " | recvStatus : " +  std::to_string(this->recvStatus) + " | isConnected : " +  std::to_string(this->isConnected) ;
    return (ret);
}
