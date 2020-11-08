#include "../inc/Webserv.hpp"

Config gConfig;

int init(int ac, char **av) {

	std::string configuration;

	if (ac > 2) {
		std::cout << "Usage: ./webserv <configuration file>" << std::endl;
		return (-1);
	} else {
		if (ac == 1) configuration = DEFAULT_CONF_PATH;
		else {
			struct stat statbuf;
			if (stat(av[1], &statbuf) == 0 && !S_ISDIR(statbuf.st_mode)) {
				configuration = av[1];
				if (configuration.compare(configuration.size() - 5, 5, ".conf") != 0) {
					std::cout << "Error: file must be of type .conf" << std::endl;
					return (-1);
				}
			}
			else {
				std::cout << "Error: could not find configuration file" << std::endl;
				return (-1);
			}
		}
	}

	Conf webconf(&configuration[0]);
	try {
		webconf.parseConf();
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
    gConfig.servers = webconf.getServers();
    signal(SIGINT, handleCTRLC);
    if (SILENTLOGS == 1) std::cout << "\n     ----- Webserv is running in silent mode ----- \n" << std::endl;
    return (0);
}

int main(int ac, char **av) {
	
	Server *s;
	Client *c;

	if (init(ac, av))
		return (-1);
	while (gConfig.run) {
		gConfig.resetFds();
		NOCLASSLOGPRINT(SELECT, ("----- ----- ----- ----- ----- NEW SELECT() CALL ----- ----- ----- ----- ----- "));
		gConfig.showFDSETS();
		select(gConfig.getMaxFds(), &gConfig.readSet, &gConfig.writeSet, NULL, NULL);
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
					if (s->clients.empty()) break ;
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