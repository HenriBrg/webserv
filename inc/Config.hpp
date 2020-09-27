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

class Config {
    
    public:
        
        int             run;
        int             nfds;

        fd_set          readSet;
        fd_set          writeSet;
        fd_set          readSetBackup;
        fd_set          writeSetBackup;
        // struct  timeval         timeout;

        // TODO : OPTI : Vec ou List ?
        std::set<int>           activeFds;
        std::vector<Server*>    servers;

        Config();
        virtual ~Config();

        int     getMaxFds(void);
        void    addFd(int fd);
        void    removeFd(int fd);
        void    resetFds();

        void    init();

    private:



};

#endif