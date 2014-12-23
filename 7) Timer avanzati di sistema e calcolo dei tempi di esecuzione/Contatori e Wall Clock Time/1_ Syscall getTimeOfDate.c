#include <stdio.h>
#include <sys/time.h>

/* La SYSCALL gettimeofday restituisce il tempo in secondi.microsecondi trascorso dal 1970 alla data attuale 
 * E' utile per misurare il Wall Clock Time = Tempo passato dall'inizio alla fine del processo (compreso il tempo in cui il processo è in attesa di CPU)
 * Oppure per conoscere la data attuale
 */

int main()
{
  struct timeval tm;
  //Secondo parametro deve essere NULL poichè la timezone è stata deprecata
  while(1)
  {
    gettimeofday(&tm, NULL);
    printf("Secondi: %lu,%lu\n", tm.tv_sec, tm.tv_usec);
    sleep(1);
  }
  return 0;
}
