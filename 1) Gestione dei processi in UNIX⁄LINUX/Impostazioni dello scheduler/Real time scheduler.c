#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define MOLTIPLICATOR 1000


void sort(int v[], unsigned long int dim)
{
  unsigned long int i = 0, j = 0;
    
  for(i = 0; i < dim-1; i++)
  {
    for(j = i+1; j < dim; j++)
    {
      if(v[i] < v[j])
      {
	int temp = v[i];
	v[i] = v[j];
	v[j] = temp;
      }
    }
  }
}


int main(int argn, char* args[])
{
  if(argn != 4)
  {
    printf("Usage: priority [policy: 1=FIFO, 2=RR] [priority=1-99] [Input size (es: 100)]\n");
    return 0;
  }
  
  int policy = 0;
  if(atol(args[1]) == 1)
  { 
    policy = SCHED_FIFO;
    printf("Scheduler FIFO\n");
  }
  else
  {
    policy = SCHED_RR;
    printf("Scheduler ROUND ROBIN\n");
  }
  
  /* Imposto la politica di scheduler: sched_setscheduler(pid, policy, real_time priority (nullo se la politica non è RoundRobin o FIFO))
   * FIFO: Il processo viene eseguito e sarà interrotto solo se fa richiesta di I/O, termina, fa richiesta esplicità di interruzione, oppure viene interrotto da un processo FIFO con priorità più alta.
   * Se il processo viene interrotto da uno con priorità più alta allora resta in cima alla lista e sarà il prossimo ad essere eseguito. Se invece si blocca volontariamente (I/O o con sched_yield) allora è posto in coda la lista.
   * Se la priorità di un processo FIFO è 99 (massima) allora sarà visto come processo Real Time dal sistema.
   * RoundRobin: Come FIFO ma a ciascun processo è assegnato un time-slice
   * Sia la politica FIFO che RoundRobin hanno priorità da 1 (bassa) a 99 (alta).
   */
  struct sched_param prior;
  prior.sched_priority = atol(args[2]);
  if( sched_setscheduler(0, policy, &prior) == -1)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  
  unsigned long int arraysize = MOLTIPLICATOR * atol(args[3]);
  printf("%ld elementi interi nell'array\n", arraysize);
  int* vect = malloc(sizeof(int)*arraysize);
  
  srand(time(NULL));
  
  unsigned long int i = 0;
  for(i = 0; i < arraysize; i++)
    vect[i] = rand() % 1000;
  
  sort(vect, arraysize);
    
  return 0;
}