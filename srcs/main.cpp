# include "../inc/Webserv.hpp"

Config gConfig;

int main(int ac, char **av) {

    Server *s;
	Client *c;

    gConfig.parse();
    while (gConfig.run) {

        gConfig.resetFds();
        select(gConfig.getNfds(), &gConfig.rFds, &gConfig.wFds, NULL, NULL);
        std::vector<Server*>::iterator its;
        for (its = gConfig.servers.begin(); its != gConfig.servers.end(); its++) {
            
            s = *its; // 1) On itère sur les serveurs pour vérifier si l'un deux fait l'objet d'une requête
            if (FD_ISSET(s->sockFd, &gConfig.rFds)) {
                try {
                    s->acceptNewClient();
                } catch (std::exception & e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            // 2) On itère sur les clients afin de trouver celui ayant été à l'origine du "déblocage" de select()
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