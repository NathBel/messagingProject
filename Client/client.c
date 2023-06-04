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
int n;
int sizeMess;
int dS;

// Define a struct to hold the parameters
struct SendingParams
{
  char *ip;
  char *port;
};

void send_file(FILE *fp, int sockfd)
{
  int n;
  char data[FILE_SIZE] = {0};

  while (fgets(data, FILE_SIZE, fp) != NULL)
  {
    if (send(sockfd, data, sizeof(data), 0) == -1)
    {
      perror("Error in sending file.");
      exit(1);
    }
    bzero(data, FILE_SIZE);
  }
}

long getFileSize(FILE *file)
{
  long size;

  fseek(file, 0L, SEEK_END); // Move the file pointer to the end of the file
  size = ftell(file);        // Get the current position of the file pointer, which is the file size
  rewind(file);              // Reset the file pointer to the beginning of the file

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
    char *badWords[12] = {"merde", "con", "pute", "connasse", "connard", "salope", "enculé", "encule", "enculer", "pd", "pédé", "Thomas Godel n'est pas le chef"};
    //Check if msg contains bad words
    for (int i = 0; i < 12; i++)
    {
      if (strstr(msg, badWords[i]) != NULL)
      {
        printf("\x1b[31m%s\x1b[0m\n", "Message censuré\n");
        return;
      }
    }

    printf("\x1b[34m \t \t \t %s\x1b[0m\n", msg);
  }
}

void receiveFile()
{

  char *ip = "127.0.0.1";
  int port = 8080;
  int e;

  int sockfd, new_sock;
  struct sockaddr_in server_addr, new_addr;
  socklen_t addr_size;
  char buffer[200];

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    perror("[-]Error in socket");
    exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  e = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (e < 0)
  {
    perror("[-]Error in bind");
    exit(1);
  }

  if (listen(sockfd, 10) != 0)
  {
    perror("[-]Error in listening");
    exit(1);
  }

  addr_size = sizeof(new_addr);
  new_sock = accept(sockfd, (struct sockaddr *)&new_addr, &addr_size);

  // Reception taille du nom du fichier
  int res = recv(new_sock, &sizeMess, sizeof(sizeMess), 0);

  // Reception du nom du fichier
  char filename[sizeMess];
  res = recv(new_sock, filename, sizeMess, 0);

  // Reception taille de la taille du fichier
  int sizeSizefile;
  res = recv(new_sock, &sizeSizefile, sizeof(sizeSizefile), 0);


  // Reception taille du fichier
  int sizefile;
  res = recv(new_sock, &sizefile, sizeSizefile, 0);

  int n;
  FILE *fp;

  char result[100]; // Allocate enough memory to hold the concatenated string

  strcpy(result, "/home/nathan/FAR-MessagingProject/Client/File/"); // Copy the folderPath
  strcat(result, filename);                                               // Concatenate the filename

  fp = fopen(result, "wb");
  if (fp == NULL)
  {
    perror("Error in reading file.");
    exit(1);
  }

  while (1)
  {
    n = recv(new_sock, buffer, 200, 0);
    if (n <= 0)
    {
      if (n == 0)
      {
        // Fermeture de la socket
        close(sockfd);
      }
      else
      {
        perror("[-]Error in receiving file.");
      }
      break;
    }

    size_t bytesWritten = fwrite(buffer, sizeof(char), n, fp);
    if (bytesWritten != n)
    {
      perror("[-]Error in writing file.");
      break;
    }

    bzero(buffer, 200);
  }
  fclose(fp);
}

void sendfile()
{
  char *folderPath = "/home/nathan/FAR-MessagingProject/Client/File";
  DIR *dir;
  struct dirent *entry;

  printf("\x1b[31m Quel fichier souhaitez vous envoyer (entrez le numéro) ?\x1b[0m\n");

  dir = opendir(folderPath);
  if (dir == NULL)
  {
    perror("Failed to open the folder.\n");
  }

  int i = 1;
  char *filenumber = malloc(BUFFER_SIZE * sizeof(char));

  // Max 100 fichiers
  struct dirent files[100];

  while ((entry = readdir(dir)) != NULL)
  {
    if (entry->d_type == DT_REG)
    { // Only display regular files, excluding directories
      printf("\x1b[31m %d. %s\x1b[0m\n", i, entry->d_name);
      files[i - 1] = *entry;
      i++;
    }
  }
  // Reset the position of the directory stream pointer
  rewinddir(dir);

    // Récupération du nom du fichier à envoyer
    fgets(filenumber, BUFFER_SIZE, stdin);
    filenumber[strcspn(filenumber, "\n")] = '\0'; // Remove the newline character

    int index = atoi(filenumber) - 1;

    char *filename = strdup(files[index].d_name);
    if (filename == NULL)
    {
      printf("Failed to allocate memory\n");
      // Handle memory allocation failure here
    }

    free(filenumber);

    int find = 0;

    // Vérification que le fichier existe
    while ((entry = readdir(dir)) != NULL)
    {
      if (entry->d_type == DT_REG)
      { // Only display regular files, excluding directories
        if (strcmp(entry->d_name, filename) == 0)
        {
          find = 1;
          break;
        }
      }
    }

    if (find == 0)
    {
      printf("Le fichier n'existe pas\n");
      return;
    }

  char *messSendFile = "/sendfile";
  int sizeMessSendFile = strlen(messSendFile) + 1;

  // Envoi la taille de la commande
  if (send(dS, &sizeMessSendFile, 4, 0) == -1)
  {
    printf("Erreur d'envoi\n");
    exit(1);
  }

  // Envoi de la commande
  int res = send(dS, messSendFile, sizeMessSendFile, 0);
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

  sleep(1);

  char *ip = "127.0.0.1";
  int port = 8080;
  int e;

  int sockfd;
  struct sockaddr_in server_addr;
  // Création d'une nouvelle socket pour l'envoi du fichier
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    perror("Error in socket");
    exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  e = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (e == -1)
  {
    perror("[-]Error in socket");
    exit(1);
  }

  // Envoi la taille du nom du fichier
  int sizeFilename = strlen(filename) + 1;

  if (send(sockfd, &sizeFilename, 4, 0) == -1)
  {
    printf("Erreur d'envoi\n");
    exit(1);
  }

  // Envoi du nom du fichier
  if (send(sockfd, filename, sizeFilename, 0) == -1)
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
  strcat(result, "/");        // Concatenate a / character
  strcat(result, filename);   // Concatenate the filename

  printf("\x1b[31m Envoi du fichier \x1b[0m\n");

  FILE *fp = fopen(result, "rb");
  if (fp == NULL)
  {
    perror("Error in reading file.");
    exit(1);
  }

  // Envoi de la taille du fichier
  long sizeFile = getFileSize(fp) + 1;
  int sizeSizeFile = sizeof(sizeFile);

  // Envoi de la taille de la taille du fichier
  if (send(sockfd, &sizeSizeFile, 4, 0) == -1)
  {
    printf("Erreur d'envoi\n");
    exit(1);
  }

  // Envoi de la taille du fichier
  if (send(sockfd, &sizeFile, sizeSizeFile, 0) == -1)
  {
    printf("Erreur d'envoi\n");
    exit(1);
  }

  void *data = malloc(200); // Allocate memory for data

  // fseek(fp, 0, SEEK_END);  // Move file pointer to the end of the file
  // long size = ftell(fp);   // Get the current position of the file pointer (size of the file)
  // fseek(fp, 0, SEEK_SET);  // Move file pointer back to the beginning of the file

  while (1)
  {
    size_t bytesRead = fread(data, sizeof(char), 200, fp);
    if (bytesRead == 0)
    {
      // shutdown socket
      shutdown(sockfd, 2);
      break; // Reached end of file, exit the loop
    }

    if (send(sockfd, data, bytesRead, 0) == -1)
    {
      perror("[-]Error in sending file.");
      exit(1);
    }

    bzero(data, 200);
  }

  free(data); // Desallocated memory
  fclose(fp); // Close the file

  // Fermeture de la socket
  close(sockfd);
}

void *sending(void *arg)
{
  while (1)
  {
    fgets(msg, BUFFER_SIZE, stdin);
    if (msg[strlen(msg) - 1] == '\n')
      msg[strlen(msg) - 1] = '\0';

    if (strncmp(msg, "/sendfile", strlen("/sendfile")) == 0)
    {
      sendfile();
    }
    else
    {
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
      if (strcmp(msg, "Server: /download") == 0)
      {
        // Reception du nombre de fichiers dans FileToSend
        int nbFiles;
        res = recv(dS, &nbFiles, 4, 0);
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

        printf("\x1b[31m Quel fichier souhaitez vous télécharger (entrez le numéro) ?\x1b[0m\n");

        // Reception des noms des fichiers
        char *filenames[nbFiles];
        char *filenumber = malloc(BUFFER_SIZE * sizeof(char));

        for (int i = 0; i < nbFiles; i++)
        {
          // Reception de la taille du nom du fichier
          int sizeFilename;
          res = recv(dS, &sizeFilename, 4, 0);
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

          // Reception du nom du fichier
          filenames[i] = malloc(sizeFilename * sizeof(char));
          res = recv(dS, filenames[i], sizeFilename, 0);
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

          printf("\x1b[31m %d. %s\x1b[0m\n", i + 1, filenames[i]);
        }

        // Désallocation mémoire de filenames
        for (int i = 0; i < nbFiles; i++)
        {
          free(filenames[i]);
        }

        receiveFile();
        printf("\x1b[31m Fichier téléchargé \x1b[0m\n");
      }
      else
      {
        // Formatage & affichage du message reçu
        formattingMessage(msg);
      }
    }
  }
  pthread_exit(0);
}

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
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