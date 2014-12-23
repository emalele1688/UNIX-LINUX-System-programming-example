#include <stdio.h>
#include <math.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <unistd.h>

/* times torna un valore diverso da clock_gettime(REAL o MONOTONIC... ) e gettimeofday 
 * Dalla struttura dati torna il valore usr e sys preso con la tecnica dell'interval counting
 */

//Dato n, calcola da 0 a n quanti numeri primi sono presenti
int frequency_of_primes (int n) 
{
  int i,j;
  int freq=n-1;
  for(i=2; i<=n; ++i) 
  {
    for(j=sqrt(i); j>1; --j) 
      if(i%j ==0) 
      {
	--freq; 
	break;
      }
  }
  return freq;
}

int main ()
{
  struct tms st_cpu;
  struct tms en_cpu;
  clock_t st_time;
  clock_t en_time;
  
  //Chiedo al sistema di dirmi a quanto è calibrato il clock usato da times
  const int TIMES_CLOCK_TICK = sysconf(_SC_CLK_TCK);
  printf("times è calibrato a: %ld\n", TIMES_CLOCK_TICK);
  
  st_time = times(&st_cpu);
  printf("Times preso a tempo: %f\n", ((float)st_time/TIMES_CLOCK_TICK));
  
  //Perdo tempo
  int f = frequency_of_primes(1000000);
  
  en_time = times(&en_cpu);
  
  printf("Tempo di calcolo per la funzione frequency_of_primes:\nUser: %lf\nSys: %lf\nReal: %lf (Non si blocca quando il processo è in prelazione)\n",
	 (float)(en_cpu.tms_utime - st_cpu.tms_utime)/TIMES_CLOCK_TICK, (float)(en_cpu.tms_stime - st_cpu.tms_stime)/TIMES_CLOCK_TICK, (float)(en_time - st_time)/TIMES_CLOCK_TICK);
 
  return 0;
} 
