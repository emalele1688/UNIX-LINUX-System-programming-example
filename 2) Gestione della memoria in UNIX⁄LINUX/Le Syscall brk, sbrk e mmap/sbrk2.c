#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

unsigned int pstop;
unsigned long int pstart;

void gestione_sigfault(void)
{
  printf("La memoria ha ricevuto la sigfault accedendo all'indirizzo %u\n", pstop);
  printf("Dunque l'heap allocato era da %u byte", pstop - pstart);
  exit(0);
}

int main(int argc, char *args[])
{
  signal(SIGSEGV, gestione_sigfault);
  
  //startbreak punta all'inizio del heap
  void* startbreak = sbrk(0);
  printf("L'heap inizia da: %u (ancora non allocato)\n", startbreak);
  pstart = (unsigned long int)startbreak;
  
  //(*startbreak) = 100 => segmentation fault poichè non è allocata
  
  size_t heapsize;
  printf("Quanto allochi? Alloca possibilmente un multiplo di sizeof(int). \n(ricorda che il tuo valore sarà alineato con il PAGE_SIZE)\n");
  scanf("%u", &heapsize);
  
  printf("Allocazione della memoria da %u byte\n", heapsize);
  brk(startbreak + heapsize);
  printf("Memoria allocata, ora ci puoi scrivere dentro\n");
  
  int i, s;
  int* curr = startbreak;
  printf("Inserisci %d elementi int\n", heapsize/sizeof(int));
  for(i = 0; i < heapsize/sizeof(int); i++)
  {
    scanf("%d", &s);
    *(curr) = s;
    curr++;
  }
  
  curr = startbreak;
  
  for(i = 0; i < heapsize/sizeof(int); i++)
  {
    printf("curr = %d\n", *(curr));
    curr++;
  }

  int* ch = malloc(32);
  printf("La memoria allocata da 32 byte malloc parte da %u\nNota: inizia dopo lo startbreak + heapsize \n", ch);
  
  printf("Vedo fino a quando ho una SIGFAULT partendo da startbreak\n");
  
  while(1)
  {
    *((int*)startbreak) = 0;
    startbreak++;
    pstop = (unsigned int)startbreak;
  }
  
  return 0;
}
