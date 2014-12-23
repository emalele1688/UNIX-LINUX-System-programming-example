/* CHILD3 ENTRA IN ATTESA SU 1 E SARA' RILASCIATO DA CHILD2 DOPO 5 SEC, POICHE' CHILD2, MENTRE RILASCIA CHILD3 ENTRA IN ATTESA DI CHILD1 SU 0; CHILD1 RILASCIA CHILD2 SU 0 DOPO 4 SECONDI, CHE CHILD2 RILASCIERA' QUINDI CHILD3 SU
 * 1 DOPO 5 SECONDI*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

void childproc3(int semid)
{
  struct sembuf oper[2];
  
  printf("child3\n");
  
  //Entra in attesa sul semaforo 1
  oper[0].sem_num = 1;
  oper[0].sem_op = -1;
  oper[0].sem_flg = 0;
  
  if( semop(semid, &oper, 1) == -1)
  {
    printf("Errore su semop 3.c1\n");
    exit(-1);
  }
  printf("child3 esce dall'attesa\n");
  printf("Uscita di child3\n");
  exit(0);
}

void childproc1(int semid)
{
  struct sembuf oper[2];
  
  sleep(4);
  printf("child1\n");
  printf("child1 rilascia child2 dall'attesa\n");
  
  //Rilascia il semaforo 0, sul quale è in attesa proc2
  oper[0].sem_num = 0;
  oper[0].sem_op = 1;
  oper[0].sem_flg = 0;
  
  if( semop(semid, &oper, 1) == -1)
  {
    printf("Errore su semop 1.c1\n");
    exit(-1);
  }
  
  printf("Uscita di child1\n");
  exit(0);
}

void childproc2(int semid)
{
  struct sembuf oper[2];
  
  sleep(1);
  printf("child2\n");
  printf("child2 entra in attesa\n");
  
  //Entro in attesa su 0
  oper[0].sem_num = 0;
  oper[0].sem_op = -1;
  oper[0].sem_flg = 0;
  
  //Rilascierò, dopo l'attesa, il semaforo 1 sul quale è in attesa child3
  oper[1].sem_num = 1;
  oper[1].sem_op = 1;
  oper[1].sem_flg = 0;
  
  if( semop(semid, &oper, 2) == -1)
  {
    printf("Errore su semop 1.c2\n");
    exit(-1);
  }
  //CHILD2 LEVA DALL'ATTESA CHILD3, SOLO QUANDO PERÒ CHILD2 NON E' PIU' IN ATTESA!! QUINDI QUANDO CHILD1 RILASCIA CHILD2
  printf("child2 esce dall'attesa\n");
  
  printf("Uscita di child2\n");
  exit(0);
}

int main(int* argc, char** argv)
{
  int semkey = 88;
  
  int semid = semget(semkey, 2, IPC_CREAT|IPC_EXCL|0666);
  if(semid == -1)
  {
    printf("Primo tentativo di semget faild\n");
    
    semid = semget(semkey, 1, IPC_CREAT|0666);
    semctl(semid, 0, IPC_RMID);
    
    if( (semid = semget(semkey, 1, IPC_CREAT|IPC_EXCL|0666)) == -1 )
    {
      printf("Secondo tentativo di semget faild, check the problem\n");
      exit(-1);
    }
  }
  
  printf("START-----------------------------\n");
  int status = 0;
  if( fork() != 0)
    if( fork() != 0)
      if( fork() != 0)
      {
	wait(&status);
	wait(&status);
	wait(&status);
      }
      else
	childproc3(semid);
    else 
      childproc1(semid);
  else
    childproc2(semid);
  
  printf("END-------------------------------\n");
  printf("Rilascio risorse\n");
  
  semctl(semid, 0, IPC_RMID);
  
  return 0;
}