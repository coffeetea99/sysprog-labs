/*
 * proxy.c - a simple, iterative HTTP proxy server
 */
#include "csapp.h"
#include "cache.h"

#define PROXY_LOG "proxy.log"

void doit(int fd);
void proxy_cache_log(char*, char*, int);
void print_requesthdrs(rio_t *rp);
void parse_uri_proxy(char*,char*,int*);
void clienterror(int fd, char *cause, char *errnum,char *shortmsg, char *longmsg);
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
/*
 * main: 
 *  listens for connections on the given port number, handles HTTP requests 
 *  with the doit function then closes the connection 
 */

  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /// listen for connections
  /// if a client connects, accept the connection, handle the requests
  /// (call the do it function), then close the connection
  port = atoi(argv[1]);
  listenfd = Open_listenfd(port);
  while(1){
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
    doit(connfd);
    Close(connfd);
  }
}

//-----------------------------------------------------------------------------
void doit(int fd)
{
/*
 * doit: reads HTTP request from connection socket, forwards the request to the
 *  requested host. Reads the response from the host server, and writes the
 *  response back to the client 
 * params:
 *    - fd (int): file descriptor of the connection socket.
 */  
	
  char line[MAXLINE], host[MAXLINE];
  char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  int serverfd, port=80;

  rio_t rio;
  Rio_readinitb(&rio, fd);

  /// read request header
  Rio_readlineb(&rio, line, MAXLINE);
  sscanf(line, "%s %s %s", method, uri, version);

  /// get hostname, port, filename by parse_uri()
  parse_uri_proxy(uri, host, &port);

  /// check the method is GET, if it is not, return a 501 error by using clienterror()
  if (strcmp(method, "GET")) {
    clienterror(fd, method, "501", "Not implemented", "This method is not implemented");
    return;
  }

  print_requesthdrs(&rio);

  /// find the URI in the proxy cache. 
  /// if the URI is in the cache, send directly to the client 
  /// be sure to write the log when the proxy server send to the client 
  cache_block* cache_content;
  char cached;
  int contentLength;

  if ( (cache_content = find_cache_block(uri)) == NULL)
  {
    /* --- not in the cache ---*/
    cached = 0;

    /// read response header
    /// read the response header from the server and build the proxy's responseBuffer
    /// header by repeatedly adding the responseBuffer (server response)
    /// this proxy server only supports 'Content-Length' format.

    /// send request to server
    serverfd = Open_clientfd(host, port);
    Rio_writen(serverfd, line, strlen(line));

    char hostline[MAXLINE];
    sprintf(hostline, "Host: %s:%d\r\n\r\n", host, port);
    Rio_writen(serverfd, hostline, strlen(hostline));

    /// get response header from server and write to client

    char lengthHeader[20];
    char responseBuffer[RESP_SIZE];
    memset(responseBuffer, 0, sizeof(responseBuffer));
    strcpy(lengthHeader, "Content-Length: ");

    Rio_readinitb(&rio, serverfd);
    Rio_readlineb(&rio, line, MAXLINE);
    while (strcmp(line, "\r\n"))
    {

      /// get length of the content
      if (strncmp(lengthHeader, line, strlen(lengthHeader)) == 0)
      {
        contentLength = atoi(line + strlen(lengthHeader));
      }

      Rio_writen(fd, line, strlen(line));
      sprintf(responseBuffer, "%s%s", responseBuffer, line);
      Rio_readlineb(&rio, line, MAXLINE);
    }
    Rio_writen(fd, "\r\n", strlen("\r\n"));
    sprintf(responseBuffer, "%s\r\n", responseBuffer);

    /// Content-Length
    /// using the 'Content-Length' read from the http server response header,
    /// you must allocate and read that many bytes to our buffer
    /// you now write the response heading and the content back to the client
    char contentBuffer[contentLength];
    Rio_readnb(&rio, contentBuffer, contentLength);
    Rio_writen(fd, contentBuffer, contentLength);

    /// add the proxy cache
    /// logging the cache status and other information
    /// check the free or close
    add_cache_block(uri, contentBuffer, responseBuffer, contentLength);
  }
  else
  {
    /* --- in the cache ---*/
    cached = 1;

    contentLength = (*cache_content).contentLength;

    char* response = (*cache_content).resp;
    Rio_writen(fd, response, strlen(response));
    char* content = (*cache_content).content;
    Rio_writen(fd, content, contentLength);
  }

  proxy_cache_log(&cached, uri, contentLength);
}

void proxy_cache_log(char* cached, char* uri, int contentLength){
/*
 * proxy_cache_log:
 * 		keep the track of all the cache-log when add or find the contents
 *params:
 	-cached: status of the cache corresponding uri
	-uri: uri string
	-contentLength: size of the content length (bytes)
 * 		
 */
  const char * days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char * months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  char timebuf[MAXLINE];
  time_t timet;
  struct tm *timeinfo;

  time(&timet);
  timeinfo = localtime(&timet);
  sprintf(timebuf, "%s %d %s %d %d:%d:%d KST:",
          days[(*timeinfo).tm_wday], (*timeinfo).tm_mday, months[(*timeinfo).tm_mon],
          1900 + (*timeinfo).tm_year, (*timeinfo).tm_hour, (*timeinfo).tm_min, (*timeinfo).tm_sec);

  int fd;
  if ((fd = open("./proxy.log", O_WRONLY | O_CREAT | O_APPEND)) > 0)
  {
    char log[MAXLINE];
    sprintf(log, "[%s] %s %s %d\n", *cached == 1 ? "cached" : "uncached", timebuf, uri, contentLength);
    write(fd, log, strlen(log));
    close(fd);
  } else {
    printf("Failed to open log file\n");
  }
}

void print_requesthdrs(rio_t *rp)
{
/**** DO NOT MODIFY ****/
/**** WARNING: This will read out everything remaining until a line break ****/
/*
 * print_requesthdrs: 
 *        reads out and prints all request lines sent to the server
 * params:
 *    - rp: Rio pointer for reading from file
 *
 */
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf, "\r\n")) {
    printf("%s", buf);
    Rio_readlineb(rp, buf, MAXLINE);
  }
    printf("\n");
  return;
}

void parse_uri_proxy(char* uri, char* host, int *port){
/*
 * parse_uri_proxy:
 * 		 Get the hostname, port, and parse the uri.
 * params:
 * 		- uri: uri string 
 * 		- host: host string extracted from the uri.
 * 		- port: port number
 *
 * you have to add further parsing steps in http.c 
 * 
 * example1: http://www.snu.ac.kr/index.html
 *			 host: www.snu.ac.kr 
 *			 filename: should be /index.html 
 *			 port: 80 (default)
 *
 * example2: http://www.snu.ac.kr:1234/index.html
 * 			 host: www.snu.ac.kr
 * 			 filename: /index.html 
 * 			 port: 1234
 *
 * example3: http://127.0.0.1:1234/index.html
 * 			 host: 127.0.0.1
 * 			 filename: /index.html
 * 			 port: 1234
 * 			 
 *	
*/
  int length;
  int uriptr, hostptr = 0, portptr = 0;
  char portstr[MAXLINE];
  int status;

  length = strlen(uri);
  status = 0;   //0: host, 1: port, 2: filename
  for ( uriptr = 7 ; uriptr < length ; ++uriptr ) {
    if ( status == 0 ) {
      if ( uri[uriptr] == ':' ) {
        host[hostptr]= '\0';
        status = 1;
      } else if ( uri[uriptr] == '/' ) {
        host[hostptr]= '\0';
        status = 2;
      } else {
        host[hostptr] = uri[uriptr];
        ++hostptr;
      }
    } else if ( status == 1 ) {
      if ( uri[uriptr] == '/') {
        portstr[portptr] = '\0';
        status = 2;
      } else {
        portstr[portptr] = uri[uriptr];
        ++portptr;
      }
    }
  }

  if ( portptr > 0 ) {
    *port = atoi(portstr);
  }
}

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
/**** DO NOT MODIFY ****/
/*
 * clienterror: 
 *        Creates appropriate HTML error page and sends to the client 
 * params:
 *    - fd: file descriptor of connection socket.
 *    - cause: what has caused the error: the filename or the method
 *    - errnum: The HTTP status (error) code
 *    - shortmsg: The HTTP status (error) message
 *    - longmsg: A longer description of the error that will be printed 
 *           on the error page
 *
 */  
  char buf[MAXLINE], body[MAXBUF];
  ///build the HTTP response body 
  sprintf(body, "<html><title>Mini Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s<b>%s: %s</b>\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>Mini Web server</em>\r\n", body);

  /// print the HTTP response
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-Length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

