#include "../inc/Webserv.hpp"

namespace ft {


	std::string getcwdString() {

		char cwd[256];

		if (getcwd(cwd, sizeof(cwd)) != NULL)
			return (std::string(cwd));
		else return NULL;
	}



	std::string decodeBase64(std::string & token) {

		int val = 0;
		int valb = -8;
		unsigned char c;
		std::string out;
		std::vector<int> tab(256, -1);
		std::string::iterator it = token.begin();
		std::string::iterator itend = token.end();

		for (int i = 0; i < 64; i++)
			tab["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i; 
		for (; it != itend; it++) {
			c = *it;
			if (tab[c] == -1) break;
			val = (val << 6) + tab[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}

	void	getLine(std::string & buf, std::string & line)
	{
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

	std::vector<std::string>	split(const std::string & str, char delim)
	{
		size_t						start;
		size_t						end = 0;
		std::vector<std::string>	strings;

		// std::string::find_first_not_of searches the string for the first character that
		// does not match any of the characters specified in its arguments.
		while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
			end = str.find(delim, start);
			strings.push_back(str.substr(start, end - start));
		}
		return (strings);
	}

	std::vector<std::string>	splitWhtSp(std::string str)
	{
		size_t						start;
		size_t						end = 0;
		std::vector<std::string>	strings;

		for (size_t i = 0; i < str.length(); i++)
		{
			if (str[i] == '\f' || str[i] == '\t' || str[i] == '\n' \
			|| str[i] == '\r' || str[i] == '\v')
				str.replace(i, 1, " ");
		}
		while ((start = str.find_first_not_of(' ', end)) != std::string::npos)
		{
			end = str.find(' ', start);
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

	time_t		getTime(void)
	{
		struct timeval 	tv;
		struct timezone tz;

		gettimeofday(&tv, &tz);
		return (tv.tv_sec);
	}

	std::string 	getDate(void) {
		struct timeval 	tv;
		struct tm 		time;
		struct timezone tz;
		char 			buffer[1000];
		std::string 	date;

		gettimeofday(&tv, &tz);
		strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
		strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S CEST", &time);
		date = buffer;
		return (date);
	}

	std::string 	getLastModifDate(std::string file) {
		struct stat 	info;
		std::string 	sec;
		struct tm 		time;
		std::string 	date;
		char 			buffer[1000];

		stat(file.c_str(), &info);
		sec = std::to_string(info.st_mtime).c_str();
		strptime(sec.c_str(), "%s", &time);
		strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S CEST", &time);
		date = buffer;
		return (date);
	}

	int		isWhiteSpace(char c)
	{
		if (c == '\f' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == ' ')
			return (1);
		return (0);
	}

	int		isSpace(char c)
	{
		if (c == '\f' || c == '\t' || c == '\r' || c == '\v' || c == ' ')
			return (1);
		return (0);
	}

	bool	isNumber(std::string const &str)
	{
		std::string::const_iterator it = str.begin();
		while (it != str.end() && std::isdigit(*it))
			++it;
		return (!str.empty() && it == str.end());
	}

	int		countElem(std::string str, std::string target)
	{
		int		occ = 0;
		size_t	pos = 0;

		while ((pos = str.find(target, pos)) != std::string::npos)
		{
			pos += target.length();
			occ++;
		}
		return (occ);
	}
}

namespace utils
{
	void strTabFree(char ** strTab) {
		
		int i = -1;
		while (strTab[++i] != 0)
			free(strTab[i]);
		free(strTab);

	}

	void deleteCarriageReturn(std::string &str) {
		size_t pos = str.find_last_of('\r');
		if (pos != std::string::npos)
			str.erase(pos);
	}

	void displayHeaderMultiMap(std::multimap<float, std::string, std::greater<float> > data, std::string headerName) {

		if (data.empty())
				return ;

		std::multimap<float, std::string, std::greater<float> >::iterator it;
		std::multimap<float, std::string, std::greater<float> >::iterator tmp;

		std::cout << headerName << " : ";
		for (it = data.begin(); it != data.end(); it++) {
			std::cout << it->second << " ";

			std::cout << it->first << " ";

			tmp = data.end();
			--tmp;
			if (it != tmp)
				std::cout << "; ";
		}
		std::cout << std::endl;

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

	int		isExtension(std::string str, std::string ext) 
	{
		int i = str.size() - 1;
		std::string temp;
		while (i > 0 && str[i] != '.')
			i--;
		if (i == 0)
			return (0);
		temp = str.substr(i, str.size() - 1);
		if (temp == ext)
			return (1);
		return (0);
	}

}

namespace responseUtils
{
	std::string		getContentType(std::string file) {

		std::string		tmp;
		
		int	j = 0;
		int	i = file.size() - 1;

		std::string ext[67] = {"php", "aac", "abw", "arc", "avi", "azw", "bin", "bz", "bz2", "csh", "css", "csv", "doc", "docsx", "eot", "epub", "gif", "htm", "html", "ico",
		"ics", "jar", "jpeg", "jpg", "js", "json", "mid", "midi", "mpeg", "mpkg", "odp", "ods", "odt", "oga", "ogv", "ogx", "otf", "png", "pdf", "ppt", "pptx", "rar", "rtf", "sh"
		"svg", "swf", "tar", "tif", "tiff", "ts", "ttf", "vsd", "wav", "weba", "webm", "webp", "woff" ,"woff2", "xhtml", "xls", "xlsx","xml", "xul", "zip", "3gp", "3g2", "7z"};
		
		std::string ret[67] = {"text/html", "audio/aac", "application/x-abiword", "application/octet-stream", "video/x-msvideo", "application/vnd.amazon.ebook", "application/octet-stream", "application/x-bzip",
		"application/x-bzip2", "application/x-csh", "text/css", "text/csv", "application/msword", "application/vnd.openxmlformats-officedocument.wordprocessingml.document", "application/vnd.ms-fontobject",
		"application/epub+zip", "image/gif", "text/html", "text/html", "image/x-icon", "text/calendar", "application/java-archive", "image/jpeg", "image/jpeg" ,"application/javascript", "application/json",
		"audio/midi","audio/midi", "video/mpeg", "	application/vnd.apple.installer+xml", "application/vnd.oasis.opendocument.presentation", "application/vnd.oasis.opendocument.spreadsheet", "application/vnd.oasis.opendocument.text", "audio/ogg", "video/ogg", "application/ogg", "font/otf", "image/png", "application/pdf", "application/vnd.ms-powerpoint", "application/vnd.openxmlformats-officedocument.presentationml.presentation",
		"application/x-rar-compressed", "application/rtf", "application/x-sh", "image/svg+xml", "application/x-shockwave-flash", "application/x-tar", "image/tiff", "image/tiff", "application/typescript", "font/ttf", "application/vnd.visio", "audio/x-wav", "audio/webm", "video/webm", "image/webp", "font/woff", "font/woff2", "application/xhtml+xml", "application/vnd.ms-excel", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
		"application/xml", "application/vnd.mozilla.xul+xml", "application/zip", "video/3gpp", "video/3gpp2", "application/x-7z-compressed"};
		
		tmp.clear();
		while (file[i] != 0 && file[i] != '.')
			i--;
		tmp = file.substr(i + 1, file.size());
		while (j < 67)
		{
			if (ext[j] == tmp)
				return (ret[j]);
			j++;
		}
		return ("text/plain");
	}

	std::string getReasonPhrase(int code) {

	    std::map<int, std::string> reasonMap;

		if (code < 200 || code > 500) {
	        NOCLASSLOGPRINT(LOGERROR, "utils::getReasonPhrase() : Anormal status code ---> no reason phrase match");
			code = 500;
		}
    
		reasonMap[200] = "OK";
		reasonMap[201] = "Created";
		reasonMap[202] = "Accepted";
		reasonMap[204] = "No Content";
		reasonMap[305] = "Use Proxy";
		reasonMap[400] = "Bad Request";
		reasonMap[401] = "Unauthorized";
		reasonMap[404] = "Not Found";
		reasonMap[405] = "Method Not Allowed";
		reasonMap[413] = "Request Entity Too Large";
		reasonMap[414] = "Request-URI Too Long";
		reasonMap[495] = "SSL Certificate Error";
		reasonMap[500] = "Internal Server Error";
		reasonMap[501] = "Not Implemented";
		reasonMap[503] = "Service Unavailable";

		return reasonMap[code];

	}

	void headerFormat(std::string & respStr, std::string key, int value) {
		if (value == -1)
			return ;
		respStr.append(key);
		respStr.append(": ");
		respStr.append(std::to_string(value));
		respStr.append("\r\n");
	}

	void headerFormat(std::string & respStr, std::string key, std::string value) {
		if (value.empty())
			return ;
		respStr.append(key);
		respStr.append(": ");
		respStr.append(value);
		respStr.append("\r\n");
	}

	void headerFormat(std::string & respStr, std::string key, std::map<int, std::string> value) {
		size_t i = 0;
		if (!value.size())
			return ;
		respStr.append(key);
		respStr.append(":");
		while (i < value.size()) {
			respStr.append(" ");
			respStr.append(value[i]);
			if (i + 1 < value.size())
				respStr.append(";");
			i++;
		}
		respStr.append("\r\n");
	}
}