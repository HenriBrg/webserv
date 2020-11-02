#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <vector>
# include <set>
# include <unistd.h>
# include <sys/select.h>
# include <sys/time.h>
# include <sys/types.h>
# include <algorithm>

# include "Server.hpp"

void    handleCTRLC(int s);

class Config {
    
    public:
        int             run;

        fd_set          readSet;
        fd_set          writeSet;
        fd_set          readSetBackup;
        fd_set          writeSetBackup;

        std::set<int>           activeFds;
        static int              _availableConnections;
        std::vector<Server*>    servers;

        Config();
        virtual ~Config();

        int     getMaxFds(void);
        void    addFd(int fd);
        void    removeFd(int fd);
        void    resetFds();
        static void    webservShutdown();
        void    showFDSETS();


    private:

};



# endif