# Server-Client-TCP

This project consists of three separate programs:

1) Site Creator

This bash script creates a number of catalogs-sites and html webpages and fills the webpages with random text segments from a text file. The root directory, the text file and the number of sites and pages are given as arguments.

2) Server

This program implements an HTTP server which supports GET requests of HTTP/1.1. The arguments given in the command line: request PORT, command PORT, Number of Threads, Root Directory.

Server listens on these two sockets and accepts connections. Polling is used to avoid busy waiting. If a connection is successful, the file descriptor returned by ACCEPT is inserted in a synchronized queue. One of the existing threads retrieves that fd and reads from the corresponding socket the HTTP request. It then parses and validates that request according to HTTP standards. After a validated request, the same thread finds the requested .html file, creates a response (e.g 200 OK ...) and sends it through the request socket.

On the other socket the server accepts commands. The two supported commands are: a) STATS b) SHUTDOWN

When STATS, server responds with statistics about the number of webpages sent, total byte size and server time. Shutdown command terminates the program after it frees the memory and terminates any existing threads.

3) Crawler-Client

The Web Crawler is similar with the Server in terms of thread functionality and memory allocation, it just works as a client. It sends requests to the server and with the responses it recreates the server's website directory. Each webpage created by the crawler includes links to both webpages from the same website, and external websites-catalogs. This is made such as every webpage is linked. 

Web Crawler gets the starting URL-link from the command line and inserts it in a synchronized queue. A crawler thread retrieves the link, tokenizes and forms the request for that certain html file. After it gets the file from the webserver it searches for links to other files and recursively all folders and files are created.

