#include "../inc/Webserv.hpp"

void exitCTRLC(int signal) {
    (void)signal;
    gConfig.run = 0;
}

Config::Config() {
    run = 1;
    nfds = 0;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&readSetBackup);
    FD_ZERO(&writeSetBackup);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    signal(SIGINT, exitCTRLC);
}

Config::~Config() {}

int Config::getMaxFds(void) {
    
    int tmp;
    int retMaxFd = 0;
    
    std::vector<Server*>::iterator it;
    for (it = servers.begin(); it != servers.end(); it++) {
        tmp = (*it)->getMaxFdServer();
        if (tmp > retMaxFd)
            retMaxFd = tmp;
    }

    return (retMaxFd + 1);
}

void Config::resetFds() {

    readSet = readSetBackup;
    writeSet = writeSetBackup;

}

void Config::init() {

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

    servers.push_back(s1);
    servers.push_back(s2);

}
