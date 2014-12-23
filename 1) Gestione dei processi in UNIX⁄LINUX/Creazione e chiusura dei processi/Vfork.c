#include <stdio.h>

int main(int *argc, char **argv)
{
  int pid = 0;
  
  //vfork attende la terminazione del figlio (chiamata bloccante)
  if((pid = vfork()) == -1)
  {
    printf("Errore nel fork del processo\n");
    return -1;
  }

  if(pid == 0)
  {
    printf("Sono il figlio, ho voglia di dormire\n");
    sleep(5);
    exit(0);
  }
  
  //Passo qui solo quando il processo figlio ha terminato
  printf("Sono main %d, mio figlio ha pid %d. BYE BYE!\n", getpid(), pid);
  return 0;
}
