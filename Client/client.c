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

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &(aS.sin_addr));
  aS.sin_port = htons(atoi(argv[2]));
  socklen_t lgA = sizeof(struct sockaddr_in);
  connect(dS, (struct sockaddr *)&aS, lgA);
  printf("Socket Connecté\n");

  int n = 20;

  char *msg = malloc(n * sizeof(char));
  while (1)
  {

    //Partie envoi
    printf("Entrez un message ('fin' pour quitter) : ");
    fgets(msg, n, stdin);
    if (msg[strlen(msg) - 1] == '\n')
      msg[strlen(msg) - 1] = '\0';
    send(dS, msg, strlen(msg)+1, 0);
    printf("Message Envoyé \n");

    if (strcmp(msg, "fin") == 0)
    {
      break;
    }
  }

  shutdown(dS, 2);
  printf("Fin du programme");
}