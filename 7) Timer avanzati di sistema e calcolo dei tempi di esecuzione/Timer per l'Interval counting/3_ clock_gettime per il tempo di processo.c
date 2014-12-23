#include <stdio.h>
#include <time.h>

#define NANO_SECOND 1000000000

/* clock_gettime => 
 * CLOCK_REALTIME/CLOCK_MONOTONIC: Guarda in "Contatori di sistema per il wall clock time" (non è usato nell'interval counting)
 * CLOCK_PROCESS_CPUTIME_ID: Tempo trascorso dallo start del processo
 */

/* calcola la differenza tra i due tempi */
void getTimeDifferent(struct timespec* ts1, struct timespec* ts2, struct timespec* tst)
{
  tst->tv_sec = ts2->tv_sec - ts1->tv_sec;
  tst->tv_nsec = ts2->tv_nsec - ts1->tv_nsec;
  
  if(tst->tv_nsec < 0)
  {
    tst->tv_sec--;
    tst->tv_nsec += NANO_SECOND; //tst.tv_nsec è negativo, sommo un secondo ottenendo il nanosocondo
  }
}

int main(void)
{
  int i;
  struct timespec proc_start, proc_end, proc;  
  
  //Test 1
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_start);
  printf("Ciclo per occupare CPU\n");
  for(i = 0; i < 1000000000; i++);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_end);
  getTimeDifferent(&proc_start, &proc_end, &proc);
  printf("Tempo consumato dal ciclo for:  %ld.%ld\n", proc.tv_sec, proc.tv_nsec);
  
  //Test 2
  printf("La misura riguarda solo il tempo di CPU del processo, infatti se entro in sleep per 2 secondi\n");
  sleep(2);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &proc_end);
  getTimeDifferent(&proc_start, &proc_end, &proc);
  printf("Il tempo consumato dal processo non è conteggiato durante lo sleep: %ld.%ld\n", proc.tv_sec, proc.tv_nsec);
  
  return 0;
}
 
