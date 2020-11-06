#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <fstream>
# include <unistd.h>

# include <sys/select.h>
# include <sys/time.h>
# include <sys/types.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include "Request.hpp"
# include "Response.hpp"

# define BUFMAX 32768

# define STDIN 0
# define STDOUT 1

# define TUBE_OUT 0
# define TUBE_IN 1


class Server;

class Client {
    
    friend class Server;
    friend class Config;

    public:

        /* Etat de réception de la requête */
        enum {
            HEADER,
            BODY,
            COMPLETE,
            WAIT,
            ERROR,
            CGI            
        };

        /* The client might disconnect while we're handle its request */
        bool        isConnected;
        bool        _isAccepted;
        int         recvStatus;

        int         port;
        int         acceptFd;
        time_t      _lastRequest;

        Server      *server;
        std::string ip;
        char        *buf;
        std::string strbuf;

        Request     req;
        Response    res;

        pid_t       cgipid;
        
        Client(int acceptFd, Server *server, struct sockaddr_in clientAddr);
        virtual ~Client();
        void reset();
        std::string const logInfo(void);
        void resetTimeOut(void);

    private:

        Client();

};

#endif