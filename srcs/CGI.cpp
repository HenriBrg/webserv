# include "../inc/Webserv.hpp"

void showCGIEnv(std::map<std::string, std::string> & envmap) {

    std::cout << std::endl << std::endl;
    std::cout << YELLOW <<  "   CGI ENVIRONNEMENT VARIABLES -----------" << END << std::endl << std::endl;
    std::map<std::string, std::string>::iterator it = envmap.begin();
	while (it != envmap.end()) {
		if (!it->second.empty()) std::cout << "      " << it->first << "=" << it->second << std::endl;
        else std::cout << "      " << it->first << "=(empty value)" << std::endl;
		++it;
	}
    std::cout << std::endl << YELLOW << "    ----------------------------- END " << END << std::endl << std::endl;
    std::cout << std::endl << std::endl;

}

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
    envmap["PATH_TRANSLATED"]     = ft::getcwdString() + req->reqLocation->root.substr(1, req->reqLocation->root.size()) + req->uri.substr(req->uri.find("/" + req->isolateFileName));

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
    if (req->cgiType == PHP_CGI)
        envmap["REDIRECT_STATUS"] = "200";

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

void clearCGI(char **args, char **env) {
    utils::strTabFree(args);
    utils::strTabFree(env);
    args = env = NULL;
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
    if (stat(executable.c_str(), &buffer) != 0 || !(buffer.st_mode & S_IFREG)) {
        clearCGI(args, env);
        return LOGPRINT(LOGERROR, this, ("Request::execCGI() : The CGI provided in the configuration file isn't executable"));
    }
    if ((tmpFd = open(CGI_OUTPUT_TMPFILE, O_WRONLY | O_CREAT, 0666)) == -1) {
        clearCGI(args, env);
        LOGPRINT(LOGERROR, this, ("Request::execCGI() : open(./www/tmpFile) failed - Internal Error 500"));
        return setErrorParameters(Response::ERROR, INTERNAL_ERROR_500);
    }
    NOCLASSLOGPRINT(INFO, ("Request::execCGI() : We will fork and perform the cgi, with execve() receiving args[0] = " + std::string(args[0]) + " and args[1] = " + std::string(args[1])));
    pipe(tubes);
    if (req->method == "GET")
        close(tubes[SIDE_IN]); // Si GET, on aura jamais besoin d'écrire dans le fils, donc on close maintenant
    if ((pid = fork()) == 0) {
        dup2(tubes[SIDE_OUT], STDIN);   // Pour POST uniquement de façon à ce que le CGI récupère l'informations dans son STDIN, mais requis pour GET même s'il y a pas de body (autrement le cgi_tester freeze apparement)
        dup2(tmpFd, STDOUT);            // On veut que la sortie du CGI soit dirigée vers le fichier CGI_OUTPUT_TMPFILE
        ret = execve(executable.c_str(), args, env);
        if (ret == -1)
            NOCLASSLOGPRINT(LOGERROR, ("Request::execCGI() : execve() returned -1. Error = " + std::string(strerror(errno))));
        exit(ret);
    } else {
        if (req->method == "POST") {
            write(tubes[SIDE_IN], req->_reqBody.c_str(), req->_reqBody.size());
            // TODO : check ereurs
            close(tubes[SIDE_IN]);
        }
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            LOGPRINT(INFO, this, ("Request::execCGI() : execve() with CGI has succeed and returned : " + std::to_string(ret)));
        } else {
            LOGPRINT(LOGERROR, this, ("Request::execCGI() : execve() with CGI has failed an return -1 - Internal Error 500"));
            setErrorParameters(Response::ERROR, INTERNAL_ERROR_500);
        }
        close(tubes[SIDE_OUT]);
        close(tmpFd);
    }
    _didCGIPassed = true;
    handleCGIOutput(req->cgiType);
    LOGPRINT(INFO, this, ("Request::execCGI() : END of execCGI(). _resBody.size() = " + std::to_string(_resBody.size())));
    clearCGI(args, env);
}

void Response::handleCGIOutput(int cgiType) {

    std::ifstream tmpFile(CGI_OUTPUT_TMPFILE);
    std::string buffer((std::istreambuf_iterator<char>(tmpFile)), std::istreambuf_iterator<char>());

    _cgiOutputBody = buffer;
    if (_cgiOutputBody.find("\r\n\r\n") == std::string::npos) {
        LOGPRINT(LOGERROR, this, ("Response::handleCGIOutput() : CGI (type : " + std::to_string(cgiType) + ") output doesn't contain <CR><LR><CR><LR> pattern. Invalid CGI. Internal Error"));
        return setErrorParameters(Response::ERROR, INTERNAL_ERROR_500);
    }
    parseCGIHeadersOutput(cgiType, buffer);
    remove(CGI_OUTPUT_TMPFILE);
    buffer.clear();
    _cgiOutputBody.clear();

}

void Response::parseCGIHeadersOutput(int cgiType, std::string & buffer) {

    size_t pos;
    size_t endLine;
    std::string key;
    std::string value;
    std::string headersSection;

    if (cgiType == PHP_CGI)
        _statusCode = OK_200;
    headersSection = buffer.substr(0, buffer.find("\r\n\r\n" + 1));
    if ((pos = headersSection.find("Status")) != std::string::npos) {
        pos += 8;
        endLine = headersSection.find("\r", pos);
        if (endLine == std::string::npos) endLine = headersSection.find("\n", pos);
        _statusCode = std::stoi(headersSection.substr(pos, endLine));
    }
    pos = headersSection.find("Content-Type");
    if (pos == std::string::npos) pos = headersSection.find("Content-type");
    if (pos != std::string::npos) {
        pos += 14;
        endLine = headersSection.find("\r", pos);
        if (endLine == std::string::npos) endLine = headersSection.find("\n", pos);
        contentType[0] = headersSection.substr(pos, endLine);
    }
    pos = endLine = 0;
    pos = buffer.find("\r\n\r\n") + 4;
    if (pos == std::string::npos) return NOCLASSLOGPRINT(LOGERROR, "Response::parseCGIHeadersOutput: Invalid CGI Output, not <CR><LF><CR><LF> present to separate headers from body");
    _resBody = buffer.substr(pos);
    _resFile.clear();

}

int Response::getCGIType(Request * req) {
    if (!req->reqLocation->ext.empty() && !req->reqLocation->cgi.empty() && req->uri.find(req->reqLocation->ext) != std::string::npos)
        return (TESTER_CGI);
    else if (!req->reqLocation->php.empty() && utils::isExtension(req->file, ".php"))
        return (PHP_CGI);
    else
        return (NO_CGI);
}