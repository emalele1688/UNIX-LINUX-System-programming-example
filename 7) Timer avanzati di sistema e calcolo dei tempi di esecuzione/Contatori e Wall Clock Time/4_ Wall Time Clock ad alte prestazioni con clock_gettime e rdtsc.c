#include <stdint.h>
#include <time.h>
#include <stdio.h>

#define NANO_SECOND 1000000000

//Legge il tempo da TSC
uint64_t RDTSC(void)
{
  unsigned int hi, lo;
  //Catturo il tempo da rdtsc nei registri eax edx
  asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
  //In un intero a 64 bit senza segno inserisco nella parte più significativa hi, e meno significativa lo
  return ((uint64_t)hi << 32) | lo;
}

/* calcola la differenza tra i due tempo */
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

/* calibrazione del clock */
double calibrateTicks(void)
{
  struct timespec begin_ts, end_ts, tmpts;
  uint64_t begin = 0, end = 0, elapsed, i = 0;
  
  clock_gettime(CLOCK_MONOTONIC, &begin_ts);
  begin = RDTSC();
  
  //Attendo un secondo (frequenza = colpi_di_clock_in_s_secondi / s_secondi
  sleep(1);
  
  end = RDTSC();
  clock_gettime(CLOCK_MONOTONIC, &end_ts);
  
  getTimeDifferent(&begin_ts, &end_ts, &tmpts);
  
  /* Trasformo tmpts in nanosocondo poichè end e begin sono in nanosocondi
   * Dunque ricavo elapsed che è il tempo passato tra le due chiamate RDTSC (approsimatamente un secondo di sleep)
   */
  elapsed = tmpts.tv_sec * NANO_SECOND + tmpts.tv_nsec;
  
  printf("Frequenza del Time Stamp Counter (approsimatamente): %lu\n", (end - begin));
  
  //Colpi di clock del'TSC in elapsed secondi / elapsed secondi = frequenza del TSC
  return (double)(((double)end - (double)begin) / (double)elapsed);
}

/* preleva il tempo da RDTSC */
void getRdtscTime(struct timespec* ts, double calibrateVal)
{
  uint64_t usectime = RDTSC() / calibrateVal;
  
  ts->tv_sec = usectime / NANO_SECOND;
  ts->tv_nsec = usectime % NANO_SECOND;
}


int main()
{
  struct timespec ts1, ts2, ts;
  clock_t tps, tpe;
  double cal = calibrateTicks();
  
  printf("Frequenza del Time Stamp Counter (più precisa): %lf\n", cal);
  getRdtscTime(&ts1, cal);
 while(1){ 
  
  sleep(1);
  getRdtscTime(&ts2, cal);
  
  getTimeDifferent(&ts1, &ts2, &ts);
  
  printf("Tempo globale %lu.%lu\n", ts.tv_sec, ts.tv_nsec);
}
  return 0;
}