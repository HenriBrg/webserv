#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <vector>
# include <unistd.h>
# include <sys/select.h>
# include <sys/time.h>
# include <sys/types.h>

# include "Server.hpp"

class Config {
    
    public:
        
        int             run;

        fd_set          readSet;
        fd_set          writeSet;
        fd_set          readSetBackup;
        fd_set          writeSetBackup;

        struct  timeval         timeout;
        std::vector<Server*>    servers;

        Config();
        virtual ~Config();

        int     getMaxFds(void);

        void    init();
        void    resetFds();

    private:

        int     nfds;


};

#endif