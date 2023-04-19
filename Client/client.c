#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define NB_THREADS 2

char *msg;
char *msg2;
int n;
int sizeMess;
int dS;

void *sending()
{
  while (1)
  {
    printf("Entrez un message ('fin' pour quitter) : ");
    fgets(msg, sizeof(msg), stdin);
    if (msg[strlen(msg) - 1] == '\n')
      msg[strlen(msg) - 1] = '\0';
    // Envoi de la taille du mess
    int sizeMess = strlen(msg) + 1;

    if (send(dS, &sizeMess, 4, 0) == -1)
    {
      printf("Erreur d'envoi\n");
      exit(1);
    }
    else
    {
      printf("Taille du message : %d\n", sizeMess);
    }
    // Envoi du mess
    if (send(dS, msg, sizeMess, 0) == -1)
    {
      printf("Erreur d'envoi\n");
      exit(1);
    }
    else
    {
      printf("Message Envoyé \n");
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
    if (recv(dS, &sizeMess, 4, 0) == -1)
    {
      printf("Erreur de reception\n");
      exit(1);
    }
    else
    {
      printf("Message 2 recu de taille : %d\n", sizeMess);
    }

    if (recv(dS, msg, sizeMess, 0) == -1)
    {
      printf("Erreur de reception\n");
      exit(1);
    }
    else
    {
      printf("Message reçu : %s\n", msg);
    }
  }
  pthread_exit(0);
}

int main(int argc, char *argv[])
{

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
  n = 200;

  msg = malloc(n * sizeof(char));
  msg2 = malloc(n * sizeof(char));

  pthread_create(&thread[0], NULL, sending, (void *)0);
  pthread_create(&thread[1], NULL, receiving, (void *)1);
  // fermeture des threads
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);
  shutdown(dS, 2);
}