#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <string>
# include <map>
# include <unistd.h>
# include <deque>

# include "Server.hpp"

# ifndef __APPLE__
    # define PLATFORM "Linux"
# else
    # define PLATFORM "Macos"
# endif

# define OK_200							    200
# define CREATED_201					    201
# define ACCEPTED_202					    202
# define NO_CONTENT_204				    	204
# define USE_PROXY_305                      305	
# define BAD_REQUEST_400			    	400
# define UNAUTHORIZED_401			    	401
# define FORBIDDEN_403  			    	403
# define NOT_FOUND_404 					    404
# define METHOD_NOT_ALLOWED_405			    405
# define NOT_ACCEPTABLE_406                 406
# define CONFLICT_409			            409
# define REQUEST_ENTITY_TOO_LARGE_413	    413
# define REQUEST_URI_TOO_LONG_414           414
# define SSL_CERTIFICATE_ERROR_495          495
# define INTERNAL_ERROR_500				    500
# define NOT_IMPLEMENTED_501			    501
# define SERVICE_UNAVAILABLE_503		    503
# define HTTP_VERSION_NOT_SUPPORTED_505		505


# define CGI_OUTPUT_TMPFILE "./www/tmpFile"
# define NO_CGI 0
# define TESTER_CGI 1
# define PHP_CGI 2

# define SIDE_OUT 0
# define SIDE_IN 1

# define STDIN 0
# define STDOUT 1

# define CREATE 1
# define UPDATE 2

// In HTTP, content negotiation is the mechanism that is used for serving different representations of a resource at the same URI,
// so that the user agent can specify which is best suited for the user (for example, which language of a
// document, which image format, or which content encoding).


class Client;
class Server;
class Location;


class Response {

	friend class Client;
    
    public:

        Client      *resClient;
        std::string _errorFileName;
        std::string _resFile;
        std::string formatedResponse;
        int         _bytesSent;

        bool         _didCGIPassed;
        std::string _cgiOutputBody;


        int _sendStatus;
        enum resStatus {
            PREPARE,
            SENDING,
            DONE,
            ERROR
        };

        /* GLOBAL */

        Response();
        Response(Client *c);
        virtual ~Response();
        void    reset();

        /* Building response functions */
        void    control(Request * req, Server * serv);
        void    callMethod(Request * req);
        void    setHeaders(Request * req);
        void    handleAutoIndex(Request * req);
        void    setBody(const Server *server);
        void    setBodyHeaders(void);
        void    format(void);
        void    intoChunk();

        /* CONTROL */
        void    versionControl(Request *req);
        void    resourceControl(Request * req);
        void    methodControl(Request * req, Server * serv);
        void    authControl(Request * req);
        void    reqHeadersControl(Request * req);

        /* LOGS AND ERRORS */
        std::string const logInfo(void);
        void    errorHandler(void);
        
        /* METHODS */
        void    getReq(Request * req);
        void    headReq(Request * req);
        void    putReq(Request * req);
        void    postReq(Request * req);
        void    patchReq(Request * req);
        void    deleteReq(Request * req);
        void    (Response::*_methodFctPtr)(Request * req);

        /* HELPER METHODS FUNCTIONS */
        void deleteDir(std::string directory);

        /* CGI */

        void    execCGI(Request * req);
        char    **buildCGIEnv(Request * req);
        int     getCGIType(Request * req);
        void    handleCGIOutput(int cgiType);
        void    parseCGIOutput(int cgiType, std::string & buffer);


        /* BODY */
        void addBody(Request * req);

        /* UTILS METHODS */

        void negotiateAcceptLanguage(Request * req);
        void negotiateAcceptCharset(Request * req);
        void showFullHeadersRes(void);
        void showRes(void);


        /* ERRORS */
        void setRefusedClient(const Server *serv);
        void setErrorParameters(int sendStatus, int code);
        void replaceErrorCode(const Server *server);
        
        /* MEMBERS */

        // https://web.maths.unsw.edu.au/~lafaye/CCM/internet/http.htm

        char *_resBody;
        
        /* 1) Ligne de statut */
        
        std::string httpVersion;
        int         _statusCode;
        std::string reason;


        /* 2) Headers */

        // Cet entête doit être envoyée si le serveur répond avec un statut 405 Method Not Allowed pour indiquer quelles méthodes peuvent être utilisées pour la requête.
        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Allow
        // Allow: GET, POST, HEAD
        std::string allow;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Content-Language
        std::map<int, std::string> contentLanguage;
        bool _isLanguageNegociated;

        int contentLength;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Content-Location
        std::map<int, std::string> contentLocation;
        
        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Content-Type
        // NB : Dans les requêtes, (telles que POST ou PUT), le client indique au serveur quel type de données a réellement été envoyé.
        std::map<int, std::string> contentType;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Last-Modified
        // L'entête HTTP de réponse Last-Modified contient la date et l'heure à laquelle le serveur
        // d'origine pense que la ressource a été modifiée pour la dernière fois. Il est utilisé comme un validateur
        // pour déterminer si une ressource reçue et une stockée sont les mêmes.
        std::string lastModified;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Location
        // Uniquement utilisé lors d'une réponse d'état 3XX
        std::string location;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Date
        std::string date;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Retry-After
        // Use : 503 / 429 / 301
        int retryAfter;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Serveur
        std::string server;

        // Specify the response format
        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Transfer-Encoding
        std::map<int, std::string> transfertEncoding;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/WWW-Authenticate
        // Use : 401 Unauthorized
        std::string wwwAuthenticate;

    private:



};

#endif