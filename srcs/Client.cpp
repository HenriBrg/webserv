# include "../inc/Webserv.hpp"

Client::Client(Server *server, int acceptFd, fd_set *readSet, fd_set *writeSet, struct sockaddr_in clientAddr):
    server(server), 
    acceptFd(acceptFd) {

    readFd = -1;
    writeFd = -1;

    recvStatus = Client::WAIT;

    // Temporary Forbidden Functions
    ip = inet_ntoa(clientAddr.sin_addr);
    port = htons(clientAddr.sin_port);

    // fcntl(acceptFd, F_SETFL, O_NONBLOCK);  // Nécessaire ou non ?
	// FD_SET(acceptFd, &gConfig.wFds);       // Nécessaire ou non ?
	// FD_SET(acceptFd, &gConfig.wFds);       // Nécessaire ou non ?


    // ADD Tableau de FD dans la config !

    req.client = this;
    FD_SET(acceptFd, &gConfig.rFds);
    gConfig.setNfds(acceptFd);

    buf = (char*)malloc(sizeof(char) * (BUFMAX + 1));
    memset((void*)buf, 0, BUFMAX + 1);

    recvStatus = HEADER;
}

Client::~Client() {

}
