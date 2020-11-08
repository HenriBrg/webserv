#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <signal.h>
# include <dirent.h>
# include <string.h>
# include <errno.h>
# include <time.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include <iostream>
# include <string>
# include <vector>
# include <list>
# include <queue>
# include <stack>
# include <map>
# include <algorithm>
# include <iterator>

# include "Config.hpp"
# include "Parser.hpp"
# include "Client.hpp"
# include "Logger.hpp"
# include "Utils.hpp"

extern Config gConfig;

#endif