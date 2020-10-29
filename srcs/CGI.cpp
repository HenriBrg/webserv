# include "../inc/Webserv.hpp"


void showCGIEnv(std::map<std::string, std::string> & envmap) {

    std::cout << std::endl << std::endl;
    std::cout << YELLOW <<  "   CGI ENVIRONNEMENT VARIABLES -----------" << END << std::endl << std::endl;

    std::map<std::string, std::string>::iterator it = envmap.begin();
	while (it != envmap.end()) {
		if (!it->second.empty())
			std::cout << "      " << it->first << " = " << it->second << std::endl;
        else
			std::cout << "      " << it->first << " = EMPTY" << std::endl;
		++it;
	}

    std::cout << std::endl << YELLOW << "    ----------------------------- END " << END << std::endl << std::endl;

    std::cout << std::endl << std::endl;

}

// https://perso.liris.cnrs.fr/lionel.medini/enseignement/M1IF03/Tutoriels/Tutoriel_CGI_SSI.pdf
// https://fr.wikipedia.org/wiki/Variables_d%27environnement_CGI
// https://computer.howstuffworks.com/cgi3.htm
// The line "Content-type: text/html\n\n" is special piece of text that must be the first thing sent to the browser by any CGI script

char ** Response::buildCGIEnv(Request * req) {

    char **env;
    size_t pos;
    std::pair<std::string, std::string> tmp;
    std::map<std::string, std::string> envmap;
    std::map<std::string, std::string> hdmap;

    /* 1) CGI INFORMATIONS */

    envmap["GATEWAY_INTERFACE"]   = "CGI/1.1";        // CGI Version
	envmap["SERVER_PROTOCOL"]     = "HTTP/1.1";       // Protocole
	envmap["SERVER_SOFTWARE"]     = "webserv";        // Serveur
	envmap["REQUEST_URI"]         = req->uri;         // URI demandée
	envmap["REQUEST_METHOD"]      = req->method;      // Type de requête
	envmap["REMOTE_ADDR"]         = req->client->ip;  // IP du client
	envmap["PATH_INFO"]           = req->uri;         // Partie de l'URI entre le nom du CGI (exclus) et le reste de l'URI (voir Wikipédia) ) - à vérifier
    // Partie de l'URI entre le nom du CGI (exclus) et le reste de l'URI (voir Wikipédia) ) - à vérifier
    envmap["PATH_TRANSLATED"]     = ft::getcwdString() + req->reqLocation->root.substr(1, req->reqLocation->root.size()) + req->uri;
    
    envmap["CONTENT_LENGTH"]      = std::to_string(req->_reqBody.size()); // The length of the query information. It's available only for POST requests
	envmap["QUERY_STRING"]        = req->uriQueries.empty() ? "" : req->uriQueries; // The URL-encoded information that is sent with GET method request.
	envmap["CONTENT_TYPE"]        = req->contentType; // type MIME des données véhiculées dans la requête
    envmap["SCRIPT_NAME"]         = getCGIType(req) == TESTER_CGI ? req->reqLocation->cgi : req->reqLocation->php; // chemin du CGI à partir de la racine du serveur HTTP
	envmap["SERVER_NAME"]         = "127.0.0.1"; // The server's hostname or IP Address.
	envmap["SERVER_PORT"]         = std::to_string(req->client->server->port); // Port du serveur

    if (!req->authorization.empty()) {
		pos = req->authorization.find(" ");
        if (pos != std::string::npos) {
            envmap["AUTH_TYPE"] = req->authorization.substr(0, pos); // Méthode : souvent "BASIC"
            envmap["REMOTE_USER"] = req->authorization.substr(pos + 1); // TODO : décoder ou non ?
        }
	}

    // if (getCGIType(req) == PHP_CGI)
    //     envmap["REDIRECT_STATUS"] = "200"; // CG Repo Centdix required for php I guess


    /* 2) REQUEST HEADERS PASSED TO CGI */
    /* Toutes les variables qui sont envoyées par le client sont aussi passées au script CGI, après que le serveur a rajouté le préfixe « HTTP_ » */
    
    hdmap = req->mapReqHeaders();
	std::map<std::string, std::string>::iterator it = hdmap.begin();
	while (it != hdmap.end()) {
		if (!it->second.empty())
			envmap["HTTP_" + it->first] = it->second;
		++it;
	}

    showCGIEnv(envmap);

    /* 3) Allocation, car il faut retourner un char ** pour matcher l'injection de **env dans execve() */

    int i = -1;
    env = (char**)malloc(sizeof(char*) * (envmap.size() + 1));
    it = envmap.begin();
    while (it != envmap.end()) {
        env[++i] = strdup((it->first + "=" + it->second).c_str());
		++it;
	}
    env[i] = 0;
    return env;

}

// http://www.zeitoun.net/articles/communication-par-tuyau/start

void Response::execCGI(Request * req) {

    int     ret;
    int     tmpFd;
    char    **args = NULL;
    char    **env = NULL;
    int     tubes[2];
	struct stat	buffer;
    std::string executable;
    
    executable.clear();
    if (getCGIType(req) == TESTER_CGI)
        executable = req->reqLocation->cgi;
    else if (getCGIType(req) == PHP_CGI)
        executable = req->reqLocation->php;
    else return LOGPRINT(LOGERROR, this, ("Request::execCGI() : Internal Error"));

    env = buildCGIEnv(req);
    args[0] = strdup(executable.c_str());
    args[1] = strdup(req->file.c_str());
    args[2] = 0;

    if (stat(executable.c_str(), &buffer) != 0 || !(buffer.st_mode & S_IFREG))
        return LOGPRINT(LOGERROR, this, ("Request::execCGI() : The CGI provided in the configuration file isn't executable"));

    LOGPRINT(INFO, this, ("Request::execCGI() : Ready to fork and execute cgi. Execve will receive args[0] = " + std::string(args[0]) + " and args[1] = " + std::string(args[1])));
	tmpFd = open("./www/tmpFile", O_WRONLY | O_CREAT, 0666);
    pipe(tubes);
    if (req->method == "POST") write(tubes[SIDE_IN], req->_reqBody.c_str(), req->_reqBody.size());
    close(tubes[SIDE_IN]);
    if ((req->client->cgipid = fork()) == 0) {

        dup2(tmpFd, STDOUT);            // On veut que la sortie du CGI aille droit dans le fichier "tmpFile"
        dup2(tubes[SIDE_OUT], STDIN);   // On veut que le CGI prenne en input ce qu'on aura au préalable écrit
        ret = execve(executable.c_str(), args, env);
        if (ret == -1) {
            LOGPRINT(LOGERROR, this, ("Request::execCGI() : syscall execve() failed - error is : " + std::string(strerror(errno))));
            exit(EXIT_FAILURE);
        }

    } else {
        waitpid(req->client->cgipid, NULL, 0);
        close(tubes[SIDE_OUT]);
        close(tmpFd);   
    }

}