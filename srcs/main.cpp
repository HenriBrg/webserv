# include "../inc/Webserv.hpp"

Config gConfig;

int main(int ac, char **av) {

    Server *s;
	Client *c;

    gConfig.init();
    signal(SIGINT, handleCTRLC);

    while (gConfig.run) {
        gConfig.resetFds();
        NOCLASSLOGPRINT(INFO, ("New SELECT() CALL"));
        gConfig.showFDSETS();
        select(gConfig.getMaxFds(), &gConfig.readSet, &gConfig.writeSet, NULL, NULL);
        std::vector<Server*>::iterator its = gConfig.servers.begin();
        for (; its != gConfig.servers.end(); its++) {
            s = *its;
            if (FD_ISSET(s->sockFd, &gConfig.readSet)) {
                try {
                    s->acceptNewClient();
                } catch (std::exception & e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            std::vector<Client*>::iterator itc = s->clients.begin();
            for (; itc != s->clients.end(); itc++) {
                c = *itc;

                // std::cout << std::to_string(!!c->isConnected) << std::endl;

                if (c->isConnected == false) {

                    NOCLASSLOGPRINT(DEBUG, ("DESTROY CLIENT 1"));

                    delete c;
                    itc = s->clients.erase(itc);
                    if (s->clients.empty())
                        break ;
                    else {
                        itc = s->clients.begin();
                        continue ;
                    }
                }

                s->handleClientRequest(c);

                if (c->isConnected == false) {

                    NOCLASSLOGPRINT(DEBUG, ("DESTROY CLIENT 2"));

                    delete c;
                    itc = s->clients.erase(itc);
                    if (s->clients.empty())
                        break ;
                    else {
                        itc = s->clients.begin();
                        continue ;
                    }
                }

                // if (c->res._sendStatus == Response::DONE) {
                //     NOCLASSLOGPRINT(DEBUG, ("DESTROY CLIENT 3"));

                //     delete c;
                //     itc = s->clients.erase(itc);
                //     if (s->clients.empty())
                //         break ;
                //     else {
                //         itc = s->clients.begin();
                //         continue ;
                //     }
                // }

            }
        }
    }
    return (EXIT_SUCCESS);
}