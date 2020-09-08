/*

https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/

Steps : 

1. Create TCP i.e Listening socket
2. Create a UDP socket
3. Bind both socket to server address.
4. Initialize a descriptor set for select and calculate maximum of 2 descriptor for which we will wait
5. Call select and get the ready descriptor(TCP or UDP)
6. Handle new connection if ready descriptor is of TCP OR receive data gram if ready descriptor is of UDP

*/

/*

int select(int maxfd, fd_set *readsset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout);
Return positive count of descriptors ready, 0 on timeout, -1 error

Arguments:

- maxfd: maximum number of descriptor ready.
- timeout: How long to wait for select to return.

    - if timeout==NULL then wait forever
    - if timeout == fixed_amount_time then wait until specified time
    - if timeout == 0 return immediately.

readset: Descriptor set that we want kernel to test for reading.
writeset: Descriptor set that we want kernel to test for writing.
exceptset: Descriptor set that we want kernel to test for exception conditions.


int read(int sockfd, void * buff, size_t nbytes);
Returns:  number of bytes read from the descriptor. -1 on error

Arguments:

- sockfd: Descriptor which receives data.
- buff: Application buffer socket descriptor data is copied to this buffer.
- nbytes:Number of bytes to be copied to application buffer.

*/

#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#define PORT 5000 
#define MAXLINE 1024 

int max(int x, int y) 
{ 
    if (x > y) 
        return x; 
    else
        return y; 
} 

int main() 
{ 
    int listenfd, connfd, udpfd, nready, maxfdp1; 
    char buffer[MAXLINE]; 
    pid_t childpid; 
    fd_set rset; 
    ssize_t n; 
    socklen_t len; 
    const int on = 1; 
    struct sockaddr_in cliaddr, servaddr; 
    char* message = "Hello Client"; 
    void sig_chld(int); 
  
    /* create listening TCP socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0); 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // binding server addr structure to listenfd 
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
    listen(listenfd, 10); 
  
    /* create UDP socket */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0); 
    // binding server addr structure to udp sockfd 
    bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
  
    // clear the descriptor set 
    FD_ZERO(&rset); 
  
    // get maxfd 
    maxfdp1 = max(listenfd, udpfd) + 1;
    printf("%d\n", maxfdp1);
    for (;;) { 
  
        // set listenfd and udpfd in readset 
        FD_SET(listenfd, &rset); 
        FD_SET(udpfd, &rset); 
  
        // select the ready descriptor 
        nready = select(maxfdp1, &rset, NULL, NULL, NULL); 
  
        // if tcp socket is readable then handle 
        // it by accepting the connection 
        if (FD_ISSET(listenfd, &rset)) { 
            len = sizeof(cliaddr); 
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
            if ((childpid = fork()) == 0) { 
                close(listenfd); 
                bzero(buffer, sizeof(buffer)); 
                printf("Message From TCP client: "); 
                read(connfd, buffer, sizeof(buffer)); 
                puts(buffer); 
                write(connfd, (const char*)message, sizeof(buffer)); 
                close(connfd); 
                exit(0); 
            } 
            close(connfd);
        } 
        // if udp socket is readable receive the message. 
        if (FD_ISSET(udpfd, &rset)) { 
            len = sizeof(cliaddr); 
            bzero(buffer, sizeof(buffer)); 
            printf("\nMessage from UDP client: "); 
            n = recvfrom(udpfd, buffer, sizeof(buffer), 0, 
                         (struct sockaddr*)&cliaddr, &len); 
            puts(buffer); 
            sendto(udpfd, (const char*)message, sizeof(buffer), 0, 
                   (struct sockaddr*)&cliaddr, sizeof(cliaddr)); 
        } 
    } 
}