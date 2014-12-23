#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


int main(int argn, char *argv[])
{
  printf("User ID reale %zu\n", getuid());
  printf("Group ID reale %zu\n", getgid());
  printf("User ID effettivo %zu\n", geteuid());
  printf("Group ID effettivo %zu\n", getegid());
  
  printf("\nAbilito i privileggi di amministratore\n");
  if(setuid(0) == -1)
    printf("%s\n", strerror(errno));
  
  printf("\nUser ID reale %zu\n", getuid());
  printf("Group ID reale %zu\n", getgid());
  printf("User ID effettivo %zu\n", geteuid());
  printf("Group ID effettivo %zu\n", getegid());
  
  return 0;
}
