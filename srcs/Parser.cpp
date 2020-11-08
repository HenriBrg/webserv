/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Conf.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rofernan <rofernan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/23 12:49:31 by rofernan          #+#    #+#             */
/*   Updated: 2020/10/23 12:49:34 by rofernan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../inc/Parser.hpp"

Conf::Conf() {}

Conf::Conf(char *filename): _fileName(filename), _confFile("") {}

Conf::~Conf() {
}

Conf	&Conf::operator=(const Conf &copy) {
	(void)copy;
	return (*this);
}

void	Conf::openFile(void) {
	
	int		ret;
	int		fd;
	char	buf[BUFFMAX + 1];

	fd = open(_fileName, O_RDONLY);
	while ((ret = read(fd, buf, BUFFMAX)) > 0)
	{
		buf[ret] = '\0';
		_confFile += buf;
	}
	close(fd);
}

std::string	Conf::parseLocation(Server *serv, std::string locs) {

	int			max_body = -1, beg, end;
	bool		autoindex = false;
	std::string	uri, root, index, methods, auth, cgi, phpcgi, ext;
	std::string	location;
	std::vector<std::string>	locBlock;
	std::vector<std::string>	line;

	while ((beg = locs.find("{") != std::string::npos))
	{
		end = locs.find("}");
		location = locs.substr(beg, end + 1 - beg);
		if (ft::countElem(location, "\troot ") != 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter root must be present once.");
		if (ft::countElem(location, "\tindex ") > 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter index must be present max once.");
		if (ft::countElem(location, "\tmethod ") != 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter method must be present once.");
		if (ft::countElem(location, "\tauth ") > 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter auth must be present max once.");
		if (ft::countElem(location, "\tmax_body ") > 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter max_body must be present max once.");
		if (ft::countElem(location, "\tautoindex ") > 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter autoindex must be present max once.");
		if (ft::countElem(location, "\tphp ") > 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter php must be present max once.");
		if (ft::countElem(location, "\tcgi ") > 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter cgi must be present max once.");
		if (ft::countElem(location, "\text ") > 1)
			return ("in server port " + std::to_string(serv->port) + ": parameter ext must be present max once.");
		locBlock = ft::split(location, '\n');
		for (size_t i = 0; i < locBlock.size(); i++)
		{
			line = ft::splitWhtSp(locBlock[i]);
			if (!line.empty())
			{
				if (line[0] == "location")
				{
					if (line.size() != 3 || line[1][0] != '/' || line[2] != "{")
						return ("in server port " + std::to_string(serv->port) + ": location syntax.");
					uri = line[1];
				}
				else if (line[0] == "root")
				{
					if (line.size() != 2)
						return ("in server port " + std::to_string(serv->port) + ": root syntax.");
					DIR *dir;
					if (!(dir = opendir(line[1].c_str())))
					{
						closedir(dir);
						return ("in server port " + std::to_string(serv->port) + ": cannot find root path.");
					}
					closedir(dir);
					root = line[1];
				}
				else if (line[0] == "index")
				{
					if (line.size() != 2)
						return ("in server port " + std::to_string(serv->port) + ": index syntax.");
					index = line[1];
				}
				else if (line[0] == "method")
				{
					if (line.size() != 2)
						return ("in server port " + std::to_string(serv->port) + ": method syntax.");
					methods = line[1];
				}
				else if (line[0] == "auth")
				{
					if (line.size() != 2)
						return ("in server port " + std::to_string(serv->port) + ": auth syntax.");
					auth = line[1];
				}
				else if (line[0] == "cgi")
				{
					if (line.size() != 2)
						return ("in server port " + std::to_string(serv->port) + ": cgi syntax.");
					cgi = line[1];
				}
				else if (line[0] == "php")
				{
					if (line.size() != 2)
						return ("in server port " + std::to_string(serv->port) + ": php syntax.");
					phpcgi = line[1];
				}
				else if (line[0] == "ext")
				{
					if (line.size() != 2)
						return ("in server port " + std::to_string(serv->port) + ": ext syntax.");
					ext = line[1];
				}
				else if (line[0] == "max_body")
				{
					if (line.size() != 2 || !ft::isNumber(line[1]))
						return ("in server port " + std::to_string(serv->port) + ": max_body syntax.");
					max_body = std::stoi(line[1]);
				}
				else if (line[0] == "autoindex")
				{
					if (line.size() != 2 || (line[1] != "on" && line[1] != "off"))
						return ("in server port " + std::to_string(serv->port) + ": autoindex syntax.");
					if (line[1] == "on")
						autoindex = true;
				}
				else if (line[0] != "}")
					return ("in server port " + std::to_string(serv->port) + ": unknown parameter.");
			}
			if (autoindex == true && !index.empty())
				return ("in server port " + std::to_string(serv->port) + ": index must be asbent if autoindex is on.");
			line.clear();
		}
		locBlock.clear();
		Location *loc = new Location(uri, root, index, methods, max_body, auth, cgi, phpcgi, ext, autoindex);
		serv->locations.push_back(loc);
		locs.erase(0, end + 1);
		NOCLASSLOGPRINT(INFO, ("Conf::parseLocation() | uri: " + uri + " | root: " + root + " | index: " + index + " | methods: " + methods + " | max_body: " + std::to_string(max_body) + " | auth: " + auth + " | cgi: " + cgi + " | php: " + phpcgi + " | ext: " + ext + " | autoindex: " + std::to_string(autoindex)));
		uri.clear();
		root.clear();
		index.clear();
		methods.clear();
		auth.clear();
		auth.clear();
		cgi.clear();
		phpcgi.clear();
		ext.clear();
		max_body = -1;
		autoindex = false;
	}
	return ("0");
}

std::string	Conf::parseServerBlock(std::string block)
{
	std::vector<std::string>	serverBlock;
	std::vector<std::string>	line;

	int			port = 0;
	std::string	serverName;
	static int	nameNb = 1;
	std::string	error;
	int			nbLoc;
	int			idx;
	std::string	servParams;

	if ((nbLoc = ft::countElem(block, "\tlocation ")) < 1)
		return ("parameter location must be present at least once.");
	idx = block.find("\tlocation");
	servParams = block.substr(0, idx);
	if (ft::countElem(servParams, "\tlisten ") != 1)
		return ("parameter listen must be present once.");
	if (ft::countElem(servParams, "\tserver_name ") > 1)
		return ("parameter server_name must be present max once.");
	if (ft::countElem(servParams, "\terror ") != 1)
		return ("there must be one errors path.");
	serverBlock = ft::split(servParams, '\n');
	for (size_t i = 0; i < serverBlock.size(); i++)
	{
		line = ft::splitWhtSp(serverBlock[i]);
		if (line[0] == "listen")
		{
			if (line.size() != 2 || !ft::isNumber(line[1]))
				return ("port syntax.");
			port = std::stoi(line[1]);
			if (port <= 1024)
				return ("port must be over 1024.");
		}
		else if (line[0] == "server_name")
		{
			if (line.size() != 2)
				return ("server name syntax.");
			serverName = line[1];
		}
		else if (line[0] == "error")
		{
			DIR *dir = NULL;
			if (line.size() != 2 || !(dir = opendir(line[1].c_str()))) {
				closedir(dir);
				return ("cannot find errors path.");
			}
			closedir(dir);
			error = line[1];
		}
		else
			return ("wrong server parameter.");
		line.clear();
	}
	serverBlock.clear();
	servParams = block.substr(idx, block.size() - idx - 1);
	if (serverName == "")
		serverName = "webserv" + std::to_string(nameNb++);
	Server *serv = new Server(port, serverName, error);
	NOCLASSLOGPRINT(INFO, ("Conf::parseServerBlock() : FOR SERVER PORT " + std::to_string(port)));
	if ((_errMsg = parseLocation(serv, servParams)) != "0")
	{
		for (std::vector<Location*>::iterator itl = serv->locations.begin(); itl != serv->locations.end(); itl++)
		{
			Location *loc = *itl;
			delete loc;
		}
		delete serv;
		return (_errMsg);
	}
	try
	{
        serv->start();
	}
	catch (std::exception &e)
	{
        std::cerr << e.what() << std::endl;
    }
	_servers.push_back(serv);
	for (size_t i = 0; i < _servers.size() - 1; i++) {
		if (_servers[i]->port == _servers[_servers.size() - 1]->port)
			return ("each server must have a different port.");
	}
	NOCLASSLOGPRINT(INFO, ("Server::parseServerBlock() : SERVER CREATED PORT(" + std::to_string(port) + "), NAME(" + serverName + "), ERROR_PATH(" + error + ")"));
	return ("0");
}

int			Conf::findServer(std::vector<std::string> confTmp)
{
	size_t		i;
	size_t		j;
	int			bracket;
	std::string	block;

	i = 6;
	while (ft::isSpace(confTmp[0][i]) || confTmp[0][i] == '{')
		i++;
	if (confTmp[0][i] != '\0' || confTmp[0].find('{') == std::string::npos)
		throw (Conf::errorSyntaxException("syntax error."));
	bracket = 1;
	j = 1;
	while (bracket != 0 && j < confTmp.size())
	{
		if (confTmp[j].find('{') != std::string::npos)
			bracket++;
		if (confTmp[j].find('}') != std::string::npos)
			bracket--;
		if (ft::countElem(confTmp[j], "{") > 1 || ft::countElem(confTmp[j], "}") > 1)
			throw (Conf::errorSyntaxException("syntax error."));
		if (bracket != 0)
			block += '\t' + confTmp[j] + '\n';
		j++;
	}
	i = 0;
	while (ft::isSpace(confTmp[j - 1][i]) || confTmp[j - 1][i] == '}')
		i++;
	if (confTmp[j - 1][i] != '\0' || bracket != 0)
		throw (Conf::errorSyntaxException("syntax error."));
	if ((_errMsg = parseServerBlock(block)) != "0")
		return (-1);
	return (j);
}

void	Conf::parseConf(void)
{
	int							i;
	size_t						j;
	std::string					line;
	std::vector<std::string>	confTmp;

	openFile();
	confTmp = ft::split(_confFile, '\n');
	for (i = 0; i < (int)confTmp.size(); i++)
	{
		j = 0;
		while (ft::isSpace(confTmp[i][j]))
			j++;
		if (confTmp[i][j] == '\0')
		{
			confTmp.erase(confTmp.begin() + i);
			i--;
		}
		else
			confTmp[i].erase(0, j);
		if (confTmp[i][0] == '#')
		{
			confTmp.erase(confTmp.begin() + i);
			i--;
		}
	}
	for (i = 0; i < (int)confTmp.size(); i++)
	{
		j = 0;
		while (confTmp[i][j])
		{
			if (ft::isSpace(confTmp[i][j]))
				confTmp[i][j] = ' ';
			j++;
		}
	}
	i = 0;
	while (confTmp.size())
	{
		if (confTmp[0].compare(0, 6, "server") != 0)
			throw (Conf::errorSyntaxException("parameter must be a server."));
		i = findServer(confTmp);
		if (i == -1)
			throw (Conf::errorSyntaxException(_errMsg));
		confTmp.erase(confTmp.begin(), confTmp.begin() + i);
	}
}

/* **************************************************** */
/*                     EXCEPTIONS     		            */
/* **************************************************** */

std::vector<Server*>	Conf::getServers(void)
{
	return (_servers);
}

Conf::errorSyntaxException::errorSyntaxException(std::string type)
{
	_type = type;
	_msg = "Error in .conf file: " + _type;
}

Conf::errorSyntaxException::~errorSyntaxException(void) throw() {}

const char *Conf::errorSyntaxException::what(void) const throw()
{
	return (_msg.c_str());
}
