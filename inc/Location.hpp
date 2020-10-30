#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Config.hpp"

class Location
{
		public:
		std::string		uri;
		std::string		root;
		std::string		index;
		std::string		methods;
		std::string		auth;
		std::string		cgi;
		std::string		php;
		std::string		ext;
		int				max_body;

		// Pour la phase de développement pré-parsing, on utilisera ces locations :

		// Location *newLoc1 = new Location("/", "./www", "index.html", "GET");
		// Location *newLoc2 = new Location("/", "./www", "page.html", "GET POST");

		Location(std::string _uri, \
				std::string _root, \
				std::string _index, \
				std::string _methods, \
				int			_max_body, \
				std::string _auth, \
				std::string _cgi, \
				std::string _phpcgi, \
				std::string _ext)
		{
			uri = _uri;
			root = _root;
			index = _index;
			methods = _methods;
			max_body = _max_body;
			auth = _auth;
			cgi = _cgi;
			php = _phpcgi;
			ext = _ext;
		};

		private:
			Location();
};

#endif
