#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main(int narg, char *args[])
{
  int pfile = 0;
  
  //Creo un file Test.txt e lo aprto in lettura/scrittura, se esiste viene "troncato" a 0 (ricreato)----------------
  pfile = open("Test.txt", O_CREAT|O_RDWR|O_TRUNC, 0660);
  
  char scelta[32];
  do
  {
    printf("Inserisci:\n\t quit per uscire\n\t insert per inserire parole\n\t read per leggere il file\n");
    fgets(scelta, 32, stdin);
    scelta[strlen(scelta)-1] = '\0';
    
    if(strcmp(scelta, "insert") == 0)
    {      
      char message[256];
      printf("Inserisci il messaggio da scrivere nel file\n");
      fgets(message, 256, stdin);
      
      //Scrivo message in pfile ------------------------------------------------------------------------------------
      if(write(pfile, message, strlen(message)) == -1)
	printf("Impossibile scrivere sul file\n");
    }
    
    if(strcmp(scelta, "read") == 0)
    {
      //Leggo il file in pfile e metto fino a 256 byte in message; Sarà ritornata la dimensione del file
      char message[256];
      int size = read(pfile, message, 256);
      if(size == -1)
      {
	printf("Impossibile leggere il file\n");
	continue;
      }
      
      printf("Il contenuto letto è: %s il file ha dimensione %d\n", message, size);
    }
    
  } 
  while(strcmp(scelta, "quit") != 0);
  
  if(close(pfile) == -1)
    printf("Impossibile chiudere il file\n");
}
