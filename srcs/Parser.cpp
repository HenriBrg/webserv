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

Conf::Conf()
{
}

Conf::Conf(char *filename): _fileName(filename), _confFile("")
{
}

Conf::~Conf()
{
}

Conf	&Conf::operator=(const Conf &copy)
{
	(void)copy;
	return (*this);
}

void	Conf::openFile(void)
{
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

void	Conf::parseLocation(Server *serv, std::string locs)
{
	int			max_body = -1, beg, end;
	bool		autoindex = false;
	std::string	uri, root, index, methods, auth, cgi, phpcgi, ext;
	std::string	location;
	std::vector<std::string>	locBlock;
	std::vector<std::string>	line;

	while ((beg = locs.find("{") != std::string::npos))
	{
		// locBlock = ft::split(locs, '\n');
		// line = ft::splitWhtSp(locBlock[0]);
		// if (line.size() != 3 || line[0] != "location" || line[1][0] != '/' || line[2] != "{")
		// 	throw(Conf::errorSyntaxException("location syntax."));
		// uri = line[1];
		// line.clear();
		// locBlock.clear();
		// beg = locs.find("{");
		// std::cout << beg << std::endl;
		end = locs.find("}");
		location = locs.substr(beg, end + 1 - beg);
		// if (ft::countElem(location, "\troot ") != 1)
		// 	throw(Conf::errorSyntaxException("parameter root must be present once."));
		// if (ft::countElem(location, "\tindex ") != 1)
		// 	throw(Conf::errorSyntaxException("parameter index must be present once."));
		// if (ft::countElem(location, "\tmethods ") != 1)
		// 	throw(Conf::errorSyntaxException("parameter methods must be present once."));
		// if (ft::countElem(location, "\tauth ") > 1)
		// 	throw(Conf::errorSyntaxException("parameter auth must be present max once."));
		// std::cout << "deb: " << location << " :fin" << std::endl << std::endl;
		locBlock = ft::split(location, '\n');
		for (size_t i = 0; i < locBlock.size(); i++)
		{
			line = ft::splitWhtSp(locBlock[i]);
			if (!line.empty())
			{
				if (line[0] == "location")
				{
					if (line.size() != 3 || line[1][0] != '/' || line[2] != "{")
						throw(Conf::errorSyntaxException("location syntax."));
					uri = line[1];
				}
				if (line[0] == "root")
				{
					if (line.size() != 2)
						throw(Conf::errorSyntaxException("root syntax."));
					DIR *dir;
					if (!(dir = opendir(line[1].c_str())))
					{
						closedir(dir);
						throw(Conf::errorSyntaxException("cannot find root path."));
					}
					closedir(dir);
					root = line[1];
				}
				if (line[0] == "index")
				{
					if (line.size() != 2)
						throw(Conf::errorSyntaxException("index syntax."));
					index = line[1];
				}
				if (line[0] == "method")
				{
					if (line.size() != 2)
						throw(Conf::errorSyntaxException("method syntax."));
					methods = line[1];
				}
				if (line[0] == "auth")
				{
					if (line.size() != 2)
						throw(Conf::errorSyntaxException("auth syntax."));
					auth = line[1];
				}
				if (line[0] == "cgi")
				{
					if (line.size() != 2)
						throw(Conf::errorSyntaxException("cgi syntax."));
					cgi = line[1];
				}
				if (line[0] == "php")
				{
					if (line.size() != 2)
						throw(Conf::errorSyntaxException("php syntax."));
					phpcgi = line[1];
				}
				if (line[0] == "ext")
				{
					if (line.size() != 2)
						throw(Conf::errorSyntaxException("ext syntax."));
					ext = line[1];
				}
				if (line[0] == "max_body")
				{
					if (line.size() != 2 || !ft::isNumber(line[1]))
						throw(Conf::errorSyntaxException("max_body syntax."));
					max_body = std::stoi(line[1]);
				}
				if (line[0] == "autoindex")
				{
					if (line.size() != 2 || (line[1] != "on" && line[1] != "off"))
						throw(Conf::errorSyntaxException("autoindex syntax."));
					if (line[1] == "on")
						autoindex = true;
				}
			}
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
}

void	Conf::parseServerBlock(std::string block)
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

	// std::cout << block << std::endl;
	if ((nbLoc = ft::countElem(block, "\tlocation ")) < 1)
		throw (Conf::errorSyntaxException("parameter location must be present at least once."));
	idx = block.find("\tlocation");
	servParams = block.substr(0, idx);
	if (ft::countElem(servParams, "\tlisten ") != 1)
		throw(Conf::errorSyntaxException("parameter listen must be present once."));
	if (ft::countElem(servParams, "\tserver_name ") > 1)
		throw(Conf::errorSyntaxException("parameter server_name must be present max once."));
	if (ft::countElem(servParams, "\terror ") != 1)
		throw(Conf::errorSyntaxException("there must be one errors path."));
	// std::cout << servParams << std::endl << std::endl;
	
	serverBlock = ft::split(servParams, '\n');
	for (size_t i = 0; i < serverBlock.size(); i++)
	{
		// std::cout << serverBlock[i] << std::endl;
		line = ft::splitWhtSp(serverBlock[i]);
		if (line[0] == "listen")
		{
			if (line.size() != 2 || !ft::isNumber(line[1]))
				throw(Conf::errorSyntaxException("port syntax."));
			port = std::stoi(line[1]);
			if (port <= 1024)
				throw(Conf::errorSyntaxException("port must be over 1024."));
		}
		else if (line[0] == "server_name")
		{
			if (line.size() != 2)
				throw(Conf::errorSyntaxException("server name syntax."));
			serverName = line[1];
		}
		else if (line[0] == "error")
		{
			DIR *dir = NULL;
			if (line.size() != 2 || !(dir = opendir(line[1].c_str())))
			{
				closedir(dir);
				throw(Conf::errorSyntaxException("cannot find errors path."));
			}
			closedir(dir);
			error = line[1];
		}
		else
			throw(Conf::errorSyntaxException("wrong server parameter."));
		line.clear();
	}
	serverBlock.clear();
	servParams = block.substr(idx, block.size() - idx - 1);
	if (serverName == "")
		serverName = "webserv" + std::to_string(nameNb++);
	Server *serv = new Server(port, serverName, error);
	serverBlock = ft::split(servParams, '\n');
	// std::cout << servParams << std::endl << std::endl;
	// for (int i = 0; i < nbLoc; i++)
	// 	parseLocation(serv, serverBlock);
	NOCLASSLOGPRINT(INFO, ("Conf::parseServerBlock() : FOR SERVER PORT " + std::to_string(port)));
	parseLocation(serv, servParams);
	try
	{
        serv->start();
	}
	catch (std::exception &e)
	{
        std::cerr << e.what() << std::endl;
    }
	_servers.push_back(serv);
	for (size_t i = 0; i < _servers.size() - 1; i++)
		if (_servers[i]->port == _servers[_servers.size() - 1]->port)
			throw(Conf::errorSyntaxException("each server must have a different port."));
	NOCLASSLOGPRINT(INFO, ("Server::parseServerBlock() : SERVER CREATED PORT(" + std::to_string(port) + "), NAME(" + serverName + "), ERROR_PATH(" + error + ")"));
}

size_t	Conf::findServer(std::vector<std::string> confTmp)
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
			throw (Conf::errorSyntaxException("line: " + confTmp[j - 1] + " : syntax error."));
		if (bracket != 0)
			block += '\t' + confTmp[j] + '\n';
		j++;
	}
	i = 0;
	while (ft::isSpace(confTmp[j - 1][i]) || confTmp[j - 1][i] == '}')
		i++;
	if (confTmp[j - 1][i] != '\0' || bracket != 0)
		throw (Conf::errorSyntaxException("line: " + confTmp[j - 1] + " : syntax error."));
	parseServerBlock(block);
	return (j);
}

void	Conf::parseConf(void)
{
	size_t						i;
	size_t						j;
	std::string					line;
	std::vector<std::string>	confTmp;

	openFile();
	confTmp = ft::split(_confFile, '\n');
	for (i = 0; i < confTmp.size(); i++)
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
	}
	for (size_t i = 0; i < confTmp.size(); i++)
	{
		j = 0;
		while (confTmp[i][j])
		{
			if (ft::isSpace(confTmp[i][j]))
				confTmp[i][j] = ' ';
			j++;
		}
		// std::cout << confTmp[i] << std::endl;
	}
	i = 0;
	while (confTmp.size())
	{
		if (confTmp[0].compare(0, 6, "server") != 0)
			throw (Conf::errorSyntaxException("parameter must be a server."));
		i = findServer(confTmp);
		confTmp.erase(confTmp.begin(), confTmp.begin() + i);
	}
	// for (int i = 0; i < _servers.size(); i++)
	// {
	// 	std::cout << _servers[i]->name << " " << _servers[i]->port << " " << _servers[i]->error << std::endl;
	// 	for (int j = 0; j < _servers[i]->locations.size(); j++)
	// 	{
	// 		std::cout << "\nlocation" << j << std::endl;
	// 		std::cout << _servers[i]->locations[j]->uri << std::endl;
	// 		std::cout << _servers[i]->locations[j]->root << std::endl;
	// 		std::cout << _servers[i]->locations[j]->index << std::endl;
	// 		std::cout << _servers[i]->locations[j]->methods << std::endl;
	// 		std::cout << _servers[i]->locations[j]->max_body << std::endl;
	// 		std::cout << _servers[i]->locations[j]->auth << std::endl;
	// 		std::cout << _servers[i]->locations[j]->cgi << std::endl;
	// 		std::cout << _servers[i]->locations[j]->php << std::endl;
	// 		std::cout << _servers[i]->locations[j]->ext << std::endl;
	// 	}
	// 	std::cout << std::endl;
	// }
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
