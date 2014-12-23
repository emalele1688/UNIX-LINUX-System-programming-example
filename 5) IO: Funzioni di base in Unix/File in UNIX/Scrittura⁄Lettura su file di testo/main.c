#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define BUFFSIZE 32

int main(int arg, char *args)
{
  int pfile = open("File.txt", O_CREAT|O_RDWR|O_APPEND, 0660);
  
  if(pfile == -1)
  {
    printf("Impossibile creare/aprire un file\n");
    exit(0);
  }
  
  char message[BUFFSIZE];
  printf("Inserisci le stringhe da inserire nel file, dai quit per terminare\n");
  do
  {
    memset(message, '\0', BUFFSIZE);
    fgets(message, BUFFSIZE, stdin);
    //Scriviamo nel file il contenuto di message
    write(pfile, message, strlen(message));
  }
  while(strncmp(message, "quit", strlen(message)-1) != 0);
  
  printf("Le stringhe registrate nel file sono:\n");
  
  //Azzeriamo messagge per correttezza
  memset(message, '\0', BUFFSIZE);
  
  //lseek in SEEK_END restituisce il numero di caratteri trovati dall'inizio alla fine
  printf("Numero di stringhe: %ld\n", lseek(pfile, 0, SEEK_END));
  
  //File si trova alla fine del file, lo posiziono all'inizio
  lseek(pfile, 0, SEEK_SET);
  int size = 0;
  do
  {
    //Read restituisce la dimensione del file letto. 0 se non legge nulla
    size = read(pfile, message, BUFFSIZE);
    
    message[strlen(message)-1] = '\0';
    printf("%s\n", message);
    printf("Letti byte: %d\n", size);
  }
  while(size > 0);
  
  return 0;
}
