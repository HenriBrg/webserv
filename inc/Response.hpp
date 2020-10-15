#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <string>
# include <map>
# include <unistd.h>

# include "Server.hpp"
# include "Location.hpp"

#define OK_200							200
#define CREATED_201						201
#define ACCEPTED_202					202
#define NO_CONTENT_204					204
#define BAD_REQUEST_400					400
#define UNAUTHORIZED_401				401
#define NOT_FOUND_404 					404
#define METHOD_NOT_ALLOWED_405			405
#define REQUEST_ENTITY_TOO_LARGE_413	413
#define INTERNAL_ERROR_500				500
#define NOT_IMPLEMENTED_501				501
#define SERVICE_UNAVAILABLE_503			503

// In HTTP, content negotiation is the mechanism that is used for serving different representations of a resource at the same URI,
// so that the user agent can specify which is best suited for the user (for example, which language of a
// document, which image format, or which content encoding).


class Client;

class Response {

	friend class Client;

    
    public:

        Client      *resClient;
        std::string finalResponse;

        int sendStatus;
        enum resStatus {
            PREPARE,
            SENDING,
            DONE,
            ERROR
        };

        /* GLOBAL */

        Response();
        virtual ~Response();
        void reset();
        void resDispatch(Request * req);

        /* CONTROL */

        void methodControl(Request * req);
        void authControl(Request * req);

        std::string const logInfo(void);
        void  errorHandler(void);
        
        /* METHODS */

        void (Response::*_methodFctPtr)(Request * req);
        void getReq(Request * req);
        void headReq(Request * req);
        void putReq(Request * req);
        void postReq(Request * req);
        void connectReq(Request * req);
        void traceReq(Request * req);
        void optionsReq(Request * req);
        void deleteReq(Request * req);
        
        /* MEMBERS */

        // https://web.maths.unsw.edu.au/~lafaye/CCM/internet/http.htm

        std::string body;

        
        /* 1) Ligne de statut */
        
        std::string httpVersion;
        int         statusCode;
        std::string reason;


        /* 2) Headers */

        // Cet entête doit être envoyée si le serveur répond avec un statut 405 Method Not Allowed pour indiquer quelles méthodes peuvent être utilisées pour la requête.
        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Allow
        // Allow: GET, POST, HEAD
        std::string allowedMethods;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Content-Language
        std::map<int, std::string> contentLanguage;

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
        std::string retryAfter;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Serveur
        std::string server;

        // Specify the response format
        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/Transfer-Encoding
        std::map<int, std::string> transfertEncoding;

        // https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/WWW-Authenticate
        // Use : 401 Unauthorized
        std::map<int, std::string> wwwAuthenticate;

    private:



};

#endif