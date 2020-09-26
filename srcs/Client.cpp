# include "../inc/Webserv.hpp"

Client::Client(Server *server, int acceptFd, struct sockaddr_in clientAddr):
    server(server), 
    acceptFd(acceptFd) {

    ip = inet_ntoa(clientAddr.sin_addr);
    port = htons(clientAddr.sin_port);
    req.client = this;
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

	memset((void *)buf, 0, BUFMAX + 1);

	FD_SET(acceptFd, &gConfig.readSetBackup);
	FD_CLR(acceptFd, &gConfig.readSet);
	FD_CLR(acceptFd, &gConfig.writeSetBackup);
	FD_CLR(acceptFd, &gConfig.writeSet);

	recvStatus = HEADER;


}