# include "../inc/Webserv.hpp"

Config gConfig;

int main(int ac, char **av) {

    Server *s;
	Client *c;

    gConfig.init();
    while (gConfig.run) {
        gConfig.resetFds();
        NOCLASSLOGPRINT(INFO, ("Select call with maxFds = " + std::to_string(gConfig.getMaxFds()) + "\n"));
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
                s->handleClientRequest(c);
            }
        }
    }
    return (EXIT_SUCCESS);
}