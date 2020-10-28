# include "../inc/Webserv.hpp"

Config gConfig;

int main(int ac, char **av) {

    Server *s;
	Client *c;

    gConfig.init();
    signal(SIGINT, handleCTRLC);
    while (gConfig.run) {
        gConfig.resetFds();
        NOCLASSLOGPRINT(INFO, ("----- ----- ----- ----- ----- NEW SELECT() CALL ----- ----- ----- ----- -----"));
        gConfig.showFDSETS();
        select(gConfig.getMaxFds(), &gConfig.readSet, &gConfig.writeSet, NULL, NULL);
        NOCLASSLOGPRINT(INFO, ("SELECT() trigerred !"));
        std::vector<Server*>::iterator its = gConfig.servers.begin();
        for (; its != gConfig.servers.end(); its++) {
            s = *its;
            if (FD_ISSET(s->sockFd, &gConfig.readSet))
                    s->acceptNewClient();
            std::vector<Client*>::iterator itc = s->clients.begin();
            for (; itc != s->clients.end(); itc++) {
                c = *itc;

                s->handleClientRequest(c);

                if (c->isConnected == false) {
                    delete c;
                    itc = s->clients.erase(itc);
                    if (s->clients.empty())
                        break ;
                    else {
                        itc = s->clients.begin();
                        continue ;
                    }
                }

            }
        }
    }
    return (EXIT_SUCCESS);
}