#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>



#define NB_THREADS 2
#define BUFFER_SIZE 200
#define FILE_SIZE 1024

char *msg = NULL;
char *msg2 = NULL;
char filename[BUFFER_SIZE];
int n;
int sizeMess;
int dS;

// Define a struct to hold the parameters
struct SendingParams {
    char *ip;
    char *port;
};

void send_file(FILE *fp, int sockfd){
  int n;
  char data[FILE_SIZE] = {0};
 
  while(fgets(data, FILE_SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("Error in sending file.");
      exit(1);
    }
    bzero(data, FILE_SIZE);
  }
}


long getFileSize(FILE *file) {
    long size;

    fseek(file, 0L, SEEK_END); // Move the file pointer to the end of the file
    size = ftell(file); // Get the current position of the file pointer, which is the file size
    rewind(file); // Reset the file pointer to the beginning of the file

    return size;
}

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

void *sending(void *arg)
{
  while (1)
  {
    fgets(msg, BUFFER_SIZE, stdin);
    if (msg[strlen(msg) - 1] == '\n')
      msg[strlen(msg) - 1] = '\0';

    if(strncmp(msg,"/sendfile", strlen("/sendfile")) == 0){
      // Cast the argument to the appropriate struct type
      struct SendingParams *params = (struct SendingParams *)arg;


      char *folderPath = "/home/nathan/FAR-MessagingProject/Client/FileToSend";
      DIR *dir;
      struct dirent *entry;

      printf("Quelle fichier souhaitez vous envoyer ?\n");

      dir = opendir(folderPath);
      if (dir == NULL) {
          perror("Failed to open the folder.\n");
      }

      while ((entry = readdir(dir)) != NULL) {
          if (entry->d_type == DT_REG) {  // Only display regular files, excluding directories
              printf("%s\n", entry->d_name);
          }
      }

      // Reset the position of the directory stream pointer
      rewinddir(dir);

      //Récupération du nom du fichier à envoyer
      fgets(filename, BUFFER_SIZE, stdin);
      filename[strcspn(filename, "\n")] = '\0';  // Remove the newline character

      int find = 0;

      //Vérification que le fichier existe
      while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Only display regular files, excluding directories
            if (strcmp(entry->d_name, filename) == 0) {
                find = 1;
                break;
            }
        }
      }

      if (find == 0) {
          printf("Le fichier n'existe pas\n");
          exit(1);
      }   

      char *messSendFile = "/sendfile";
      int sizeMessSendFile = strlen(messSendFile) + 1;

      //Envoi la taille de la commande
      if (send(dS, &sizeMessSendFile, 4, 0) == -1)
      {
        printf("Erreur d'envoi\n");
        exit(1);
      }

      //Envoi de la commande
      int res = send(dS, messSendFile, sizeMessSendFile, 0);
      if(res == -1)
      {
        perror("Erreur d'envoi");
        exit(1);
      }
      else if (res == 0)
      {
        perror("Fin de la connexion");
        pthread_exit(0);
      }

      sleep(1);

      char *ip = "127.0.0.1";
      int port = 8080;
      int e;

      int sockfd;
      struct sockaddr_in server_addr;
      //Création d'une nouvelle socket pour l'envoi du fichier
      sockfd = socket(AF_INET, SOCK_STREAM, 0);

      if(sockfd < 0) {
        perror("Error in socket");
        exit(1);
      }

      server_addr.sin_family = AF_INET;
      server_addr.sin_port = port;
      server_addr.sin_addr.s_addr = inet_addr(ip);

      e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
      if(e == -1) {
        perror("[-]Error in socket");
        exit(1);
      }
      printf("[+]Connected to Server.\n");

      //Envoi la taille du nom du fichier
      int sizeFilename = strlen(filename) + 1;
      
      if (send(sockfd, &sizeFilename, 4, 0) == -1)
      {
        printf("Erreur d'envoi\n");
        exit(1);
      }

      //Envoi du nom du fichier
      if(send(sockfd, filename, sizeFilename, 0) == -1)
      {
        perror("Erreur d'envoi");
        exit(1);
      }
      else if (res == 0)
      {
        perror("Fin de la connexion");
        pthread_exit(0);
      }

      char result[100]; // Allocate enough memory to hold the concatenated string

      strcpy(result, folderPath); // Copy the folderPath
      strcat(result, "/"); // Concatenate a / character
      strcat(result, filename); // Concatenate the filename

      printf("[+]Sending file : %s\n", result);

      FILE *fp = fopen(result, "r");
      if (fp == NULL) {
        perror("Error in reading file.");
        exit(1);
      }

      printf("[+]File opened successfully.\n");

      //Envoi de la taille du fichier
      long sizeFile = getFileSize(fp) +1;
      int sizeSizeFile = sizeof(sizeFile);

      //Envoi de la taille de la taille du fichier
      if (send(sockfd, &sizeSizeFile, 4, 0) == -1)
      {
        printf("Erreur d'envoi\n");
        exit(1);
      }

      //Envoi de la taille du fichier
      if (send(sockfd, &sizeFile, sizeSizeFile, 0) == -1)
      {
        printf("Erreur d'envoi\n");
        exit(1);
      }

      char data[200] = {0};

      while(fgets(data, 200, fp) != NULL) {
        printf("%s", data);
        if (send(sockfd, data, strlen(data), 0) == -1) {
          perror("[-]Error in sending file.");
          exit(1);
        }
        bzero(data, sizeFile);
      }

      //Fermeture de la socket
      close(sockfd);
    }
   
  }
    
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

  // Create a struct instance and populate the parameters
  struct SendingParams params;
  params.ip = argv[1];
  params.port = argv[2];

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

  pthread_create(&thread[0], NULL, sending, (void *)&params);
  pthread_create(&thread[1], NULL, receiving, (void *)1);
  // fermeture des threads
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);

  // libération de la mémoire
  free(msg);
  free(msg2);

  shutdown(dS, 2);
}