#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/capability.h>



int main(int argn, char *argv[])
{
  int i = 0;
  
  printf("My pid %d\n", getpid());
    
  while(1)
  {
    printf("Counter = %d\n", i * 2);
    sleep(2);
    i++;
  }
  
  return 0;
}
