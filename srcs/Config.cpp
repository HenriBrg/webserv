#include "../inc/Webserv.hpp"

int Config::_availableConnections = 220;

Config::Config() {
    run = 1;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&readSetBackup);
    FD_ZERO(&writeSetBackup);
}

void handleCTRLC(int s) {
    (void)s;
    write(1, "\b\b", 2);
    exit(EXIT_FAILURE);
}

void Config::webservShutdown() {

    Server                                *srv;
    std::vector<Client*>::iterator        itCli;
    std::vector<Location*>::iterator      itLoc;
    std::vector<Server*>::iterator        itSrv;

    for (itSrv = gConfig.servers.begin(); itSrv != gConfig.servers.end(); itSrv++) {
        srv = *itSrv;
        for (itCli = srv->clients.begin(); itCli != srv->clients.end(); itCli++)
            delete *itCli;
        for (itLoc = srv->locations.begin(); itLoc != srv->locations.end(); itLoc++)
            delete *itLoc;
        srv->clients.clear();
        srv->locations.clear();
    }

    for (itSrv = gConfig.servers.begin(); itSrv != gConfig.servers.end(); itSrv++) {
        gConfig.removeFd(((*itSrv)->sockFd));
        delete *itSrv;
    }

    gConfig.servers.clear();
    gConfig.activeFds.clear();
    
    FD_ZERO(&gConfig.readSet);
    FD_ZERO(&gConfig.writeSet);
    FD_ZERO(&gConfig.readSetBackup);
    FD_ZERO(&gConfig.writeSetBackup);

    NOCLASSLOGPRINT(INFO, ("Shutting down Webserv, good bye !"));
}

Config::~Config() {
    webservShutdown();
}

int Config::getMaxFds(void) {
    return (*std::max_element(activeFds.begin(), activeFds.end()) + 1);
}


void Config::addFd(int fd) {
    std::pair<std::set<int>::iterator, bool> ret = activeFds.insert(fd);
    if (ret.second == false)
        NOCLASSLOGPRINT(LOGERROR, ("Config::addFd() : FD " + std::to_string(fd) + " already exists in the activeFds. Bad things will happen !"));
}

void Config::removeFd(int fd) {
    activeFds.erase(fd);
}

void Config::resetFds() {
    readSet = readSetBackup;
    writeSet = writeSetBackup;
}

void Config::showFDSETS() {

    int x = 0;
    std::string tmp;
    std::set<int>::iterator it = activeFds.begin();

    for (; it != activeFds.end(); it++) {
        if (FD_ISSET(*it, &gConfig.readSet)) {
            tmp += std::to_string(*it) + " - ";
            x++;
        }
    }
    if (x)
        tmp = tmp.substr(0, tmp.size() - 3);

    NOCLASSLOGPRINT(INFO, ("Config::showFDSETS() : Inside gConfig.readSet, " + std::to_string(x) + " sockets [" + tmp + "] are watched on their read status"));
   
    x = 0;
    tmp.clear();
    it = activeFds.begin();
    for (; it != activeFds.end(); it++) {
        if (FD_ISSET(*it, &gConfig.writeSet)) {
            tmp += (std::to_string(*it) + " - ");
            x++;
        }
    }
    if (x)
        tmp = tmp.substr(0, tmp.size() - 3);

    NOCLASSLOGPRINT(INFO, ("Config::showFDSETS() : Inside gConfig.writeSet, " + std::to_string(x) + " sockets [" + tmp + "] are watched on their write status"));

}