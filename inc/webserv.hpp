#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <string>
# include <iostream>
# include <errno.h>  

# include "Config.hpp"
# include "Client.hpp"


extern Config gConfig;

int        putstrError(std::string str);

# endif