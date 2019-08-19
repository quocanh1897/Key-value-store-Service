#ifndef _CONNECTION_
#define _CONNECTION_
#include <errno.h>
#include <string.h>
#include <iostream>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define IP "127.0.0.1"
#define PORT 7779
#define BACKLOG 32

using namespace std;
int checkErr(int n, char *err, string succ)
{
   if (n == -1)
   {
      perror(err);
      exit(1);
   }
   else
   {
      cout << succ;
   }
   return n;
}
int getConnectionServer(){
    int sockfd;
    int rc, on = 1;
    struct sockaddr_in6 addr;
    /* Get the Socket file descriptor */
   /* AF_INET - IPv4, SOCK_STREAM - TCP/IP */
   checkErr((sockfd = socket(AF_INET6, SOCK_STREAM, 0)),
            "ERROR: Failed to obtain Socket Descriptor.",
            "[Server] Obtaining socket descriptor successfully.\n");

   /* Set sockfd reuseable */
   checkErr(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)),
            "ERROR: setsockopt() failed", "");

   /* Set sockfd NONBLOCKING mode */
   checkErr(rc = ioctl(sockfd, FIONBIO, (char *)&on),
            "ERROR: ioctl() failed", "");
   if (rc < 0)
   {
      close(sockfd);
      exit(-1);
   }

   /* Fill the client socket address struct */
   memset(&addr, 0, sizeof(addr));
   addr.sin6_family = AF_INET6; /* Protocol Family */
   memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
   addr.sin6_port = htons(PORT); /* Port number - host to network short int */
   // inet_pton(AF_INET, IP, &addr.sin6_addr); /* convert IPv4 into sin_addr*/
   // memset(&addr.sin6_zero, '\0', sizeof(addr.sin6_zero));

   /* Bind a special Port */
   checkErr(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)),
            "ERROR: Failed to bind Port.",
            "[Server] Binded tcp port " + to_string(PORT) + " in addr 127.0.0.1 sucessfully.\n");

   /* Listen to connection*/
   checkErr(listen(sockfd, BACKLOG),
            "ERROR: Failed to listen Port.",
            "[Server] Listening the port " + to_string(PORT) + " ....\n");

    return sockfd;
}

#endif