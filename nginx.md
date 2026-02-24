
!! Subject: Directory where the requested file should be located (e.g., if URL /kapouet
is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for
/tmp/www/pouic/toto/pouet). !!


Return Specific Status Codes
Some website URIs require immediate return of a response with a specific error or redirect code, for example when a page has been moved temporarily or permanently. The easiest way to do this is to use the return directive. For example:

nginx
location /wrong/url {
    return 404;
}
The first parameter of return is a response code. The optional second parameter can be the URL of a redirect (for codes 301, 302, 303, and 307) or the text to return in the response body. For example:

nginx
location /permanently/moved/url {
    return 301 http://www.example.com/moved/here;
}

The return directive can be included in both the location and server contexts.



Allows disabling chunked transfer encoding in HTTP/1.1. It may come in handy when using a software failing to support chunked encoding despite the standard’s requirement.

Syntax:	client_body_buffer_size size;
Default:	
client_body_buffer_size 8k|16k;
Context:	http, server, location


Syntax:	client_body_timeout time;
Default:	
client_body_timeout 60s;
Context:	http, server, location
Defines a timeout for reading client request body. The timeout is set only for a period between two successive read operations, not for the transmission of the whole request body. If a client does not transmit anything within this time, the request is terminated with the 408 (Request Time-out) error.


Syntax:	autoindex on | off;
Default:	
autoindex off;
Context:	http, server, location
Enables or disables the directory listing output.


Directives
Syntax:	index file ...;
Default:	
index index.html;
Context:	http, server, location
Defines files that will be used as an index. The file name can contain variables. Files are checked in the specified order. The last element of the list can be a file with an absolute path.