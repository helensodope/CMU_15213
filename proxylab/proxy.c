/*this implementation of proxy takes a request from the client,
 * sends the request to the server on behalf of the client,
 * receives response from the server and forward it to the client.
 * Caching is implemented and used to ensure that if the request is
 * in the cache, response is sent to the client without connecting to
 * the server. Pseudo-LRU policy is used for eviction process for
 * caching.*/
#include "csapp.h"
#include "cache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";
//global variables
sem_t mutex, cache_mut, op_mut;
cache_block *cache;
int readcnt;
//helper functions
void *thread(void *vargp);
void operate(int connfd);
int parse(char *uri, char *hostname, char *port, char *filepath);

//main function to initialize cache and semaphores
//Also accepts connection, creates threads
int main(int argc, char *argv[])
{
    int listenfd;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    int *connfd;
    pthread_t tid;
    cache = NULL;
    readcnt = 0;

    sem_init(&cache_mut, 0, 1);
    sem_init(&op_mut, 0, 1);
    sem_init(&mutex, 0, 1);
    //ignore sigpipe
    Signal(SIGPIPE, SIG_IGN);

    if(argc != 2) 
    {
        fprintf(stderr, "wrong number of arguments\n");
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while(1)
    {
        connfd = malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        //lock before creating threads
        P(&mutex);
        pthread_create(&tid, NULL, thread, connfd);
    }
    return 0;
}

//creates threads
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    //unlock
    V(&mutex);
    pthread_detach(pthread_self());
    //note that vargp, connfd from main, was malloced
    free(vargp);
    operate(connfd);
    close(connfd);
    return NULL;
}

/*read request from client
parse request
if request not in cache, send request to server
read response from server
write(forward) to client
add to cache*/
void operate(int connfd)
{
    rio_t client_rio, server_rio;
    char buf[MAXLINE], response_buf[MAXLINE],
         method[MAXLINE], uri[MAXLINE], httpver[MAXLINE],
         hostname[MAXLINE], filepath[MAXLINE],
         proxy_request[MAXLINE],
         port[MAXLINE];
    int serverfd, numbytes;
    size_t temp;
    cache_block *block;

    //read from client
    Rio_readinitb(&client_rio, connfd);
    Rio_readlineb(&client_rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, httpver);
    
    //check if request exists in cache
    P(&op_mut);
    readcnt++;
    if(readcnt == 1) P(&cache_mut);
    V(&op_mut);

    //access cache safely
    block = cache_inquiry(uri, cache);
    
    P(&op_mut);
    readcnt--;
    if(readcnt == 0) V(&cache_mut);
    V(&op_mut);
    //lock to write to client, unlock afterwards
    P(&mutex);
    if(block != NULL)
    {
        /*********request exits in cache*****************/
        Rio_writen(connfd, block->buf, block->size);
        V(&mutex);
        return;
    }
    V(&mutex);
    
    /***********request doesn't exist in cache*********/
    //parse uri
    if (!parse(uri, hostname, port, filepath))
    {
        fprintf(stderr, "parsing error");
        return;
    }
    //prepare request
    sprintf(proxy_request, "GET %s HTTP/1.0\r\n", filepath);
    //headers
    strcat(proxy_request, "Host: ");
    strcat(proxy_request, hostname);
    strcat(proxy_request, "\r\n");
    strcat(proxy_request, user_agent_hdr);
    strcat(proxy_request, accept_hdr);
    strcat(proxy_request, accept_encoding_hdr);
    strcat(proxy_request, "Connection: close\r\n");
    strcat(proxy_request, "Proxy-Connection: close\r\n");
    strcat(proxy_request, "\r\n");
    //request to server
    serverfd = open_clientfd(hostname, port);
    //send line to server
    Rio_writen(serverfd, proxy_request, strlen(proxy_request));
    //read response from server
    Rio_readinitb(&server_rio, serverfd);
    numbytes = 0;
    while((temp = Rio_readnb(&server_rio, response_buf, MAXLINE)) > 0)
    {
        //write response to client
        Rio_writen(connfd, response_buf, temp);
        numbytes += temp;
    }
    //since this request wasn't in cache, add to cache
    if(numbytes <= MAX_OBJECT_SIZE)
    {
        P(&cache_mut);
        //safe cache access
        cache = cache_insert(uri, response_buf, numbytes, cache);
        V(&cache_mut);
    }
    Close(serverfd);
}

//parse given uri to hostname, port, filepath
int parse(char *uri, char *hostname, char *port, char *filepath)
{
    char host_temp[MAXLINE];
    char *http = strstr(uri, "http");
    char *name_end;
    char *portptr;
    int len = strlen("http://");

    if(http == NULL) return 0;
    http += len;
    
    name_end = strchr(http, '/');
    
    strcpy(filepath, name_end);
    *name_end = '\0';
    strcpy(host_temp, http);
    *name_end = '/';

    portptr = strstr(host_temp, ":");
    strcpy(hostname, host_temp);
    strcpy(port, "80");
    if(portptr != NULL)
    {
        *portptr = '\0';
        strcpy(port, portptr+1);
        strcpy(hostname, host_temp);
        *portptr = ':';
        return 1;
    }
    return 1;
}
