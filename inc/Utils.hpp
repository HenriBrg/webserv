# ifndef UTILS_HPP
# define UTILS_HPP

#include <string>
#include <iostream>

namespace ft {

    std::string		getTimestamp(void);
    int             putstrError(std::string str);
    void	        getLine(std::string & buf, std::string & line);
    std::vector<std::string> split(const std::string & str, char delim);
	std::string     trim(const std::string& str);
	void            deleteCarriageReturn(std::string &str);
	std::string 	decodeBase64(std::string & token);


}

namespace utils {

	void deleteCarriageReturn(std::string &str);
	void displayHeaderMap(std::map<int, std::string> data, std::string headerName);
	int strHexaToDecimal(std::string strHexa);

}

# endif