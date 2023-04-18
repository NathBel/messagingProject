#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
   
printf("Début programme\n");

        int dS = socket(PF_INET, SOCK_STREAM, 0);
        printf("Socket Créé\n");

        struct sockaddr_in ad;
        ad.sin_family = AF_INET;
        ad.sin_addr.s_addr = INADDR_ANY;
        ad.sin_port = htons(atoi(argv[1]));
        bind(dS, (struct sockaddr *)&ad, sizeof(ad));
        printf("Socket Nommé\n");


        listen(dS, 7);
        printf("Mode écoute\n");


    while(1){

        struct sockaddr_in aC;
        struct sockaddr_in aC2;
        socklen_t lg = sizeof(struct sockaddr_in);
        socklen_t lg2 = sizeof(struct sockaddr_in);
        int dSC = accept(dS, (struct sockaddr *)&aC, &lg);
        printf("Client Connecté\n");
        int dSC2 = accept(dS, (struct sockaddr *)&aC2, &lg2);
        printf("Client 2 Connecté\n");

        char msg[20];
        int sizeMess;
        int stop = 0;

        while (!stop)
        {
            //Reception taille du mess
            recv(dSC, &sizeMess, sizeof(sizeMess), 0);
            printf("Message 1 recu de taille : %d\n", sizeMess);
            //Reception du mess
            recv(dSC, msg, sizeMess, 0);

            if (strcmp(msg, "fin") != 0)
            {
                printf("Message reçu : %s\n", msg);
                
                send(dSC2 , &msg, sizeMess, 0);
                printf("Message Envoyé\n");
            }
            else
            {
                printf("Fin de la connexion\n");
                stop = 1;
            }

            if(stop != 1){
                
                //Reception taille du mess
                recv(dSC2, &sizeMess, sizeof(sizeMess), 0);
                printf("Message 2 recu de taille : %d\n", sizeMess);
                //Reception du mess 
                recv(dSC2, msg, sizeMess, 0);

                if (strcmp(msg, "fin") != 0)
                {
                    printf("Message reçu : %s\n", msg);
                    //Envoi de la taille du mess

                    send(dSC, &sizeMess, 4, 0);
                    //Envoie message
                    send(dSC, msg, sizeMess, 0);
                    printf("Message Envoyé\n");
                }
                else
                {
                    printf("Fin de la connexion\n");
                    stop = 1;
                }
            }
        }
        shutdown(dSC, 2);
        shutdown(dSC2, 2);
        printf("Fin du programme\n");
    }
}