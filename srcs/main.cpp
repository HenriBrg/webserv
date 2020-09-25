# include "../inc/Webserv.hpp"

Config gConfig;

int main(int ac, char **av) {

    Server *s;
	Client *c;

    fd_set			readSet;
	fd_set			writeSet;
    fd_set          readSetBackup;
    fd_set          writeSetBackup;
    
    gConfig.init(&readSet, &writeSet, &readSetBackup, &writeSetBackup);
    while (gConfig.run) {
        gConfig.resetFds(&readSet, &writeSet, &readSetBackup, &writeSetBackup);
        select(gConfig.getMaxFds(), &readSet, &writeSet, NULL, &gConfig.timeout);
        std::vector<Server*>::iterator its;
        for (its = gConfig.servers.begin(); its != gConfig.servers.end(); its++) {
            s = *its;
            if (FD_ISSET(s->sockFd, &readSet)) {
                try {
                    s->acceptNewClient();
                } catch (std::exception & e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            // std::vector<Client*>::iterator itc;
            // for (itc = (*its)->clients.begin(); itc != (*its)->clients.end(); itc++) {
            //     c = *itc;
            //     s->handleClientRequest(c);
            // }
        }
        gConfig.run = 0;
    }
    return (EXIT_SUCCESS);
}