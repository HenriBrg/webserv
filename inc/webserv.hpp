#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <string>
# include <iostream>

# include <sys/select.h>
# include <sys/time.h>
# include <sys/types.h>

# include "Config.hpp"

int        putstrError(std::string str);

# endif