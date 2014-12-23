#define _GNU_SOURCE
#include <sys/mman.h>
#include <sched.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include "RDTSC util.h"
 
#define MIC_SEC 1000000
#define NANO_SEC 1000000000
#define TEST 1000000

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

void childrenProccess(void)
{
  struct timeval ts1, ts2;
  gettimeofday(&ts1, NULL);
  int faiqualcosa = frequency_of_primes(TEST);
  gettimeofday(&ts2, NULL);
  
  long micSec = ((ts2.tv_sec - ts1.tv_sec)*MIC_SEC + (ts2.tv_usec-ts1.tv_usec));
  double sec = (double)micSec / MIC_SEC;
  
  printf("Tempo globale del children: %f\nTempo di processo del children preso da clock: %f\n", sec, ((float)clock())/CLOCKS_PER_SEC);
  printf("Children test terminato\n");
}

int main()
{
  //Il processo figlio creato esegue il test nel modo normale (senza priorità real time e con page fault probabili)
  pid_t children;
  children = fork();
  if(children == 0)
  {
    childrenProccess();
    return;
  }
  
  if( mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
    printf("Il kernel non può caricare in memoria tutte le pagine del processo. Possono verificarsi Page Fault\n%s\n", strerror(errno));
 
  struct sched_param prior;
  prior.sched_priority = 99;
  if(sched_setscheduler(0, SCHED_RR, &prior) == -1)
    printf("Impossibile schedulare il processo in real time\n%s\n", strerror(errno));
  
  cpu_set_t mask; /* processors 0, 1, and 2 */
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  if(sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1) 
    printf("Impossibile dedicare la CPU 0 al processo\n%s\n", strerror(errno));
  
  double calibr = calibrateTicks();

  struct timespec ts1, ts2, tsdiff;
  struct timeval tv1, tv2;
  
  //Inizializzo anche un clock di interval counting per il confronto
  clock_t ck_st = clock();
  //uint64_t nsec1 = getRdtscTime(calibr);
  gettimeofday(&tv1, NULL);
  int faiqualcosa = frequency_of_primes(TEST);
  gettimeofday(&tv2, NULL);
  //uint64_t nsec2 = getRdtscTime(calibr);
  clock_t ck_en = clock();
  
  /* prova a rieseguirlo */
  
  long micSec = ((tv2.tv_sec - tv1.tv_sec)*MIC_SEC + (tv2.tv_usec-tv1.tv_usec));
  double secmicro = (double)micSec / MIC_SEC;
  //double secnano = ((double)nsec2 - (double)nsec1) / NANO_SEC; 
  
  printf("Tempo del dad preso con gettimeofday: %f\nTempo di processo del dad preso da clock: %f\n", secmicro, ((float)(ck_en - ck_st))/CLOCKS_PER_SEC);
  //printf("Il timer impostato dal TSC (calibrato a %f) restituisce: %f\n", calibr, secnano);
  
  printf("Dad test terminato\n");
  wait(children);
  
  printf("Uscita\n");
  return 0;
}
