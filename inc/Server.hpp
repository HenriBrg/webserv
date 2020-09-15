#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <vector>

# include <unistd.h>
# include <string.h>
# include <stdio.h>
# include <inttypes.h>

/* Sockets */

# include <sys/socket.h> 
# include <netinet/in.h> 
# include <netinet/ip.h>
# include <sys/types.h>

#include <fcntl.h>

/* Local */

# include "Client.hpp"


class Server {
    
    private:
    
        Server();

    public:

        friend class Config;
        
        /* Constructors */

        Server(std::string name, int port);
        virtual ~Server();

        /* Members variables */

        int                         port;
        int                         sockFd;
        std::string                 name;
        struct sockaddr_in          addr;
        std::vector<Client*>        clients;

        /* Members Methods */

        int  start(void);
        void acceptNewClient(void);
        void handleClientRequest(Client *c);
        int readClientRequest(Client *c);
        int writeClientResponse(Client *c);

        /* Exceptions */

        class ServerException: public std::exception {

            private:

                std::string error;
                ServerException();

            public:

                ServerException(std::string where, std::string error);
                virtual ~ServerException() throw();
                virtual const char *what() const throw();

        };
};

#endif