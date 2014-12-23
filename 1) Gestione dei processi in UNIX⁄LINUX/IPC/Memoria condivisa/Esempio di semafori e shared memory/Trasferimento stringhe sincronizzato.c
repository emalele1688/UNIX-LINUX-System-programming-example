#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>

#define OFFSET 20

void produttore(int shid, int semid);
void consumatore(int shid, int semid);

int main(int* argc, char** argv)
{
  int status, shid, semid, shkey = 30, semkey = 25;
  
  //Inizializzo la shared memory
  shid = shmget(shkey, 1024, IPC_CREAT|0666);
  if(shid == -1)
  {
    printf("Errore nella chiamata shmget\n");
    return -1;
  }
  
  //Inizializzo un semaforo a dimensione 1
  semid = semget(semkey, 1, IPC_CREAT|0666);
  if(semid == -1)
  {
    printf("Errore nella chiamata semget\n");
    shmctl(shid, IPC_RMID, NULL);
    return -1;
  }
 
  //Inizializzo il semaforo (semnum.val = 0 significa che il semaforo vale inizialmente 0, quindi non è bloccato) può essere omesso in questo caso
  if( semctl(semid, 0, SETVAL, 0) == -1 )
    printf("Impossibile inizializzare il demaforo\n");
  
  if( fork() != 0 )
    if( fork() != 0 )
    {
      wait(&status);
      wait(&status);
    }
    else
    {
      printf("Avvio consumatore\n");
      consumatore(shid, semid);
    }
  else
  {
    printf("Avvio produttore\n");
    produttore(shid, semid);
  }
  
  printf("Rilascio memoria condivisa e semafori\n");
  
  if( shmctl(shid, IPC_RMID, NULL) == -1)
    printf("Memoria condivisa non rilasciata\n");
  
  //La memoria condivisa deve essere rilasciata nel kernel, altrimenti persisterà anche dopo la chiusura dell'oggeto*/
  if( semctl(semid, 0, IPC_RMID, 1) == -1)
    printf("Semaforo non rilasciato\n");
  
  return 0;
}

void produttore(int shid, int semid)
{
  char ins[256];
  char* string = shmat(shid, 0, SHM_W);
  struct sembuf oper;
  
  if(string == (char*)-1)
  {
    printf("Impossibile collegare la memoria %d al processo produttore\n", shid);
    shmctl(shid, IPC_RMID, NULL);
    exit(-1);
  }
  
  do
  {
    printf("Inserisci la stringa, quit per uscire\n");
    scanf("%s", ins);
    strncpy(string, ins, OFFSET);
    string += OFFSET;
  }
  while(strcmp(ins, "quit") != 0);
  
  oper.sem_num = 0; //Solo il primo semaforo
  oper.sem_op = 1; //Il semaforo è -1 (impostato dal consumatore), io lo porto a 0 sbloccandolo
  oper.sem_flg = 0;
  
  if( semop(semid, &oper, 1) == -1)
    printf("Errore nel impostare il semop\n");
  
  exit(0);
}

void consumatore(int shid, int semid)
{
  struct sembuf oper;
  char *string = shmat(shid, 0, SHM_R);
  
  if(string == (char*)-1)
  {
    printf("Impossibile collegare la memoria %d al processo produttore\n", shid);
    shmctl(shid, IPC_RMID, NULL);
    exit(-1);
  }
  
  //Il consumatore entra in attesa lasciando il semaforo a 1, se il consumatore si avvia dopo il produttore, il semaforo vale 0, quindi l'attesa non ci sarà!!
  oper.sem_num = 0;
  oper.sem_op = -1; //Il semaforo è 0, ora lo metto a -1, quindi bloccato
  oper.sem_flg = 0;
  
  if( semop(semid, &oper, 1) == -1 )
    printf("Impossibile accedere al semaforo da parte di consumatore\n");
  
  while( strcmp(string, "quit") != 0)
  {
    printf("%s\n", string);
    string += OFFSET;
  }
  
  exit(0);
}


