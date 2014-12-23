#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>

void write(int shid, int semid)
{
  char* s = shmat(shid, 0, SHM_W);
  
  printf("Inserisci la stringa\n");
  scanf("%s", s);
  
  struct sembuf oper;
  oper.sem_num = 0;
  oper.sem_op = -1;
  oper.sem_flg = 0;
  
  if( semop(semid, &oper, 1) == -1 )
    printf("Impossibile sbloccare il semaforo\n");
}

int main(int *argc, char** argv)
{
  int shkey = 31;
  int semkey = 32;
  
  int shid = shmget(shkey, 1024, IPC_CREAT|IPC_EXCL|0666);
  if(shid == -1)
  {
    printf("Shared memory non creata\n");
    return -1;
  }
  
  int semid = semget(semkey, 1, IPC_CREAT|IPC_EXCL|0666);
  if(semid == -1)
  {
    printf("Semafori non inizializzati\n");
    return -1;
  }
  
  if(semctl(semid, 0, SETVAL, 1) == -1)
  {
    printf("Impossibile settare il semaforo a 1\n");
    return -1;
  }
  
  write(shid, semid);
  return 0;
}
