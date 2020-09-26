# ifndef LOGGER_HPP
# define LOGGER_HPP

# include <string>
# include <iostream>

# include "Server.hpp"
# include "Utils.hpp"

// TODO : ADD LOG FILE
// TODO : ADD TIMESTAMP

# define RED		"\033[1;31m"
# define GREEN		"\033[1;32m"
# define YELLOW		"\033[1;33m"
# define BLUE		"\033[1;36m"

# define END		"\033[1;0m"

// INFO  : routine
// ERROR : syscall fail
// DEBUG : soucis temporaire juste pour différencier output

# define INFO 1
# define ERROR 2
# define DEBUG 3

# define LOGPRINT(PURPOSE, SERVER, MESSAGE) Logger::print(PURPOSE, SERVER, MESSAGE);

class Logger {

    public:
    
        template <typename T>
        static void print(int type, T *x, std::string const & message) {

            std::string timestamp;
            std::string str;

            std::cout << getTimestamp();

            if (x != NULL) {
                str = "[:" + std::to_string(x->port) + "] ";
            }
            str += message;
            switch (type) {
                case INFO:
                    std::cout << YELLOW << " [INFO] " << END  << str << std::endl;
                    break ;
                case ERROR:
                    std::cout << RED << " [ERROR] " << END << str << std::endl;
                    break ;
                case DEBUG:
                    std::cout << BLUE << " [DEBUG] " << END  << str << std::endl;
                    break ;
                default:
                    break ;
            }

        }

};

# endif
