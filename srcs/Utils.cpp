#include "../inc/Webserv.hpp"

namespace ft {

	void	getLine(std::string & buf, std::string & line) {
		size_t pos;

		pos = buf.find("\n");
		if (pos != std::string::npos) {
			line = std::string(buf, 0, pos++);
			buf = buf.substr(pos);
		} else {
			if (buf[buf.size() - 1] == '\n')
				buf = buf.substr(buf.size());
			else {
				line = buf;
				buf = buf.substr(buf.size());
			}
		}
	}


	std::vector<std::string> split(const std::string & str, char delim) {
		size_t                      start;
		size_t                      end = 0;
		std::vector<std::string>    strings;

		// std::string::find_first_not_of searches the string for the first character that
		// does not match any of the characters specified in its arguments.
		while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
			end = str.find(delim, start);
			strings.push_back(str.substr(start, end - start));
		}
		return (strings);
	}

	std::string trim(const std::string& str) {
		size_t first = str.find_first_not_of(' ');

		if (std::string::npos == first)
			return str;
		size_t last = str.find_last_not_of(' ');
		return (std::string(str.substr(first, (last - first + 1))));
	}

	std::string		getTimestamp(void) {
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

}

namespace utils {

	void deleteCarriageReturn(std::string &str) {
		size_t pos = str.find_last_of('\r');
		if (pos != std::string::npos)
			str.erase(pos);
	}

	void displayHeaderMap(std::map<int, std::string> data, std::string headerName) {
		if (data.empty())
			return ;
		std::map<int, std::string>::iterator it = data.begin();
		std::map<int, std::string>::iterator ite = data.end();
		std::map<int, std::string>::iterator tmp;

		std::cout << headerName << " : ";
		for (; it != ite; it++) {
			std::cout << it->second;

			tmp = data.end();
			--tmp;
			if (it != tmp)
				std::cout << ", ";
		}
		std::cout << std::endl;
	}

	int strHexaToDecimal(std::string strHexa) {    

		int ret = 0; 
		int base = 1; 
		int size = strHexa.size(); 

		for (int i = size - 1; i >= 0; --i) {    
			if (strHexa[i] >= '0' && strHexa[i] <= '9') { 
				ret += (strHexa[i] - 48) * base;
				base = base * 16; 
			} 
			else if (strHexa[i] >= 'A' && strHexa[i] <= 'F') { 
				ret += (strHexa[i] - 55)*base; 
				base = base * 16; 
			} 
			else if (strHexa[i] >= 'a' && strHexa[i] <= 'f') { 
				ret += (strHexa[i] - 87)*base; 
				base = base * 16; 
			} 
		}
		return (ret); 
	} 

}
