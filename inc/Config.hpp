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

        fd_set          rFds;
        fd_set          wFds;
        fd_set          eFds;

        fd_set          rFdsBackup;
        fd_set          wFdsBackup;
        fd_set          eFdsBackup;

        struct  timeval         timeout;
        std::vector<Server*>    servers;

        Config();
        virtual ~Config();

        int     getNfds(void) const;
        void    setNfds(int newFd);

        void    resetFds();
        void    parse();

    private:

        int     nfds;


};

#endif