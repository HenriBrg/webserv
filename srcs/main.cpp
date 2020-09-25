# include "../inc/Webserv.hpp"

Config gConfig;

int main(int ac, char **av) {

    Server *s;
	Client *c;
    
    gConfig.init();
    while (gConfig.run) {
        gConfig.resetFds();
        select(gConfig.getMaxFds(), &gConfig.readSet, &gConfig.writeSet, NULL, &gConfig.timeout);
        std::vector<Server*>::iterator its;
        for (its = gConfig.servers.begin(); its != gConfig.servers.end(); its++) {
            s = *its;
            if (FD_ISSET(s->sockFd, &gConfig.readSet)) {
                try {
                    s->acceptNewClient();
                    std::cout << "NEW CLIENT" << std::endl;
                } catch (std::exception & e) {
                    std::cerr << e.what() << std::endl;
                }
            }

            std::vector<Client*>::iterator itc;
            for (itc = (*its)->clients.begin(); itc != (*its)->clients.end(); itc++) {
                c = *itc;
                s->handleClientRequest(c);
            }
            
        }
        gConfig.run = 0;
    }
    return (EXIT_SUCCESS);
}