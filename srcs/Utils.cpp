#include "../inc/Webserv.hpp"

std::string		getTimestamp(void)
{
	time_t 		now = 0;
	tm 			*ltm = NULL;
	char		buffer[1024];
	std::string result;
	
	now = time(0);
	if (now)
		ltm = localtime(&now);
	strftime(buffer, 1024, "%d/%m/%y %T", ltm);
	result = buffer;
	result.insert(result.begin(), '[');
	result.insert(result.end(), ']');
   	return (result);
}