#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *file;

int main(int arg, char *args[])
{
  file = fopen("Testo.txt", "w+");
  
  if(file == 0)
  {
    printf("Impossibile aprire il file\n");
    exit(1);
  }
  
  char message[256];
  memset(message, '\0', 256);
  printf("Inserire la stringa da mettere nel file, quit per uscire\n");
  while(1)
  {
    //'\0' viene inserito in automatico alla fine prima o nel 256-esimo byte
    fgets(message, 256, stdin);
    
    if(strncmp(message, "quit", strlen(message)-1) == 0)
      break;
    
    //Scrivo il message sul file
    fputs(message, file);
  }
  
  //File si trova alla fine del file, lo posiziono all'inizio
  fseek(file, 0, SEEK_SET);
  //Prelevo nel file e memorizzo in message riga a riga (per un max di 256 byte a riga, torna 0 se il file si trova alla fine (riga vuota)
  while(fgets(message, 256, file) != NULL)
    fputs(message, stdout);
  
  fclose(file);
  
  return 0;
}
