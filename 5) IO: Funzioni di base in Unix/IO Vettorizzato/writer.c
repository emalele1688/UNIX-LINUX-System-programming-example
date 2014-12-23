#include <stdio.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>
#include <string.h>

struct Entry
{
  int par;
  char nome[20];
  char cognome[20];
};

int main(int argn, char* args)
{
  int ds = open("file.bin", O_CREAT|O_RDWR|O_APPEND, 0660);
  if(ds == -1)
    return -1;
  
  int occorenze;
  printf("Quante persono vuoi inserire?\n");
  scanf("%d", &occorenze);
  struct Entry pr[occorenze];
  struct iovec io[occorenze];
  int cont = 0;
  while(cont < occorenze)
  {
    printf("Parametro intero\n");
    scanf("%d", &(pr[cont].par));
    getchar();
    printf("Nome\n");
    fgets(pr[cont].nome, 20, stdin);
    printf("Cognome\n");
    fgets(pr[cont].cognome, 20, stdin);
    
    io[cont].iov_base = &pr[cont];
    io[cont].iov_len = sizeof(struct Entry);
    cont++;
  }
  
  if(writev(ds, io, occorenze) == -1)
    printf("%s\n", strerror(errno));
  
  close(ds);
  
}
