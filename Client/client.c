#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NB_THREADS 2
#define BUFFER_SIZE 200

char *msg = NULL;
char *msg2 = NULL;
int n;
int sizeMess;
int dS;

void formattingMessage(char *msg)
{
  char *prefix = "Server: ";

  if (strncmp(msg, prefix, strlen(prefix)) == 0)
  {
    char *tok = strtok(msg, " ");
    tok = strtok(NULL, "\0");
    printf("\x1b[31m%s\x1b[0m\n", tok);
  }
  else
  {
    printf("\x1b[34m \t \t \t %s\x1b[0m\n", msg);
  }
}

void *sending()
{
  while (1)
  {
    fgets(msg, BUFFER_SIZE, stdin);
    if (msg[strlen(msg) - 1] == '\n')
      msg[strlen(msg) - 1] = '\0';
    // Envoi de la taille du mess
    int sizeMess = strlen(msg) + 1;

    if (send(dS, &sizeMess, 4, 0) == -1)
    {
      printf("Erreur d'envoi\n");
      exit(1);
    }

    // Envoi du mess
    int res = send(dS, msg, sizeMess, 0);
    if (res == -1)
    {
      perror("Erreur d'envoi");
      exit(1);
    }
    else if (res == 0)
    {
      perror("Fin de la connexion");
      pthread_exit(0);
    }

    if (strcmp(msg, "fin") == 0)
    {
      printf("Fin du programme");
      pthread_exit(0);
    }
  }
}

void *receiving()
{
  while (1)
  {
    // Reception taille du mess
    int res = recv(dS, &sizeMess, 4, 0);
    if (res == -1)
    {
      perror("Erreur de reception");
      exit(1);
    }
    else if (res == 0)
    {
      perror("Fin de la connexion");
      pthread_exit(0);
    }

    res = recv(dS, msg, sizeMess, 0);
    if (res == -1)
    {
      printf("Erreur de reception\n");
      exit(1);
    }
    else if (res == 0)
    {
      perror("Fin de la connexion");
      pthread_exit(0);
    }
    else
    {
      // Formatage & affichage du message reçu
      formattingMessage(msg);
    }
  }
  pthread_exit(0);
}

int main(int argc, char *argv[])
{
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <server IP> <port> <username>\n", argv[0]);
    exit(1);
  }

  printf("Début programme\n");
  dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");
  pthread_t thread[NB_THREADS];

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &(aS.sin_addr));
  aS.sin_port = htons(atoi(argv[2]));
  socklen_t lgA = sizeof(struct sockaddr_in);

  if (connect(dS, (struct sockaddr *)&aS, lgA) == -1)
  {
    printf("Erreur de connexion\n");
    exit(1);
  }
  else
  {
    printf("Socket Connecté\n");
  }

  int taille_username = (int)strlen(argv[3]) + 1;
  // Envoi de la taille de l'username
  send(dS, &taille_username, 4, 0);

  // Envoi de l'username
  send(dS, argv[3], taille_username, 0);

  msg = malloc(BUFFER_SIZE * sizeof(char));
  if (!msg)
  {
    perror("Allocation de mémoire échouée");
    exit(1);
  }
  msg2 = malloc(BUFFER_SIZE * sizeof(char));
  if (!msg2)
  {
    perror("Allocation de mémoire échouée");
    exit(1);
  }

  pthread_create(&thread[0], NULL, sending, (void *)0);
  pthread_create(&thread[1], NULL, receiving, (void *)1);
  // fermeture des threads
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);

  // libération de la mémoire
  free(msg);
  free(msg2);

  shutdown(dS, 2);
}