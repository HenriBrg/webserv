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
 
# define END		"\033[1;0m"

# define BCKBLACK	"\033[1;41m"
# define BCKWHITE	"\033[1;47m"



// INFO  : routine
// ERROR : syscall fail
// DEBUG : soucis temporaire juste pour différencier output

# define INFO 1
# define LOGERROR 2
# define DEBUG 3
# define DISCONNECT 4

# define LOGPRINT(PURPOSE, X, MESSAGE) Logger::print(PURPOSE, X, MESSAGE);
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
                case DEBUG:
                    std::cout << BLUE << " [DEBUG] " << END  << str << std::endl;
                    break ;
                case DISCONNECT:
                    std::cout << YELLOW << " [CLOSE] " << END  << str << std::endl;
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
                    std::cout << RED << " [ERROR] " << END << message << std::endl;
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
