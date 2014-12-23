#include <stdio.h>
#include <time.h>

#define NANO_SECOND 1000000000

/* clock_gettime => 
 * CLOCK_REALTIME: Torna il tempo in secondi e microsecondi passati "Dall'inizio dell'epoca" CORRISPONDE A gettimeofday .. STESSO VALORE
 * CLOCK_MONOTONIC: Torna il tempo in secondi e microsecondi trascorso dal boot
 * CLOCK_PROCESS_CPUTIME_ID: Tempo trascorso dallo start del processo o thread in caso di CLOCK_THREAD_CPUTIME_ID. (Non è usato in questa dimostrazione)
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
  struct timespec real_start, real_end, real;
  struct timespec mtoc_start, mtoc_end, mtoc;
  
  clock_gettime(CLOCK_REALTIME, &real_start);
  sleep(1);
  clock_gettime(CLOCK_REALTIME, &real_end);
  
  getTimeDifferent(&real_start, &real_end, &real);
  
  printf("Misurando 1 secondo con REALTIME:  %ld.%ld\n", real.tv_sec, real.tv_nsec);
  
  clock_gettime(CLOCK_MONOTONIC, &mtoc_start);
  sleep(1);
  clock_gettime(CLOCK_MONOTONIC, &mtoc_end);
  
  getTimeDifferent(&mtoc_start, &mtoc_end, &mtoc);
  
  printf("Misurando 1 secondo con MONOTONIC: %ld.%ld\n", mtoc.tv_sec, mtoc.tv_nsec);
  
  clock_gettime(CLOCK_REALTIME, &real);
  printf("CLOCK_REALTIME: %ld.%ld\n", real.tv_sec, real.tv_nsec);
  clock_gettime(CLOCK_MONOTONIC, &mtoc);
  printf("CLOCK_MONOTONIC: %ld.%ld\n", mtoc.tv_sec, mtoc.tv_nsec);
  clock_gettime(CLOCK_REALTIME, &real);
  printf("CLOCK_REALTIME: %ld.%ld\n", real.tv_sec, real.tv_nsec);
  clock_gettime(CLOCK_MONOTONIC, &mtoc);
  printf("CLOCK_MONOTONIC: %ld.%ld\n", mtoc.tv_sec, mtoc.tv_nsec);
  clock_gettime(CLOCK_REALTIME, &real);
  printf("CLOCK_REALTIME: %ld.%ld\n", real.tv_sec, real.tv_nsec);
  clock_gettime(CLOCK_MONOTONIC, &mtoc);
  printf("CLOCK_MONOTONIC: %ld.%ld\n", mtoc.tv_sec, mtoc.tv_nsec);
  
  /* Si rammenta di usare il clock MONOTONIC per il calcolo del Wall time clock */
  
  return 0;
}
