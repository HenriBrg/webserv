#include "../inc/Webserv.hpp"

void exitCTRLC(int signal) {
    (void)signal;
    gConfig.run = 0;
}

Config::Config() {
    run = 1;
    nfds = 0;
    FD_ZERO(&rFds);
    FD_ZERO(&wFds);
    FD_ZERO(&rFdsBackup);
    FD_ZERO(&wFdsBackup);
    // this->timeout.tv_sec = 5;
    // this->timeout.tv_usec = 0;
    signal(SIGINT, exitCTRLC);
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
}

void Config::parse(void) {

    std::string n1("Server One");
    std::string n2("Server Two");

    Server *s1 = new Server(n1, 7777);
    Server *s2 = new Server(n2, 8888);

    try {
        s1->start();
        s2->start();
    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }

    this->servers.push_back(s1);
    this->servers.push_back(s2);

}
