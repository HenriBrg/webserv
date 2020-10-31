# ifndef LOGGER_HPP
# define LOGGER_HPP

# include <string>
# include <iostream>

# include "Server.hpp"
# include "Utils.hpp"

// Utiliser std::cout << __LINE__ << std::endl;
// Utiliser std::cout << __FILE__ << std::endl;
// Utiliser std::cout << __DATE__ << std::endl;
// Utiliser std::cout << __TIME__ << std::endl;

// https://stackoverflow.com/a/2616912/10830328


# define BLACK		"\033[1;30m"
# define RED		"\033[1;31m"
# define GREEN		"\033[1;32m"
# define YELLOW		"\033[1;33m"
# define MAGENTA	"\033[1;35m"
# define BLUE		"\033[1;36m"
# define WHITE	    "\033[1;37m"
# define END		"\033[0m"


// # define BLACK		"\033[38;5;0m"
// # define RED		"\033[38;5;160m"
// # define GREEN		"\033[38;5;40m"
// # define YELLOW		"\033[38;5;220m"
// # define MAGENTA	"\033[38;5;99m"
// # define BLUE		"\033[38;5;31m"
// # define WHITE	    "\033[38;5;15m"
# define ORANGE	    "\033[38;5;208m"


# define BCKBLACK	"\033[38;5;41m"
# define BCKWHITE	"\033[38;5;47m"

// # define END		"\033[1;0m"



// INFO  : routine
// ERROR : syscall fail
// DEBUG : soucis temporaire juste pour différencier output


// TODO : change the hierarchy, add better filter
// exemple :


/*

INFO1 : info importante
INFO2 : moyen
INFO3 : vraiment peu utile

*/

# define INFO 1
# define LOGERROR 2
# define REQERROR 3
# define DEBUG 4
# define DISCONNECT 5

# define LOGPRINT(PURPOSE, X, MESSAGE) Logger::print(PURPOSE, X, MESSAGE);

// delete noclass log
# define NOCLASSLOGPRINT(PURPOSE, MESSAGE) Logger::noClassLogPrint(PURPOSE, MESSAGE);

class Logger {

    public:
        template <typename T>
        static void print(int type, T *x, std::string const & message) {

            std::string str;
            std::string timestamp;

            std::cout << ft::getTimestamp();
            str = x->logInfo();
            if (!message.empty())
                str += " | " + message;
            switch (type) {
                case INFO:
                    std::cout << YELLOW << " [INFO]  " << END << str << std::endl;
                    break ;
                case LOGERROR:
                    std::cout << RED << " [ERROR] " << END << str << std::endl;
                    break ;
                case REQERROR:
                    std::cout << ORANGE << " [REQ ERROR] " << END << str << std::endl;
                    break ;
                case DEBUG:
                    std::cout << BLUE << " [DEBUG] " << END  << str << std::endl;
                    break ;
                case DISCONNECT:
                    std::cout << MAGENTA << " [CLOSE] " << END  << str << std::endl;
                default:
                    break ;
            }
        }

        // Fonction requise car impossible de passer NULL à un paramètre définit par template
        static void noClassLogPrint(int type, std::string const & message) {

            std::string timestamp;
            std::cout << ft::getTimestamp();
            switch (type) {
                case INFO:
                    std::cout << YELLOW << " [INFO]  " << END << message << std::endl;
                    break ;
                case LOGERROR:
                    std::cerr << RED << " [ERROR] " << END << message << std::endl;
                    break ;
                case DEBUG:
                    std::cout << BLUE << " [DEBUG] " << END  << message << std::endl;
                    break ;
                default:
                    break ;
            }

        }

};

# endif
