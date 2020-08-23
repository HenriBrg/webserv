     #include <sys/time.h> 
       #include <sys/types.h> 
       #include <unistd.h> 
       #include <stdio.h> 

       #define STDIN 0  /* Descripteur de fichier pour entrée standard */

       int main()
       {
           struct timeval tv;
           fd_set readfds;

           tv.tv_sec = 2;
           tv.tv_usec = 500000;

           FD_ZERO(&readfds);
           FD_SET(STDIN, &readfds);

           /* ignorer writefds et exceptfds: */
           select(STDIN+1, &readfds, NULL, NULL, &tv);

           if (FD_ISSET(STDIN, &readfds))
               printf("Une touche à été pressée!\n");
           else
               printf("Timed out.\n");
               return 0;
       }