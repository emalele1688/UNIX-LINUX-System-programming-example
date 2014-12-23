#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char* args[])
{
  int ds_file;
  char* addr;
  size_t lenght;
  struct stat st;
  
  ds_file = open("file.txt", O_RDONLY);
  if(ds_file == -1)
  {
    printf("Il file file.txt non esiste\n");
    return -1;
  }
  
  if(fstat(ds_file, &st) == -1)
  {
    printf("Errore nel leggere il file\n");
    return -1;
  }

  lenght = st.st_size;
  
  addr = mmap(NULL, lenght, PROT_READ, MAP_PRIVATE, ds_file, 0);
  
  if(addr == MAP_FAILED)
  {
    printf("Problemi nel mappare il file\n");
    return -1;
  }
  
  printf("File mappato all'indirizzo %u\n", addr);
  
  //Stampa tutto il file
  printf("%s", addr);
  
  if(munmap(addr, lenght) == -1)
    printf("Impossibile smappare il file\n");
  
  return 0;
}
