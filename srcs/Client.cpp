# include "../inc/Webserv.hpp"

Client::Client(Server *server, int acceptFd, struct sockaddr_in clientAddr):
    server(server), 
    acceptFd(acceptFd) {

    readFd = -1;
    writeFd = -1;

    recvStatus = Client::WAIT;

    ip = inet_ntoa(clientAddr.sin_addr);
    port = htons(clientAddr.sin_port);

    // 
    fcntl(acceptFd, F_SETFL, O_NONBLOCK);  
	FD_SET(acceptFd, &gConfig.readSetBackup);
	FD_SET(acceptFd, &gConfig.writeSetBackup);

    req.client = this;
    buf = (char*)malloc(sizeof(char) * (BUFMAX + 1));
    memset((void*)buf, 0, BUFMAX + 1);

}

Client::~Client() {

}
