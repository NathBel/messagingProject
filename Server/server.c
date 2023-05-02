#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
// Nombre de threads crées => nombre clients max
#define NB_THREADS 10

struct client
{
    char *username;
    int socket;
};

// Socket du serveur
int dS;
// Taille max du message 20
char msg[20];
int sizeMess;
int stop = 0;
// Tableau de socket pour les clients
struct client clientsConnected[NB_THREADS];
int indexsocketClients = 0;
int indexSenderAdress;
// tableau de thread
pthread_t thread[NB_THREADS];

// Semaphore pour la queue
sem_t sem;
int threadToClean[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Tableau des threads à nettoyer
// Declare the mutex
pthread_mutex_t mutex;

void endConnection(int indexSenderAdress)
{
    shutdown(clientsConnected[indexSenderAdress].socket, 2);

    // Set socket and username of last element to -1 and NULL
    clientsConnected[indexSenderAdress].socket = -1;
    clientsConnected[indexSenderAdress].username = NULL;

    printf("Client déconnecté\n");

    // décrémente le nombre de clients connectés
    indexsocketClients--;

    // Ajoute le thread à nettoyer
    for (int i = 0; i < 10; i++)
    {
        if (threadToClean[i] == -1)
        {
            threadToClean[i] = indexSenderAdress;
            break;
        }
    }
    sem_post(&sem);

    pthread_exit(0);
}

void checkError(int res, int indexSenderAdress)
{
    if (res == -1)
    {
        perror("Erreur d'envoi\n");
        exit(1);
    }
    else if (res == 0)
    {
        perror("Fin de la connexion\n");
        endConnection(indexSenderAdress);
    }
}

int findClient(char *input_username)
{
    int i = 0;
    int find = 0;
    int socket = -1;
    while (i < NB_THREADS && find == 0)
    {
        if (clientsConnected[i].socket != -1 && clientsConnected[i].username != NULL && strcmp(clientsConnected[i].username, input_username) == 0)
        {
            find = 1;
        }
        i++;
    }
    return find;
}

void mp(char *user, char *privateMessage, int indexSenderAdress)
{
    int find = findClient(user);
    if (find != 1)
    {
        printf("Utilisateur non trouvé\n");
        char *messageToSend = "Server: Utilisateur non trouvé";
        int sizeMessageToSend = strlen(messageToSend) + 1;

        // Envoi de la taille du message
        int res = send(clientsConnected[indexSenderAdress].socket, &sizeMessageToSend, 4, 0);
        checkError(res, indexSenderAdress);

        // Envoi du message
        res = send(clientsConnected[indexSenderAdress].socket, messageToSend, sizeMessageToSend, 0);
        checkError(res, indexSenderAdress);
        return;
    }

    if (privateMessage == NULL)
    {
        char *messageToSend = "Server: Entrez un message privé à envoyer";
        int sizeMessageToSend = strlen(messageToSend) + 1;

        // Envoi de la taille du message
        int res = send(clientsConnected[indexSenderAdress].socket, &sizeMessageToSend, 4, 0);
        checkError(res, indexSenderAdress);

        // Envoi du message
        res = send(clientsConnected[indexSenderAdress].socket, messageToSend, sizeMessageToSend, 0);
        checkError(res, indexSenderAdress);
        return;
    }

    int i = 0;
    int findUser = 0;
    while (i < indexsocketClients && findUser == 0)
    {
        if (strcmp(clientsConnected[i].username, user) == 0)
        {
            int sizeMessageToSend = strlen(privateMessage) + strlen(clientsConnected[i].username) + 22;
            char messageToSend[sizeMessageToSend];
            strcpy(messageToSend, "\x1B[3m");
            strcat(messageToSend, clientsConnected[i].username);
            strcat(messageToSend, " (en mp) : ");
            strcat(messageToSend, privateMessage);
            strcat(messageToSend, "\x1B[0m");

            // Envoi de la taille du message
            int res = send(clientsConnected[i].socket, &sizeMessageToSend, 4, 0);
            checkError(res, indexSenderAdress);

            // Envoi du message
            res = send(clientsConnected[i].socket, messageToSend, sizeMessageToSend, 0);
            checkError(res, indexSenderAdress);
            printf("Message Envoyé\n");
            findUser = 1;
        }
        i++;
    }
    if (findUser == 0)
    {
        printf("Utilisateur non trouvé\n");
    }
}

char *setUsername(int indexSenderAdress)
{
    while (1)
    {
        // Reception taille de l'username
        int res = recv(clientsConnected[indexSenderAdress].socket, &sizeMess, sizeof(sizeMess), 0);
        checkError(res, indexSenderAdress);

        printf("Username recu de taille : %d\n", sizeMess);

        char input_username[sizeMess];

        // Reception de l'username
        res = recv(clientsConnected[indexSenderAdress].socket, input_username, sizeMess, 0);
        checkError(res, indexSenderAdress);

        printf("Username reçu : %s\n", input_username);

        // Vérification si l'username est déjà utilisé
        int findUser = findClient(input_username);

        if (findUser == 0)
        {
            // allouer memoire dans tableau
            clientsConnected[indexSenderAdress].username = malloc(sizeMess * sizeof(char));
            strcpy(clientsConnected[indexSenderAdress].username, input_username);

            char *successMessage = "Server: Bienvenue sur notre messagerie !\n";
            int sizeSucessError = strlen(successMessage) + 1;
            res = send(clientsConnected[indexSenderAdress].socket, &sizeSucessError, 4, 0);
            checkError(res, indexSenderAdress);

            res = send(clientsConnected[indexSenderAdress].socket, successMessage, sizeSucessError, 0);
            checkError(res, indexSenderAdress);
            break;
        }
        else
        {
            printf("Username déjà utilisé\n");
            char *messageError = "Server: Username déjà utilisé, choisissez-en un nouveau !";
            int sizeMessageError = strlen(messageError) + 1;
            res = send(clientsConnected[indexSenderAdress].socket, &sizeMessageError, 4, 0);
            checkError(res, indexSenderAdress);

            res = send(clientsConnected[indexSenderAdress].socket, messageError, sizeMessageError, 0);
            if (res == -1)
            {
                printf("Erreur d'envoi\n");
                exit(1);
            }
            else if (res == 0)
            {
                printf("Fin de la connexion\n");
                endConnection(indexSenderAdress);
                break;
            }
        }
    }
}

void *cleaningThread()
{
    while (1)
    {
        sem_wait(&sem);
        for (int i = 0; i < 10; i++)
        {
            if (threadToClean[i] != -1)
            {
                if (threadToClean[i] < 0 || threadToClean[i] >= NB_THREADS)
                {
                    fprintf(stderr, "Invalid thread index: %d\n", threadToClean[i]);
                    exit(1);
                }
                int res = pthread_join(thread[threadToClean[i]], NULL);
                if (res != 0)
                {
                    fprintf(stderr, "Error joining thread %d: %s\n", threadToClean[i], strerror(res));
                    exit(1);
                }
                printf("Thread %d joined successfully\n", threadToClean[i]);
                threadToClean[i] = -1;
            }
        }
    }
}

int checkIfCommand(char *msg)
{
    if (msg[0] == '/')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char *getManuel()
{
    char *filename = "../manuel.txt";

    // Open file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file %s", filename);
        exit(1);
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate memory for file content
    char *file_content = (char *)malloc(file_size + 1);

    // Read file content into memory
    fread(file_content, file_size, 1, file);
    file_content[file_size] = '\0';

    // Close file
    fclose(file);

    // Print file content
    return (file_content);
}

void getCommand(char *msg, int indexSenderAdress)
{
    char *tok = strtok(msg, " ");

    char *cmd = tok;

    if (strcmp(cmd, "/mp") == 0)
    {
        tok = strtok(NULL, " ");
        char *user = tok;

        tok = strtok(NULL, "\0");
        char *privateMessage = tok;

        mp(user, privateMessage, indexSenderAdress);
    }
    else if (strcmp(cmd, "/fin") == 0)
    {
        endConnection(indexSenderAdress);
    }
    else if (strcmp(cmd, "/manuel") == 0)
    {
        char *messageToSend = malloc(1024 * sizeof(char));
        strcpy(messageToSend, "Server: ");
        strcat(messageToSend, getManuel());
        int sizeMessageToSend = strlen(messageToSend) + 1;

        // Envoi de la taille du message
        int res = send(clientsConnected[indexSenderAdress].socket, &sizeMessageToSend, sizeof(sizeMessageToSend), 0);
        checkError(res, indexSenderAdress);

        // Envoi du message
        res = send(clientsConnected[indexSenderAdress].socket, messageToSend, sizeMessageToSend, 0);
        checkError(res, indexSenderAdress);
    }
    else
    {
        printf("Commande non reconnue\n");
        char *messageToSend = "Server: Commande non reconnue, tapez /manuel pour voir la liste des commandes";
        int sizeMessageToSend = strlen(messageToSend) + 1;

        // Envoi de la taille du message
        int res = send(clientsConnected[indexSenderAdress].socket, &sizeMessageToSend, sizeof(sizeMessageToSend), 0);
        checkError(res, indexSenderAdress);

        // Envoi du message
        res = send(clientsConnected[indexSenderAdress].socket, messageToSend, sizeMessageToSend, 0);
        if (res == -1)
        {
            printf("Erreur d'envoi\n");
            exit(1);
        }
        else if (res == 0)
        {
            printf("Fin de la connexion\n");
            endConnection(indexSenderAdress);
            return;
        }
    }
}

void *sendToClients(void *t)
{
    int indexSenderAdress = (long)t;

    setUsername(indexSenderAdress);

    while (stop == 0)
    {
        printf("En attente message : %s\n", clientsConnected[indexSenderAdress].username);
        // Reception taille du mess
        int res = recv(clientsConnected[indexSenderAdress].socket, &sizeMess, sizeof(sizeMess), 0);

        checkError(res, indexSenderAdress);

        printf("Message recu de taille : %d\n", sizeMess);
        // Reception du mess
        res = recv(clientsConnected[indexSenderAdress].socket, msg, sizeMess, 0);

        checkError(res, indexSenderAdress);

        if (checkIfCommand(msg) == 0)
        {
            printf("Message reçu : %s\n", msg);
            if (strcmp(msg, "fin") != 0)
            {
                int sizeMessageToSend = strlen(msg) + strlen(clientsConnected[indexSenderAdress].username) + 4;
                char messageToSend[sizeMessageToSend];
                strcpy(messageToSend, clientsConnected[indexSenderAdress].username);
                strcat(messageToSend, " : ");
                strcat(messageToSend, msg);

                // Envoi à tous les clients
                for (int i = 0; i < NB_THREADS; i++)
                {
                    if (i != indexSenderAdress && clientsConnected[i].socket != -1)
                    {
                        // Envoi de la taille du message
                        res = send(clientsConnected[i].socket, &sizeMessageToSend, 4, 0);
                        if (res == -1)
                        {
                            printf("Erreur d'envoi\n");
                            exit(1);
                        }
                        else if (res == 0)
                        {
                            printf("Fin de la connexion\n");
                            endConnection(indexSenderAdress);
                            break;
                        }

                        // Envoi du message
                        res = send(clientsConnected[i].socket, messageToSend, sizeMessageToSend, 0);
                        if (res == -1)
                        {
                            printf("Erreur d'envoi\n");
                            exit(1);
                        }
                        else if (res == 0)
                        {
                            printf("Fin de la connexion\n");
                            endConnection(indexSenderAdress);
                            break;
                        }

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
                clientsConnected[indexSenderAdress].socket = -1;
                pthread_exit(0);
            }
        }
        else
        {
            getCommand(msg, indexSenderAdress);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    printf("Début programme\n");
    struct sockaddr_in aC;
    socklen_t lg = sizeof(aC);

    dS = socket(PF_INET, SOCK_STREAM, 0);
    printf("Socket Créée\n");

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(atoi(argv[1]));
    int res = bind(dS, (struct sockaddr *)&ad, sizeof(ad));
    if (res == -1)
    {
        printf("Erreur de bind\n");
        exit(1);
    }
    printf("Socket Nommé\n");

    listen(dS, 7);
    printf("Mode écoute\n");
    int trouve = 0; // variable pour savoir si on a trouvé une place vide dans le tableau
    int indexFoundsclients = 0;
    int newClientSocket;

    // Initialisation du semaphore
    sem_init(&sem, 0, 0);

    // création du thread de nettoyage
    pthread_t threadClean;
    pthread_create(&threadClean, NULL, cleaningThread, NULL);

    for (int i = 0; i < NB_THREADS; i++)
        clientsConnected[i].socket = -1;

    while (1)
    {
        // si il y a de la place alors on attend un client
        if (indexsocketClients < NB_THREADS)
        {
            // struct sockaddr_in aC[indexAC];
            // boucle pour trouver une place vide dans le tableau

            indexsocketClients++; // on incrémente le nombre de clients connectés
            newClientSocket = accept(dS, (struct sockaddr *)&aC, &lg);
            if (newClientSocket == -1)
            {
                printf("Erreur : client pas accepté\n");
            }
            else
            {
                pthread_mutex_lock(&mutex);
                trouve = 0;
                indexFoundsclients = 0;
                while (!trouve)
                {
                    if (clientsConnected[indexFoundsclients].socket == -1)
                    {
                        trouve = 1;
                    }
                    else
                    {
                        indexFoundsclients++;
                    }
                }
                struct client newClient = {NULL, newClientSocket};

                clientsConnected[indexFoundsclients] = newClient;
            }
            printf("Nouveau client Connecté\n");
            pthread_create(&thread[indexFoundsclients], NULL, sendToClients, (void *)(long)indexFoundsclients);
            pthread_mutex_unlock(&mutex);
        }
    }
    printf("Fin du programme\n");
    shutdown(dS, 2);
    free(clientsConnected);
    free(msg);
    free(thread);
}