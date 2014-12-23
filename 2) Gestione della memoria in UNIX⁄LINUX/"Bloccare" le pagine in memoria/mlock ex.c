#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/resource.h> 

int main(int argn, char* args[])
{
  printf("Posso bloccare al massimo %d pagine nella memoria\n", RLIMIT_MEMLOCK);
  
  char* mem = malloc(4096);
  
  //Blocca la pagina in memoria
  if( mlock(mem, 4096) == -1)
    printf("Impossibile bloccare la memoria: %s\n", strerror(errno));
  
  //Sblocco la memoria
  if( munlock(mem, 4096) == -1)
    printf("Impossibile sbloccare la memoria: %s\n", strerror(errno));
  
  return 0;
}
