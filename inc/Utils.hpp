#ifndef UTILS_HPP
# define UTILS_HPP

# include "Webserv.hpp"

namespace ft
{
	time_t						getTime(void);
    std::string					getTimestamp(void);
	std::string					getDate(void);
	std::string					getLastModifDate(std::string file);
    int							putstrError(std::string str);
    void						getLine(std::string & buf, std::string & line);
    std::vector<std::string>	split(const std::string & str, char delim);
	std::vector<std::string>	splitWhtSp(std::string str);
	std::string					trim(const std::string& str);
	std::string					decodeBase64(std::string & token);
	int							isWhiteSpace(char c);
	int							isSpace(char c);
	bool						isNumber(std::string const &str);
	int							countElem(std::string str, std::string target);
	std::string 				getcwdString();
    int             			putstrError(std::string str);
	int						ft_strstr(char *str, std::string search, int size);

}

namespace utils {

	void						strTabFree(char ** strTab);
	void deleteCarriageReturn(std::string &str);
	void displayHeaderMap(std::map<int, std::string> data, std::string headerName);
	void displayHeaderMultiMap(std::multimap<float, std::string, std::greater<float> > data, std::string headerName);
	int  strHexaToDecimal(std::string strHexa);
	int	 isExtension(std::string str, std::string ext);

}

namespace responseUtils {

	std::string	getContentType(std::string file);
	int getContentLength(std::string file);
	int setupBytesArray(Response *res);
	void copyBytes(char* dest, const char* src, size_t limit, size_t offset);
	char* setBodyNoFile(std::string src, int size, int &contentLength);
	std::string getReasonPhrase(int code);
	void headerFormat(std::string & respStr, std::string key, int value);
	void headerFormat(std::string & respStr, std::string key, std::string value);
	void headerFormat(std::string & respStr, std::string key, std::map<int, std::string> value);

}

#endif