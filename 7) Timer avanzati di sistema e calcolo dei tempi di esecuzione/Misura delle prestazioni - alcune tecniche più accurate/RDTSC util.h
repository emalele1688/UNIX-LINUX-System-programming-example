#include <stdint.h>
#include <time.h>


/* calcola la differenza tra i due tempi */
void getTimeDifferent(struct timespec* ts1, struct timespec* ts2, struct timespec* tst);

/* calibrazione del clock - torna il valore da passare a getRdtscTime*/
double calibrateTicks(void);

/* preleva il tempo da RDTSC */
uint64_t getRdtscTime(double calibrateVal);