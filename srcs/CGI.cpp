# include "../inc/Webserv.hpp"


void showCGIEnv(std::map<std::string, std::string> & envmap) {

    std::cout << std::endl << std::endl;
    std::cout << YELLOW <<  "   CGI ENVIRONNEMENT VARIABLES -----------" << END << std::endl << std::endl;

    std::map<std::string, std::string>::iterator it = envmap.begin();
	while (it != envmap.end()) {
		if (!it->second.empty())
			std::cout << "      " << it->first << " = " << it->second << std::endl;
        else
			std::cout << "      " << it->first << " = (empty value) " << std::endl;
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
    // getcwd + root (sans le .) + uri
    envmap["PATH_TRANSLATED"]     = ft::getcwdString() + req->reqLocation->root.substr(1, req->reqLocation->root.size()) + req->uri;
    

    envmap["CONTENT_LENGTH"]      = std::to_string(req->_reqBody.size()); // The length of the query information. It's available only for POST requests
	envmap["QUERY_STRING"]        = req->uriQueries.empty() ? "" : req->uriQueries; // The URL-encoded information that is sent with GET method request.
	if (!req->contentType.empty())
        envmap["CONTENT_TYPE"]    = req->contentType; // type MIME des données véhiculées dans la requête
    envmap["SCRIPT_NAME"]         = getCGIType(req) == TESTER_CGI ? req->reqLocation->cgi : req->reqLocation->php; // Le chemin virtuel vers le script étant exécuté. Exemple : « /cgi-bin/script.cgi »
	envmap["SERVER_NAME"]         = "127.0.0.1"; // The server's hostname or IP Address.
	envmap["SERVER_PORT"]         = std::to_string(req->client->server->port); // Port du serveur

    if (!req->authorization.empty()) {
		pos = req->authorization.find(" ");
        if (pos != std::string::npos) {
            envmap["AUTH_TYPE"] = req->authorization.substr(0, pos); // Méthode : souvent "BASIC"
            envmap["REMOTE_USER"] = req->authorization.substr(pos + 1); // TODO : décoder ou non ?idTODO : décoder ou non ?
        }
	}

    // if (getCGIType(req) == PHP_CGI)
    //     envmap["REDIRECT_STATUS"] = "200"; // CG Repo Centdix required for php I guess


    /* 2) REQUEST HEADERS PASSED TO CGI */
    /* Toutes les variables qui sont envoyées par le client sont aussi passées au script CGI, après que le serveur a rajouté le préfixe « HTTP_ » */
    
    hdmap = req->mapReqHeaders();
	std::map<std::string, std::string>::iterator it = hdmap.begin();
	while (it != hdmap.end()) {
		if (!it->second.empty() && it->second != "-1")
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
		it++;
	}
    env[i] = 0;

    return env;

}

// http://www.zeitoun.net/articles/communication-par-tuyau/start
// NOCLASSLOGPRINT(DEBUG, "DEBUG 2 - executable = " + executable + " and file = " + req->file);
// NOCLASSLOGPRINT(DEBUG, ("DEBUG 3 ---> 0 = " + std::string(args[0]) + " 1 = " + std::string(args[1])));

void Response::execCGI(Request * req) {

    int     ret = -1;
    int     tmpFd = -1;
    char    **args = NULL;
    char    **env = NULL;
    int     tubes[2];
    int     status;
	struct stat	buffer;
    std::string executable;
    pid_t pid;
    
    executable.clear();
    if (req->cgiType == TESTER_CGI)
        executable = req->reqLocation->cgi;
    else if (req->cgiType == PHP_CGI)
        executable = req->reqLocation->php;
    else return LOGPRINT(LOGERROR, this, ("Request::execCGI() : Internal Error - If we reach execCGI(), the cgi should be TESTER_CGI or PHP_CGI"));

    env = buildCGIEnv(req);

    args = (char **)(malloc(sizeof(char*) * 3));
    args[0] = strdup(executable.c_str());
    args[1] = strdup(req->file.c_str());
    args[2] = 0;

    if (stat(executable.c_str(), &buffer) != 0 || !(buffer.st_mode & S_IFREG))
        return LOGPRINT(LOGERROR, this, ("Request::execCGI() : The CGI provided in the configuration file isn't executable"));
    if ((tmpFd = open(CGI_OUTPUT_TMPFILE, O_WRONLY | O_CREAT, 0666)) == -1) {
        LOGPRINT(LOGERROR, this, ("Request::execCGI() : open(./www/tmpFile) failed - Internal Error 500"));
        return setErrorParameters(req, Response::ERROR, INTERNAL_ERROR_500);
    }
    LOGPRINT(INFO, this, ("Request::execCGI() : We will fork and perform the cgi, with execve() receiving args[0] = " + std::string(args[0]) + " and args[1] = " + std::string(args[1])));

    pipe(tubes);
    // We write BODY in pipe[1] so that the cgi process can read that body in its pipe[0], and we close it juste after
    if (req->method == "POST") write(tubes[SIDE_IN], req->_reqBody.c_str(), req->_reqBody.size());
    NOCLASSLOGPRINT(DEBUG, "DEBUG BEFORE FORK - 1");
    if ((pid = fork()) == 0) {
        close(tubes[SIDE_IN]);
        // dup2(tubes[SIDE_OUT], STDIN);
        dup2(tmpFd, STDOUT);            // On veut que la sortie du CGI soit dirigée vers le fichier CGI_OUTPUT_TMPFILE
        ret = execve(executable.c_str(), args, env);
        exit(ret);

    } else {
        waitpid(pid, &status, 0);
        NOCLASSLOGPRINT(DEBUG, "DEBUG AFTER WAITPID - 2");
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            LOGPRINT(INFO, this, ("Request::execCGI() : execve() with CGI has succeed and returned : " + std::to_string(ret)));
        } else {
            LOGPRINT(LOGERROR, this, ("Request::execCGI() : execve() with CGI has return -1 (failed) - Internal Error 500"));
            setErrorParameters(req, Response::ERROR, INTERNAL_ERROR_500);
        }
        close(tubes[SIDE_OUT]);
        close(tubes[SIDE_IN]);
        close(tmpFd);

    }
    _didCGIPassed = true;
    utils::strTabFree(args);
    utils::strTabFree(env);
    args = env = NULL;

}


void Response::parseCGIHeadersOutput(Request * req) {

    std::string line;
    std::ifstream outputFile(CGI_OUTPUT_TMPFILE);
    std::vector<std::string> hd;

    hd.clear();
    if (!outputFile.is_open())
        return LOGPRINT(LOGERROR, this, ("Response::parseCGIOutput() : CGI Output file is closed"));
    if (req->cgiType == PHP_CGI) {

        // TODO : tester le php-cgi hors de webserv puis revenir ici
        _statusCode = OK_200;
        contentType[0] = "text/html";

    } else if (req->cgiType == TESTER_CGI) {

        // TODO : on est censé récupérer Status et Content-Type sur l'output, mais Status en premier, autrement Erreur 500 --> comportement à confirmer

        getline(outputFile, line);
        if (line.find("Status") != std::string::npos) {
            hd = ft::split(line, ' ');
            _statusCode = std::stoi(hd[1]);
            hd.clear();
        }
        getline(outputFile, line);
        if (line.find("Content-Type:") != std::string::npos) {
            hd = ft::split(line, ':');
            contentType[0] = ft::trim(hd[1]);
            hd.clear();
        }

		if (_statusCode < 200 || _statusCode > 500)
            _statusCode = 500; // Internal Error, the cgi output should always give us a status code, isn't it ? --> TODO : checker ça

    }
}