#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define BUFSIZE 1024

int main(int arg, char *argv[])
{
  int psorg, pdest, result, size;
  char buffer[BUFSIZE];
  
  if(arg < 3)
  {
    printf("Usare: sorgente destinazione\n");
    exit(1);
  }
  
  psorg = open(argv[1], O_RDONLY);
  if(psorg == -1)
  {
    printf("Impossibile trovare una sorgente\n");
    exit(1);
  }
  
  pdest = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0660);
  if(pdest == -1)
  {
    printf("Impossibile trovare una destinazione\n");
    exit(1);
  }
  
  do
  {
    size = read(psorg, buffer, BUFSIZE);
    if(size == -1)
    {
      printf("Impossibile leggere la sorgente\n");
      exit(1);
    }
    
    if( write(pdest, buffer, size) == -1 )
    {
      printf("Impossibile scrivere sulla directory specificata, è protetta da scrittura?\n");
      exit(1);
    }
  }
  while(size > 0);
  
  if(argv[3] != '\0' && strcmp(argv[3], "-r") == 0)
    remove(argv[1]);
  
  close(psorg);
  close(pdest);
}
