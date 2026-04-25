*This project has been created as part of the 42 curriculum by mausperg, mandre, tschulle*

# webserv

## Desciption
This project is about creating a non-blocking HTTP/1.1 web server written in C++. Besides that, the goal was to understand webservers like nginx and the Hypertext Transfer Protocol (HTTP). This is implemented with function calls like epoll, socket, accept, send, recv and many other. The server is able to serve a fully static website, the client can upload files and you can use CGI with a PHP or Python backend.
This webserver is designed for linux only, as it uses the linux exclusive function epoll.

## Instruction
Clone the repository from vogshpere.

### Build
Compile the binary using:
```
$ make
```
### Run
Execute the binary from the root of this repository:
```
$ ./webserv <config>
```
### Connect 
You can now access the webserver with your browser at < address > < port > as specified in the config file.
Alternatively you can send requests with telnet using correct HTTP syntax with the methods GET, POST, DELETE and versions HTTP/1.0 or HTTP/1.1, where HTTP/1.1 requires a host header field. Example:
```
GET / HTTP/1.0
```

### Config
In the root of this repository there is a webserv-configuration.md, which explains the use of config files. A very simple sample config file could look like this:
```
http{
	server{
		listen 0.0.0.0:8085;
		autoindex off;
		root /var/www/html/;
		index index.html;
    }
}
```

### Clean

To clean the object files use:
```
$ make clean
```
To clean the object files and the executable bindary use:
```
$ make fclean
```

## Resources

### Links
https://www.rfc-editor.org/rfc/inline-errata/rfc1945.html <br>
https://www.tutorialspoint.com/http/http_requests.htm <br>
https://www.youtube.com/watch?v=7GBlCinu9yg <br>
https://beej.us/guide/bgnet0/ <br>
https://www.boot.dev/lessons/cf7815cb-5692-4011-a1ab-da592cccb9e8 <br>
https://www.youtube.com/watch?v=XXfdzwEsxFk <br>
https://www.geeksforgeeks.org/c/socket-programming-cc/ <br>
https://www.linuxtoday.com/blog/multiplexed-i0-with-poll/ <br>
https://www.lowlevel.academy <br>
https://datatracker.ietf.org/doc/html/rfc2616#page-17 <br>
https://www.boot.dev/lessons/c538bd8e-3663-42e2-8321-346368dc28d6 <br>
https://www.geeksforgeeks.org/cpp/vector-data-function-in-c-stl/ <br>
https://www.geeksforgeeks.org/cpp/map-associative-containers-the-c-standard-template-library-stl/ <br>
https://nginx.org/en/docs/http/ngx_http_core_module.html <br>
https://docs.nginx.com/nginx/admin-guide/web-server/web-server/ <br>
https://nginx.org/en/docs/http/ngx_http_index_module.html <br>
https://nginx.org/en/docs/http/ngx_http_fastcgi_module.html <br>
https://suchprogramming.com/epoll-in-3-easy-steps/ <br>
https://de.wikipedia.org/wiki/Heap_(Datenstruktur)#Heap-Bedingung <br>
https://medium.com/@RobuRishabh/heap-data-structure-and-priority-queue-in-c-d2fe7a569c86 <br>
https://en.cppreference.com/w/cpp/utility/functional/greater.html <br>
https://reqbin.com/req/php/tlotpetw/send-cookies-example <br>
https://developer.mozilla.org/en-US/docs/Web/HTTP <br>


### AI Usage
AI was used to deepen understanding about the specifics of the HTTP and understand differences between version 1.0 and 1.1.
AI was used for reviewing some parts of our code.