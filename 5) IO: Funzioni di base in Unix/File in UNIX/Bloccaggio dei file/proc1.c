#include <sys/file.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define FILENAME "output.txt"
#define MEGSIZE 256

int main(int argn, char* args[])
{
  
  int fd = open(FILENAME, O_CREAT|O_RDWR, 0660);
  if(fd == -1)
  {
    printf("Impossibile aprire il file\n");
    exit(1);
  }
  
  //ACQUISENDO IL LOCK SU QUESTO PROCESSO, UN ULTERIORE PROCESSO DI QUESTO PROGRAMMA CHE DOVRA' SCRIVERE O LEGGERE SUL FILE DI FD ATTENDERA'
  //USANDO LOCK_SH SARANNO BLOCCATI I PROCESSI IN SCRITTURA AFFINCHE' VI SARANNO PROCESSI CHE LEGGERANNO SUL FILE
  if( flock(fd, LOCK_ex) == -1)
  {
    printf("Impossibile acquisire il lock per il file %s" FILENAME);
    exit(1);
  }
  
  char message[MEGSIZE];
  printf("Lock acquisito, scrivi nel file\n");
  char* idtes = "Processo 1\n";
  write(fd, idtes, strlen(idtes));
  do
  {
    memset(message, 0x0, MEGSIZE);
    fgets(message, MEGSIZE, stdin);
    write(fd, message, strlen(message));
  }
  while(strncmp(message, "quit", 4) != 0);
  
  if( flock(fd, LOCK_UN) == -1)
    printf("Problemi nel rilascio del file\n");
  
  close(fd);
  
  return 0;
}
