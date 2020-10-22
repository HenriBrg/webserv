
// https://computer.howstuffworks.com/cgi3.htm
// https://www.nginx.com/resources/wiki/start/topics/examples/full/
// The line "Content-type: text/html\n\n" is special piece of text that must be the first thing sent to the browser by any CGI script

/*

# CONTENT_TYPE    : The data type of the content. Used when the client is sending attached content to the server. For example file upload, etc.
# CONTENT_LENGTH  : The length of the query information. It's available only for POST requests
# HTTP_COOKIE     : Returns the set cookies in the form of key & value pair.
# HTTP_USER_AGENT : The User-Agent request-header field contains information about the user agent originating the request. Its name of the web browser.
# PATH_INFO       : The path for the CGI script.
# QUERY_STRING    : The URL-encoded information that is sent with GET method request.
# REMOTE_ADDR     : The IP address of the remote host making the request. This can be useful for logging or for authentication purpose.
# REMOTE_HOST     : The fully qualified name of the host making the request. If this information is not available then REMOTE_ADDR can be used to get IR address.
# REQUEST_METHOD  : The method used to make the request. The most common methods are GET and POST.
# SCRIPT_FILENAME : The full path to the CGI script.
# SCRIPT_NAME     : The name of the CGI script.
# SERVER_NAME     : The server's hostname or IP Address.
# SERVER_SOFTWARE : The name and version of the software the server is running.

*/