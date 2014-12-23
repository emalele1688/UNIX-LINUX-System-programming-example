#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>

#define READER 5

typedef struct 
{
  int destinationproc;
  int stop;
  int numberunit;
  char data[256];
} buffer;

void start(int shid, int semid, int rid, buffer* shbuf);

int main(int *argc, char **argv)
{
  int status;
  int shkey = 30, semkey = 25;
  
  //Inizializzo la shared memory
  int shid = shmget(shkey, 1024, IPC_CREAT|0666);
  if( shid == -1 )
  {
    printf("Si sono verificati problemi nel chiamare la risorsa condivisa con la torre di controllo\n");
    exit(-1);
  }
  
  //Inizializzo un semaforo a dimensione 1
  int semid = semget(semkey, 2, IPC_CREAT|0666);
  if(semid == -1)
  {
    printf("Si sono verificati problemi nel chiamare la risorsa condivisa con la torre di controllo\n");
    exit(-1);
  }
  
  buffer* shbuf;
  if( (shbuf = shmat(shid, 0, SHM_W)) == -1 )
  {
    printf("Impossibile collegare la memoria condivisa a Torre di controllo\n");
    exit(-1);
  }
  
  int rid;
  printf("Numero dell'unità?\n");
  scanf("%d", &rid);
  while( getchar() != '\n' );
  shbuf->numberunit += 1;
  
  start(shid, semid, rid, shbuf);
  
  return 0;
} 

void start(int shid, int semid, int rid, buffer* shbuf)
{  
  struct sembuf oper[2];
  
  while(1)
  {
    //Il client entra in blocco
    printf("BLOCCO\n");
    oper[0].sem_num = 0;
    oper[0].sem_op = -1;
    oper[0].sem_flg = 0;
    if( semop(semid, oper, 1) == -1)
    {
      printf("Errore nella semop reader.1\n");
      return;
    }
    printf("SBLOCCO\n");
    
    if(shbuf->stop == 1)
    {
      printf("Uscita del processo %d", rid);
      exit(0);
    }
    
    if(shbuf->destinationproc == rid)
    { 
      int i = 0;
      printf("Il writer scrive: \n%s\n", shbuf->data);
      printf("\n");
      
      char risp[256];
      printf("Inserisci la tua risposta\n");
      fgets(risp, 256, stdin);
      strcpy(shbuf->data, risp);
      
      oper[0].sem_num = 1;
      oper[0].sem_op = 1;
      oper[0].sem_flg = 0;
      
      if( semop(semid, oper, 1) == -1 )
      {
	printf("Uscita del processo %d \n", rid);
	exit(0);
      }
    }
  }
  
  return;
}