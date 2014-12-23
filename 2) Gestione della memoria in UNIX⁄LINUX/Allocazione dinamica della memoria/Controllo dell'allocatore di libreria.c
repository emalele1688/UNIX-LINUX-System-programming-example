#include <stdio.h>
#include <malloc.h>
#include <mcheck.h>
#include <string.h>
#include <errno.h>


void malloc_error(enum mcheck_status status)
{
  switch(status)
  {
    case MCHECK_OK:
      printf("Nessun problema riportato\n");
      break;
      
    case MCHECK_DISABLED:
      printf("Hai usato mcheck dopo aver chiamato malloc\n");
      break;
      
    case MCHECK_HEAD:
      printf("Header corruption\n");
      break;
      
    case MCHECK_TAIL:
      printf("Footer corruption\n");
      break;
      
    case MCHECK_FREE:
      printf("Il buffer è già stato deallocato\n");
      break;
      
    default:
      printf("Non sarà mai stampata (spero)\n");
  }
}


int main(int argn, char* args[])
{
  if(mcheck(malloc_error) == -1)
    printf("mcheck error\n");
  
  void* pbuf = malloc(64);
    
  ((int*)pbuf)[0] = 10;
  ((int*)pbuf)[8] = 4;
  
  //Header corruption
  *((char*)pbuf - 8) = 0;
  free(pbuf);
  
  printf("Success\n");
  return 0;
}
