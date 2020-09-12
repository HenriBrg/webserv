

// struct sockaddr_in {
//     sa_family_t    sin_family; /* famille d'adresses : AF_INET     */
//     uint16_t       sin_port;   /* port dans l'ordre d'octets réseau */
//     struct in_addr sin_addr;   /* adresse Internet                  */
// };

// /* Adresse Internet */
// struct in_addr {
//     uint32_t       s_addr;     /* Adresse dans l'ordre d'octets réseau */
// };

 
 
    //  #include <sys/time.h> 
    //    #include <sys/types.h> 
    //    #include <unistd.h> 
    //    #include <stdio.h> 

    //    #define STDIN 0  /* Descripteur de fichier pour entrée standard */

    //    int main()
    //    {
    //        struct timeval tv;
    //        fd_set readfds;

    //        tv.tv_sec = 2;
    //        tv.tv_usec = 500000;

    //        FD_ZERO(&readfds);
    //        FD_SET(STDIN, &readfds);

    //        /* ignorer writefds et exceptfds: */
    //        select(STDIN+1, &readfds, NULL, NULL, &tv);

    //        if (FD_ISSET(STDIN, &readfds))
    //            printf("Une touche à été pressée!\n");
    //        else
    //            printf("Timed out.\n");
    //            return 0;
    //    }

       #include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int
main(void)
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Surveiller stdin (fd 0) en attente d'entrées */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    /* Pendant 5 secondes maxi */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(1, &rfds, NULL, NULL, &tv);
    /* Considérer tv comme indéfini maintenant ! */

    if (retval == -1)
        perror("select()");
    else if (retval)
        printf("Des données sont disponibles maintenant\n");
        /* FD_ISSET(0, &rfds) est vrai */
    else
        printf("Aucune données durant les 5 secondes\n");

    exit(EXIT_SUCCESS);
}