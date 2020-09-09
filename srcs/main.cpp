# include "../inc/webserv.hpp"


/*******************************************************************************/
/*                                                                             */
/*******************************************************************************/

Config           config;
// Client           *client;
// Server           *server;

int main(int ac, char **av) {

    fd_set              readSet;
    fd_set              writeSet;
    struct     timeval  timeout;

    // if (ac != 2)
    //     return putstrError("A configuration file is required");
    
    // TRY - PARSING

    config.run = 1;
    while (config.run) {
    }

    return (EXIT_SUCCESS);
}