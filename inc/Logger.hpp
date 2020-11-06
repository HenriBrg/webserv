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

# define BCKYELLOW	    "\033[1;43m"
# define BCKBLACK	    "\033[38;5;41m"

# define BOLD       	"\033[1;1m"
# define BOLD_OFF		"\033[1;21m"

# define INFO 1
# define LOGERROR 2
# define REQERROR 3
# define RESERROR 4
# define DEBUG 5
# define DISCONNECT 6
# define SELECT 7
# define OK 8

# define LOGPRINT(PURPOSE, X, MESSAGE) Logger::print(PURPOSE, X, MESSAGE);
# define NOCLASSLOGPRINT(PURPOSE, MESSAGE) Logger::noClassLogPrint(PURPOSE, MESSAGE);

class Logger {

    public:
        template <typename T>
        static void print(int type, T *x, std::string const & message) {

            if (SILENTLOGS == 1)
                return ;
            
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
                    std::cout << ORANGE << " [RQERR] " << END << str << std::endl;
                    break ;
                 case RESERROR:
                    std::cout << ORANGE << " [RSERR] " << END << str << std::endl;
                    break ;
                case DEBUG:
                    std::cout << BLUE << " [DEBUG] " << END  << str << std::endl;
                    break ;
                case DISCONNECT:
                    std::cout << MAGENTA << " [CLOSE] " << END  << str << std::endl;
                    break ;
                case OK:
                    std::cout << GREEN << " [BYE]   " << END  << str << std::endl;
                    break ;
                default:
                    break ;
            }
        }

        static void noClassLogPrint(int type, std::string const & message) {

            if (SILENTLOGS == 1)
                return ;

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
                case SELECT:
                     std::cout << " " << BCKYELLOW << BOLD << BLACK << message  << END << BOLD_OFF << END  << std::endl;
                     break ;
                case REQERROR:
                    std::cout << ORANGE << " [RQERR] " << END << message << std::endl;
                    break ;
                case RESERROR:
                    std::cout << ORANGE << " [RSERR] " << END << message << std::endl;
                    break ;
                default:
                    break ;
            }

        }

};

# endif
