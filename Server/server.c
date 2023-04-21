#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
// Nombre de threads crées => nombre clients max
#define NB_THREADS 10

// Socket du serveur
int dS;
// Taille max du message 20
char msg[20];
int sizeMess;
int stop = 0;
// Tableau de socket pour les clients
int socketClients[NB_THREADS];
int indexsocketClients = 0;

void *sendToClients(void *t)
{
    int indexSenderAdress = (long)t;
    while (stop == 0)
    {
        // Reception taille du mess
        recv(socketClients[indexSenderAdress], &sizeMess, sizeof(sizeMess), 0); 
        printf("Message recu de taille : %d\n", sizeMess);
        // Reception du mess
        recv(socketClients[indexSenderAdress], msg, sizeMess, 0);

        if (strcmp(msg, "fin") != 0)
        {
            printf("Message reçu : %s\n", msg);

            // Envoi à tous les clients
            for (int i = 0; i < NB_THREADS; i++)
            {
                if (i != indexSenderAdress && socketClients[i] != -1)
                {
                    // Envoi de la taille du message
                    send(socketClients[i], &sizeMess, 4, 0);

                    // Envoi du message
                    send(socketClients[i], msg, sizeMess, 0);
                    printf("Message Envoyé\n");
                }
            }
        }
        else
        {
            printf("Fin de la connexion\n");
            stop = 1;
            // décrémente le nombre de clients connectés
            indexsocketClients--;
            // -1 dans le tableau = place libre
            socketClients[indexSenderAdress] = -1; 
            pthread_exit(0);
        }
    }
}

int main(int argc, char *argv[])
{

    printf("Début programme\n");
    struct sockaddr_in aC;
    pthread_t thread[NB_THREADS];
    socklen_t lg = sizeof(aC);
    ;
    dS = socket(PF_INET, SOCK_STREAM, 0);
    printf("Socket Créé\n");

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(atoi(argv[1]));
    bind(dS, (struct sockaddr *)&ad, sizeof(ad));
    printf("Socket Nommé\n");

    listen(dS, 7);
    printf("Mode écoute\n");
    int trouve = 0; // variable pour savoir si on a trouvé une place vide dans le tableau
    int indexFoundsclients = 0;
    int newClient;

    for (int i = 0; i < NB_THREADS; i++)
        socketClients[i] = -1;

    while (1)
    {
        // si il y a de la place alors on attend un client
        if (indexsocketClients < NB_THREADS)
        {
            // struct sockaddr_in aC[indexAC];
            // boucle pour trouver une place vide dans le tableau

            indexsocketClients++; // on incrémente le nombre de clients connectés
            newClient = accept(dS, (struct sockaddr *)&aC, &lg);
            if (newClient == -1)
            {
                printf("Error accepting socket client");
            }
            else
            {
                trouve = 0;
                indexFoundsclients = 0;
                while (!trouve)
                {
                    if (socketClients[indexFoundsclients] == -1)
                    {
                        trouve = 1;
                    }
                    else
                    {
                        indexFoundsclients++;
                    }
                }
                socketClients[indexFoundsclients] = newClient;
            }
            printf("Nouveau client Connecté\n");
            pthread_create(&thread[indexFoundsclients], NULL, sendToClients, (void *)indexFoundsclients);

        }
    }
    printf("Fin du programme\n");
}