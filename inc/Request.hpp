#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>
# include <unistd.h>
# include <sys/stat.h> 


# include "Server.hpp"
# include "Location.hpp"

class Client;

/* EXEMPLES DE REQUÊTES GET/POST */

// https://www.w3schools.com/tags/ref_httpmethods.asp

// PUT VS POST  : https://blog.engineering.publicissapient.fr/2014/03/17/post-vs-put-la-confusion/
// By this argument, PUT is for creating when you know the URL of the thing you will create. POST can be used to create when you know the URL of the "factory" or manager for the category of things you want to create.

// so:
// POST /expense-report
// or:
// PUT  /expense-report/10929

// GET /hello.htm HTTP/1.1
// User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
// Host: www.tutorialspoint.com
// Accept-Language: en-us
// Accept-Encoding: gzip, deflate
// Connection: Keep-Alive


// POST /cgi-bin/process.cgi HTTP/1.1
// User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
// Host: www.tutorialspoint.com
// Content-Type: application/x-www-form-urlencoded
// Content-Length: length
// Accept-Language: en-us
// Accept-Encoding: gzip, deflate
// Connection: Keep-Alive

class Request {
    
    public:


        /* METHODS */

        Request();
        virtual ~Request();

        void parse(std::vector<Location*> locations);
        void showReq(void);
        void showFullHeadersReq(void);
        void showBody();

        void    reset(void);
        void	readline(std::string & b, std::string & line);
        int     parseRequestLine();
        void    parseFile(std::vector<Location*> locations);
        void    parseQuery();
        void    parseUriQueries();
        void    assignLocation(std::vector<Location*> locations);
        void    parseHeaders();
        void    fillHeader(std::string const key, std::string const value);
        
        void    checkBody();
        void    parseChunkedBody();
        void    parseSingleBody();


        
        std::string const logInfo(void);

        /* MEMBERS */

        Client          *client;
        std::string     reqBuf;
        std::string     _reqBody;
        
        int             bodyLength;
        Location        *reqLocation;
        std::string     file;


        // Read this if you're not familiar with HTTP Requests
        // (100% Best Documentation) => https://developer.mozilla.org/fr/docs/Web/HTTP/Headers 
        // https://web.maths.unsw.edu.au/~lafaye/CCM/internet/http.htm
        // https://www.tutorialspoint.com/http/http_requests.htm and the 4 pages following this one
        // https://www.ntu.edu.sg/home/ehchua/programming/webprogramming/HTTP_Basics.html (middle of page - fields explained)

        /* 1) Request Line : Example : HEAD / HTTP/1.0 */

        std::string method;
        std::string uri;
        std::string httpVersion;
        // Everything after '?' inside URI
        std::string uriQueries;


        /* Not mandatory headers, but here to improve global understanding */

        /* "Accept" : "text/html, application/xhtml+xml, application/xml;q=0.9, image/webp"
        The Accept header lists the MIME types of media resources that the agent is willing to process. */



        /* 2) Request Headers Fields - Ordered Alphabetically */

        // Accept-Charset: Charset-1, Charset-2, ... ===> { 1: "utf-8", 2: "iso-8859-1;q=0.5" }
        // For character set negotiation, the client can use this header to tell the server which character sets it can handle or it prefers
        // NB : Si on a pas la ressource dans le format souhaité, en théorie on retourne 406 mais en pratique on ignore pour qu'une réponse, bien qu'imparfaite, soit quand même retournée
        std::map<int, std::string> acceptCharset;

        // Accept-Language: language-1, language-2, ...
        // Accept-Language: da, en-gb;q=0.8, en;q=0.7
        // The client can use the Accept-Language header to tell the server what languages it can handle or it prefers.
        // If the server has multiple versions of the requested document (e.g., in English, Chinese, French), it can check this header to decide which version to return. This process is called language negotiation
        // The optional qvalue represents an acceptable quality level for non preferred languages
        std::map<int, std::string> 	acceptLanguage;

        // Authorization : credentials
        // Authorization: Basic YWxhZGRpbjpvcGVuc2VzYW1l
        // Used by the client to supply its credential (username/password) to access protected resources.
        std::string authorization;


        // UPDT : à priori Content-Language est uniquement utile en response, à confirmer
        // Content-Language: mi, en
        // Describes the natural language(s) of the intended audience for the enclosed entity
        // Define which language the document is intended to.
        std::map<int, std::string> contentLanguage;


        // Content-Length: 3495
        // The Content-Length entity-header field indicates the size of the entity-body, in decimal number of OCTETs,
        // sent to the recipient or, in the case of the HEAD method, the size of the entity-body that would have been sent, had the request been a GET. 
        // Used by POST request, to inform the server the length of the request body
        int contentLength;


        // Il semble que ce Content-Location soit utilisé en requête mais aussi en réponse ... à voir
         
        // The principal use is to indicate the URL of a resource transmitted as the result of content negotiation.
        // The Content-Location header indicates an alternate location for the returned data. The principal use is to indicate the URL of a resource transmitted as the result of content negotiation.
        // Location and Content-Location are different.
        // Location indicates the URL of a redirect, while Content-Location indicates the direct URL to use to access the resource, without further content negotiation in the future
        std::string contentLocation;

        // The Content-Type entity-header field indicates the media type of the entity-body sent to the recipient or,
        // in the case of the HEAD method, the media type that would have been sent, had the request been a GET.
        // Dans les requêtes, (telles que POST ou PUT), le client indique au serveur quel type de données a réellement été envoyé.
        // Dans les réponses, un en-tête Content-Type indique au client le type de contenu réellement renvoyé.
        // Content-Type: text/html; charset=ISO-8859-4
        std::string contentType;

        // All HTTP date/timestamps MUST be represented in Greenwich Mean Time (GMT), without exception
        std::string date;

        // The Host request header specifies the host and port number of the server to which the request is being sent.
        // If no port is included, the default port for the service requested (e.g., 443 for an HTTPS URL, and 80 for an HTTP URL) is implied.
        // A Host header field must be sent in all HTTP/1.1 request messages.
        // A 400 (Bad Request) status code may be sent to any HTTP/1.1 request message that lacks a Host header field or that contains more than one.
        std::string host;

        // The Referer request header contains the address of the previous web page from which a link to the currently requested page was followed.
        // The Referer header allows servers to identify where people are visiting them from and may use that data for analytics, logging, or optimized caching, for example.
        std::string referer;

        // The User-Agent request header is a characteristic string that lets servers and network peers identify the application, operating system, vendor, and/or version of the requesting user agent.
        std::string userAgent;

        /* 3) Request Body */

        // If we receive that body : "a=1&b=2"
        // We will parse it as : {1: {"a": "1"}, 2: {"b": "2"}}
        // std::map<int, std::pair<std::string, std::string> > uriQueries;

        // The Transfer-Encoding header specifies the form of encoding used to safely transfer the payload body to the user.
        // Transfer-Encoding: gzip, chunked
        std::map<int, std::string> transferEncoding;

        // https://www.hostinger.fr/tutoriels/keep-alive/
        // Keep-Alive permet au navigateur du visiteur de télécharger tout le contenu (comme JavaScript, CSS, images, vidéos, etc.) via une connexion TCP persistante au lieu de faire des demandes différentes pour chaque fichier
		std::string	keepAlive; 


        
        // Chunked : Data is sent in a series of chunks. The Content-Length header is omitted in this case and at the beginning of each chunk you need to add the length of the current chunk

    private:



};

#endif