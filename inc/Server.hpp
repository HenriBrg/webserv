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

# include <fcntl.h>

/* Local */

# include "Client.hpp"
# include "Location.hpp"
# include "Response.hpp"
# include "Request.hpp"

class Response;

class Server {
    
    private:
    
        Server();

    public:

        friend class Config;
        friend class Response;
        typedef void(Response::*ptr)(Request*);

        /* Constructors */

        Server(std::string name, int port, std::string error);
        virtual ~Server();

        /* Members variables */

        int                         port;
        int                         sockFd;
        int                         srvMaxFd; 

        std::string                 name;
        std::string                 error;
        struct sockaddr_in          addr;
        std::vector<Client*>        clients;
        std::vector<Location*>      locations;
        std::map<std::string, ptr> methodsTab;


        /* Members Methods */

        int start();
        

        void acceptNewClient(void);

        /* Request functions */
        void handleClientRequest(Client *c);

        void readClientRequest(Client *c);


        /* Response functions */
        void writeClientResponse(Client *c);
        void setClientResponse(Client *c);
        int sendClientResponse(Client *c);


        std::string const logInfo(void);

        /* Exceptions */

        class ServerException: public std::exception {
            private:
                std::string error;
            public:
                ServerException(std::string where, std::string error);
                virtual ~ServerException() throw();
                virtual const char *what() const throw();
        };
};

#endif