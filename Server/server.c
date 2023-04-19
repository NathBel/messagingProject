#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define NB_THREADS 2

int dSC;
int dSC2;
char msg[20];
int sizeMess;
int stop = 0;

void *sendToClient2()
{
    while (stop == 0)
    {
        // Reception taille du mess
        recv(dSC, &sizeMess, sizeof(sizeMess), 0);
        printf("Message 2 recu de taille : %d\n", sizeMess);
        // Reception du mess
        recv(dSC, msg, sizeMess, 0);

        if (strcmp(msg, "fin") != 0)
        {
            printf("Message reçu : %s\n", msg);

            // Envoi de la taille du mess
            send(dSC2, &sizeMess, 4, 0);

            // Envoie message
            send(dSC2, msg, sizeMess, 0);
            printf("Message Envoyé\n");
        }
        else
        {
            printf("Fin de la connexion\n");
            stop = 1;
            pthread_exit(0);
        }
    }
}

void *sendToClient1()
{
    while (stop == 0)
    {
        // Reception taille du mess
        recv(dSC2, &sizeMess, sizeof(sizeMess), 0);
        printf("Message 2 recu de taille : %d\n", sizeMess);
        // Reception du mess
        recv(dSC2, msg, sizeMess, 0);

        if (strcmp(msg, "fin") != 0)
        {
            printf("Message reçu : %s\n", msg);

            // Envoi de la taille du mess
            send(dSC, &sizeMess, 4, 0);

            // Envoie message
            send(dSC, msg, sizeMess, 0);
            printf("Message Envoyé\n");
        }
        else
        {
            printf("Fin de la connexion\n");
            stop = 1;
            pthread_exit(0);
        }
    }
}

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

    while (1)
    {
        pthread_t thread[NB_THREADS];

        struct sockaddr_in aC;
        struct sockaddr_in aC2;
        socklen_t lg = sizeof(struct sockaddr_in);
        socklen_t lg2 = sizeof(struct sockaddr_in);
        dSC = accept(dS, (struct sockaddr *)&aC, &lg);
        printf("Client Connecté\n");
        dSC2 = accept(dS, (struct sockaddr *)&aC2, &lg2);
        printf("Client 2 Connecté\n");

        pthread_create(&thread[0], NULL, sendToClient1, (void *)0);
        pthread_create(&thread[1], NULL, sendToClient2, (void *)1);

        pthread_join(thread[0], NULL);
        pthread_join(thread[1], NULL);

        shutdown(dSC, 2);
        shutdown(dSC2, 2);
        printf("Fin du programme\n");
    }
}