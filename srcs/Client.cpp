# include "../inc/Webserv.hpp"

Client::Client(Server *server, int acceptFd, struct sockaddr_in clientAddr) {
  
    server = server;
    acceptFd = acceptFd;

    // Temporary Forbidden Functions
    ip = inet_ntoa(clientAddr.sin_addr);
    port = htons(clientAddr.sin_port);

    FD_SET(acceptFd, &gConfig.rFds);


}

Client::~Client() {}
