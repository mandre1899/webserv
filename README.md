*This project has been created as part of the 42 curriculum by mausperg, mandre, tschulle*

# webserv

## Desciption
This project is about creating a non-blocking HTTP/1.1 web server written in C++. Besides that, he goal was to understand webservers like nginx and the Hyptext Transfer Protocol (HTTP). This is implemented with function calls like epoll, socket, accept, send, recv and many other. The server is able to serve a fully static website, the client can upload files and there is a CGI backend.

## Instruction
Clone the repository from vogshpere.

### Build
Compile the binary using:
```
make

```
### Run
Execute the binary from the root of this repository:
```
$ ./webserv webserv.conf
```
### Connect 
You can now access the webserver with your browser at localhost:8085 as specified in the config file.
Alternatively you can send requests with telnet using correct HTTP syntax with the methods GET, POST, DELETE and versions HTTP/1.0 or HTTP/1.1, where HTTP/1.1 requires a host header field. Example:
```
GET / HTTP/1.0
```
### Clean

To clean the object files use:
```
make clean
```
To clean the object files and the executable bindary use:
```
make fclean
```

## Resources
https://www.rfc-editor.org/rfc/inline-errata/rfc1945.html
https://www.tutorialspoint.com/http/http_requests.htm
https://www.youtube.com/watch?v=7GBlCinu9yg
https://beej.us/guide/bgnet0/
https://www.boot.dev/lessons/cf7815cb-5692-4011-a1ab-da592cccb9e8
https://www.youtube.com/watch?v=XXfdzwEsxFk
https://www.geeksforgeeks.org/c/socket-programming-cc/
https://www.linuxtoday.com/blog/multiplexed-i0-with-poll/
https://www.lowlevel.academy



### AI Usage
AI was used to deepen understanding about the specifics of the HTTP and understand differences between version 1.0 and 1.1.