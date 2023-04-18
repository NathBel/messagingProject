#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int id_client = atoi(argv[3]);
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

  int n = 200;

  char *msg = malloc(n * sizeof(char));
  char *msg2 = malloc(n * sizeof(char));
  while (1)
  {
    if ( id_client == 1){
      printf("Entrez un message ('fin' pour quitter) : ");
      fgets(msg, n, stdin);
      if (msg[strlen(msg) - 1] == '\n')
        msg[strlen(msg) - 1] = '\0';
      //Envoi de la taille du mess
      int sizeMess = strlen(msg)+1;
      
      send(dS, &sizeMess, 4, 0);
      //Envoi du mess
      send(dS, msg,sizeMess, 0);
      printf("Message Envoyé \n");
      if(strcmp(msg, "fin") == 0){
        shutdown(dS, 2);
        printf("Fin du programme");
      }
      //Reception taille du mess
      recv(dS, &sizeMess, sizeof(sizeMess), 0);
      printf("Message 2 recu de taille : %d\n", sizeMess);
      //Reception du mess 
      recv(dS, msg2, sizeMess, 0);
      printf("Message reçu : %s\n", msg2);
    } 
    else if (id_client == 2){

      recv(dS, msg, 20, 0);
      printf("Message reçu : %s\n", msg);

      printf("Entrez un message ('fin' pour quitter) : ");
      fgets(msg2, n, stdin);
      if (msg2[strlen(msg2) - 1] == '\n')
        msg2[strlen(msg2) - 1] = '\0';
      //Envoi de la taille du mess
      int sizeMess2 = strlen(msg2)+1;
      send(dS, &sizeMess2, 4, 0);
      //Envoi du mess
      send(dS, msg2,sizeMess2, 0);
      printf("Message Envoyé \n");
      if(strcmp(msg2, "fin") == 0){
        shutdown(dS, 2);
        printf("Fin du programme");
      }
    }
  }
}