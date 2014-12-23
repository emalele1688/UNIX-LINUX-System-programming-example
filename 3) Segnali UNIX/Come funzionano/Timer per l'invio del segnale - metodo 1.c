#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

struct itimerval value, oldvalue;
int CICLO;

void printhelp()
{
  printf("-real\treal time alarm\n");
  printf("-virtual\tal termine del tempo CPU in usrspace\n");
  printf("-profiling\tal termine del tempo CPU in usrspace + kernel space\n");
}

void printResult()
{
  printf("Timer scaduto\n");
  printf("Il tempo è scaduto; passati %u,%u \n", oldvalue.it_value.tv_sec, oldvalue.it_value.tv_usec);
  //Non vale per timer Real Time, esce dalla condizione di ciclo
  CICLO = 0;
}

//Abilita un timer tipo alarm usando la syscall setitimer
void realTimer()
{
  int sec = 0;
  unsigned long int usec = 0;
  
  printf("secondi:\n");
  scanf("%d", &sec);
  printf("microsecondi:\n");
  scanf("%u", &usec);
  
  printf("Abilitazione timer\n");
  
  //Decrementerà da it_value a 0
  value.it_value.tv_usec = usec;
  value.it_value.tv_sec = sec;
  //Dopo il segnale it_value sarà reinpostato a 0
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = 0;
  
  signal(SIGALRM, printResult);
  
  //Imposta un timer real time (alarm) che parte da value e salva il tempo in oldvalue al lancio del segnale
  if( setitimer(ITIMER_REAL, &value, &oldvalue) == -1 )
  {
    printf("Errore nel settare il timer\n");
    exit(1);
  }
  
  printf("Attendiamo il segnale\n");
  
  pause();
}

//Abilita un timer che conta solo dal tempo di esecuzione in USRSPACE
void virtualTimer()
{
  int sec = 0;
  int usec = 0;
  
  printf("secondi:\n");
  scanf("%d", &sec);
  printf("microsecondi:\n");
  scanf("%d", &usec);
  
  printf("Abilitazione timer\n");
  
  //Decrementerà da it_value a 0
  value.it_value.tv_usec = usec;
  value.it_value.tv_sec = sec;
  //Dopo il segnale it_value sarà reinpostato a 0
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = 0;
  
  signal(SIGVTALRM, printResult);
  
  //Imposta un timer virtual che parte da value ed conta il tempo di CPU usato in user space, quando supero tale limite blocco l'esecuzione e lancio SIGVTALRM, dopo di che l'esecuzione riprende
  if( setitimer(ITIMER_VIRTUAL, &value, &oldvalue) == -1 )
  {
    printf("Errore nel settare il timer\n");
    exit(1);
  }
  
  printf("Attendiamo il segnale\n");
  
  /*Non posso entrare in pause, perchè andrei in deadlock poichè il processo verrebbe messo in blocked e il tempo di CPU non "esisterebbe più". Continuo l'esecuzione su un ciclo infinito. Quando il Timer lancia il segnale la procedura
  di gestione del segnale porta CICLO a 0 uscendo dalla condizione(una simulazione di esecuzione*/
  CICLO = 1;
  while(CICLO);
}

//Abilito un timer che conta solo dal tempo di esecuzione in USRSPACE e KernelSPACE
void profilingTimer()
{
  int sec = 0;
  int usec = 0;
  
  printf("secondi:\n");
  scanf("%d", &sec);
  printf("microsecondi:\n");
  scanf("%d", &usec);
  
  printf("Abilitazione timer\n");
  
  //Decrementerà da it_value a 0
  value.it_value.tv_usec = usec;
  value.it_value.tv_sec = sec;
  //Dopo il segnale it_value sarà reinpostato a 0
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = 0;
  
  signal(SIGPROF, printResult);
  
  //Imposta un timer profiling che parte da value ed conta il tempo di CPU usato in userspace e kernel space, quando supero tale limite blocco l'esecuzione e lancio SIGVTALRM, dopo di che l'esecuzione riprende
  if( setitimer(ITIMER_PROF, &value, &oldvalue) == -1 )
  {
    printf("Errore nel settare il timer\n");
    exit(1);
  }
  
  printf("Attendiamo il segnale\n");
  
  /*Non posso entrare in pause, perchè andrei in deadlock poichè il processo verrebbe messo in blocked e il tempo di CPU non "esisterebbe più". Continuo l'esecuzione su un ciclo infinito. Quando il Timer lancia il segnale la procedura
  di gestione del segnale porta CICLO a 0 uscendo dalla condizione(una simulazione di esecuzione*/
  CICLO = 1;
  while(CICLO);
}

int main(int argn, char* argc[])
{
  CICLO = 0;
  if(argn <= 1)
  {
    printf("Nessun parametro in input\n");
    printhelp();
    return 0;
  }
  
  if(strncmp(argc[1], "-h", 2) == 0 || strncmp(argc[1], "--help", 6) == 0)
    printhelp();
  
  if(strncmp(argc[1], "-real", 5) == 0)
    realTimer();
  
  if(strncmp(argc[1], "-virtual", 8) == 0)
    virtualTimer();
  
  if(strncmp(argc[1], "-profiling", 10) == 0)
    profilingTimer();
  
  printf("Bye Bye :)\n");
}