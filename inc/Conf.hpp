/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Conf.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rofernan <rofernan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/23 12:49:46 by rofernan          #+#    #+#             */
/*   Updated: 2020/10/23 12:49:48 by rofernan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONF_HPP
# define CONF_HPP

# include "Webserv.hpp"
# include <exception>
# include "Server.hpp"

# define BUFFMAX 512
# define DEFAULT_CONF_PATH "conf/default.conf"


class Conf
{
	public:
	Conf(char *filename);
	~Conf();

	Conf	&operator=(const Conf&);

	void	openFile(void);
	void	parseLocation(Server *serv, std::string locs);
	void	parseServerBlock(std::string block);
	size_t	findServer(std::vector<std::string> confTmp);
	void	parseConf(void);

	class errorSyntaxException: public std::exception
	{
		public:
		errorSyntaxException(std::string type);
		virtual ~errorSyntaxException(void) throw();
		virtual const char *what() const throw();

		private:
		std::string	_type;
		std::string	_msg;
	};

	std::vector<Server*>	getServers(void);

	private:
	Conf();
	char					*_fileName;
	std::string				_confFile;
	std::vector<Server*>	_servers;
};

#endif
