#include "../inc/Webserv.hpp"

Config::Config() {
    run = 1;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&readSetBackup);
    FD_ZERO(&writeSetBackup);
}

void Config::webservShutdown(int signal) {

    (void)signal;

    Server      *srv;
    std::vector<Client*>::iterator        itCli;
    std::vector<Location*>::iterator      itLoc;
    std::vector<Server*>::iterator        itSrv;

    for (itSrv = gConfig.servers.begin(); itSrv != gConfig.servers.end(); ++itSrv) {
        srv = *itSrv;
        for (itCli = srv->clients.begin(); itCli != srv->clients.end(); ++itCli)
            delete *itCli;
        for (itLoc = srv->locations.begin(); itLoc != srv->locations.end(); ++itLoc)
            delete *itLoc;
    }
    for (itSrv = gConfig.servers.begin(); itSrv != gConfig.servers.end(); ++itSrv)
        delete *itSrv;
    gConfig.servers.clear();
    activeFds.clear();
    
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&readSetBackup);
    FD_ZERO(&writeSetBackup);

    NOCLASSLOGPRINT(LOGERROR, ("Shutting down Webserv ... Good bye !"));
    exit(EXIT_SUCCESS);
}

Config::~Config() {
    webservShutdown(0);
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

void Config::init() {
    std::cout << std::endl;
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
