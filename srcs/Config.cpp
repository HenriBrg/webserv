#include "../inc/Webserv.hpp"

Config::Config() {
    run = 1;
    nfds = 0;
    FD_ZERO(&rFds);
    FD_ZERO(&wFds);
    FD_ZERO(&eFds);
    FD_ZERO(&rFdsBackup);
    FD_ZERO(&wFdsBackup);
    FD_ZERO(&eFdsBackup);
    this->timeout.tv_sec = 5;
    this->timeout.tv_usec = 0;
}

Config::~Config() {}

int Config::getNfds(void) const {
    return (this->nfds + 1);
}

void Config::setNfds(int newFd) {
    this->nfds = newFd;
}

void Config::resetFds(void) {

    this->rFds = this->rFdsBackup;
    this->wFds = this->wFdsBackup;
    this->eFds = this->eFdsBackup;

}

void Config::parse(void) {

    std::string n1("Server One");
    std::string n2("Server Two");
    Server *s1 = new Server(n1, 3000);
    Server *s2 = new Server(n2, 5000);

    try {
        s1->start();
        s2->start();
    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }

    this->servers.push_back(s1);
    this->servers.push_back(s2);

}
