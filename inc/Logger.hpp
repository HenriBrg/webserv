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

# define INFO 1
# define ERROR 2
# define DEBUG 3

# define LOGPRINT(PURPOSE, SERVER, MESSAGE) Logger::print(PURPOSE, SERVER, MESSAGE);

class Logger {

    public:
    
        static void print(int type, Server *srv, std::string const & message) {

            std::string timestamp;
            std::string str;

            std::cout << getTimestamp();

            if (srv) {
                str = "[:" + std::to_string(srv->port) + "] ";
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
