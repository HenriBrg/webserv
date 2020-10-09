#include "../inc/Webserv.hpp"

void exitCTRLC(int signal) {
    (void)signal;
    gConfig.run = 0;
}

Config::Config() {
    run = 1;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&readSetBackup);
    FD_ZERO(&writeSetBackup);
    signal(SIGINT, exitCTRLC);
}

void Config::clearConfig() {
    // TODO
    return ;
}

Config::~Config() {
    clearConfig();
}

int Config::getMaxFds(void) {
    return (*std::max_element(activeFds.begin(), activeFds.end()) + 1);
}


void Config::addFd(int fd) {
    std::pair<std::set<int>::iterator, bool> ret = activeFds.insert(fd);
    if (ret.second == false)
        NOCLASSLOGPRINT(LOGERROR, ("Config::addFd() : FD " + std::to_string(fd) + " already exists in the activeFds"));
}

void Config::removeFd(int fd) {
    activeFds.erase(fd);
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
