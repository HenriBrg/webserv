# include "../inc/Webserv.hpp"

/*******************************************************************************/
/*                                                                             */
/*******************************************************************************/

Config gConfig;

int putstrError(std::string str) {
    std::cerr << str << std::endl;
    return (EXIT_FAILURE);
}

int main(int ac, char **av) {

    Server *s;
	Client *c;

    // if (ac != 2)
    //     return putstrError("A configuration file is required");
    
    gConfig.parse();
    while (gConfig.run) {

        gConfig.resetFds();
        select(gConfig.getNfds(), &gConfig.rFds, &gConfig.wFds, &gConfig.eFds, &gConfig.timeout);
                
        std::vector<Server*>::iterator its;
        for (its = gConfig.servers.begin(); its != gConfig.servers.end(); its++) {
            
            s = *its;

            // 1) On itère sur les serveurs pour vérifier si l'un deux fait l'objet d'une requête
            
            if (FD_ISSET(s->sockFd, &gConfig.rFds)) {
                try {
                    s->acceptNewClient();
                } catch (std::exception & e) {
                    std::cerr << e.what() << std::endl;
                }
            }







            // 2) On itère sur le client (ayant été à l'origine du "déblocage" de select() et qui sera associé au serveur)
            //    afin de produire puis lui envoyer une réponse propre au contenu de la requête et à la configuration du serveur

            std::vector<Client*>::iterator itc;
            for(itc = (*its)->clients.begin(); itc != (*its)->clients.end(); itc++) {
                c = *itc;
                s->handleClientRequest(c);
            }

        }

        gConfig.run = 0;
    }
    return (EXIT_SUCCESS);
}