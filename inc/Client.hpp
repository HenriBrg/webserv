#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <unistd.h>

# include <sys/select.h>
# include <sys/time.h>
# include <sys/types.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include "Request.hpp"
# include "Response.hpp"


# define BUFMAX 4096

class Server;

class Client {
    
    friend class Server;
    friend class Config;


    public:


        // Etat de réception de la requête
        int         recvStatus;
        enum {
            HEADER,
            BODY,
            COMPLETE,
            WAIT,
            ERROR
        };

        int         port;
        int         acceptFd;

        Server      *server;
        std::string ip;
        char        *buf;
        std::string strbuf;

        Request     req;
        Response    res;

        Client(Server *server, int acceptFd, struct sockaddr_in clientAddr);
        virtual ~Client();
        void reset();


    private:

        Client();



};

#endif