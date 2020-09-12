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

class Server;

class Client {
    
    friend class Server;
    friend class Config;


    public:

        int         port;
        int         acceptFd;
        Server      *server;
        std::string ip;

        // Request     req;
        // Response    res;

        Client(Server *server, int acceptFd, struct sockaddr_in clientAddr);

        
        virtual ~Client();

    private:

        Client();



};

#endif