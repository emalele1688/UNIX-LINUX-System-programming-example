#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define NFILOSOFI 5
#define SLEEP_OFFSET 3
#define NBACCHETTE NFILOSOFI
#define EAT 2
#define SEMKEY 30

//Per la terminazione
int stop;
//Raccogli i pid dei processi filosofi --- NON UTILIZZATO
pid_t fpid[NFILOSOFI];

void termina()
{
  stop = 1;
}

void pensa(void)
{
  //Stato di pensa
  int sleeptime = 0;
  sleeptime = (int)rand() % SLEEP_OFFSET + 1;
  sleep(sleeptime);
}

void mangia(void)
{
  //Stato di mangia
  sleep(EAT);
}

//Id del semaforo, numero filosofo, bacchetta1, bacchetta2
void filosofo(int semid, int nf, int b1, int b2)
{
  struct sembuf oper[2];
  printf("Il filosofo %d si siede a tavola!\n", nf);
  
  do
  {
    printf("Il filosofo %d pensa\n", nf);
    pensa();
    printf("Il filosofo %d ha finito di pensare, ora mangia se le bacchette sono disponibili, altrimenti attende\n", nf);
    
    //Prendo la mia bacchetta destra
    oper[0].sem_num = b1;
    oper[0].sem_op = -1;
    oper[0].sem_flg = 0;
    
    //Prendo la mia bacchetta sinistra
    oper[1].sem_num = b2;
    oper[1].sem_op = -1;
    oper[1].sem_flg = 0;
    
    //Può mettere il processo nello stato di attesa se una o tutte e due le bacchette non sono disponibili
    if( semop(semid, oper, 2) == -1)
    {
      printf("Problema nel folosofo %d, abbandona la tavola\n", nf);
      exit(1);
    }
    
    printf("Il filosofo %d può mangiare\n", nf);
    mangia();
    printf("Il filosofo %d ha finito di mangiare\n", nf);
    
    //Rilascio le bacchette
    oper[0].sem_num = b1;
    oper[0].sem_op = 1;
    oper[0].sem_flg = 0;
    
    oper[1].sem_num = b2;
    oper[1].sem_op = 1;
    oper[1].sem_flg = 0;
    
    //Può mettere il processo nello stato di attesa se una o tutte e due le bacchette non sono disponibili
    if( semop(semid, oper, 2) == -1 )
    {
      printf("Problema nel folosofo %d, abbandona la tavola\n", nf);
      exit(1);
    }
  }
  while(!stop);
  
  printf("Il filosofo %d, abbandana la tavola\n", nf);
  
  exit(0);
}

int main(int args, char* argc[])
{
  int ret, i, semid;
  stop = 0;
  
  //Imposto la terminazione catturando SIGINT
  signal(SIGINT, termina);
  
  if( (semid = semget(SEMKEY, NFILOSOFI, IPC_CREAT|0666)) == -1 )
  {
    semctl(semid, NFILOSOFI, IPC_RMID);
    if( (semid = semget(SEMKEY, NFILOSOFI, IPC_CREAT|0666)) == -1 )
    {
      printf("Impossibile creare un semaforo\n");
      exit(1);
    }
  }
  
  semctl(semid, 0, SETVAL, 1);
  semctl(semid, 1, SETVAL, 1);
  semctl(semid, 2, SETVAL, 1);
  semctl(semid, 3, SETVAL, 1);
  semctl(semid, 4, SETVAL, 1);
  
  //Inizializza il random seed
  srand(time(NULL));
  
  //Creo i 5 filosofi
  for(i = 0; i < NFILOSOFI; i++)
  {
    ret = fork();
    //Il primo filosofo ha l'ultima bacchetta (NBACCHETTE-1 poichè la prima bacchetta è 0) e la prima bacchetta (i = 0)
    if(ret == 0 && i == 0)
      filosofo(semid, i, NBACCHETTE-1, i);
    else if(ret == 0 && i > 0)
      //i-1 è la sinistra, i è la bacchetta destra (infatti il primo e l'ultimo filosofo condividono l'ultima bacchetta
      filosofo(semid, i, i-1, i);
    else if(ret == -1)
    {
      printf("Il sistema si rifiuta di creare altri processi! Termino\n");
      exit(0);
    }
    fpid[i] = ret;
  }
  
  for(i = 0; i < NFILOSOFI; i++)
    wait(0);
  
  printf("Bye Bye!\n");
  
  return 0;
}
