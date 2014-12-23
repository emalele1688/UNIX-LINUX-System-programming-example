#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>


int main(int argn, char* args[])
{
  char* mem = malloc(4096);
  
  //Blocca tutte le pagine in memoria esistenti e che saranno allocate in futuro
  if( mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
    printf("Impossibile bloccare la memoria: %s\n", strerror(errno));
  
  //Sblocco la memoria
  if( munlockall() == -1)
    printf("Impossibile sbloccare la memoria: %s\n", strerror(errno));
  
  return 0;
}
 
