#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>

void read(int shid, int semid)
{  
  struct sembuf oper;
  oper.sem_num = 0;
  oper.sem_op = 0;
  oper.sem_flg = 0;
  
  if( semop(semid, &oper, 1) == -1 )
    printf("Impossibile sbloccare il semaforo\n");
  
  char* s = shmat(shid, 0, SHM_R);
  printf("%s\n", s);
}

int main(int *argc, char** argv)
{
  int shkey = 31;
  int semkey = 32;
  
  int shid = shmget(shkey, 1024, IPC_CREAT|0666);
  if(shid == -1)
  {
    printf("Shared memory non creata\n");
    return -1;
  }
  
  int semid = semget(semkey, 1, IPC_CREAT|0666);
  if(semid == -1)
  {
    printf("Semafori non inizializzati\n");
    return -1;
  }
  
  read(shid, semid);
  
  if( shmctl(shid, IPC_RMID, 0) == -1 )
    printf("Impossibile deallocare la memoria condivisa\n");
  
  if( semctl(semid, 0, IPC_RMID, 0) == -1 )
    printf("Impossibile deallocare il semaforo\n");
  
  return 0;
}
                                                                                                                       
