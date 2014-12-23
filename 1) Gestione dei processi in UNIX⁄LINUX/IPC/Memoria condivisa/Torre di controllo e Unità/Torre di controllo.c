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

void start(int shid, int semid, buffer* shbuf);

int main(int *argc, char **argv)
{
  int status;
  int shkey = 30, semkey = 25;
  
  //Inizializzo la shared memory
  int shid = shmget(shkey, 1024, IPC_CREAT|IPC_EXCL|0666);
  if( shid == -1 )
  {
    printf("Primo tentativo di shmget faild\n");
    
    shid = shmget(shkey, 1024, IPC_CREAT|0666);
    shmctl(shid, IPC_RMID, 0);
    
    if( (shid = shmget(shkey, 1024, IPC_CREAT|IPC_EXCL|0666) ) == -1 )
    {
      printf("Secondo tentativo di shmget faild\n");
      exit(-1);
    }
  }
  
  //Inizializzo un semaforo a dimensione 2
  int semid = semget(semkey, 2, IPC_CREAT|IPC_EXCL|0666);
  if(semid == -1)
  {
    printf("Primo tentativo di semget faild\n");
    
    semid = semget(semkey, 2, IPC_CREAT|0666);
    semctl(semid, 0, IPC_RMID);
    
    if( (semid = semget(semkey, 2, IPC_CREAT|IPC_EXCL|0666)) == -1 )
    {
      printf("Secondo tentativo di semget faild, check the problem\n");
      exit(-1);
    }
  } 
  
  buffer* shbuf;
  if( (shbuf = shmat(shid, 0, SHM_W)) == -1 )
  {
    printf("Impossibile collegare la memoria condivisa a Torre di controllo\n");
    exit(-1);
  }
  
  shbuf->destinationproc = 0;
  shbuf->numberunit = 0;

  start(shid, semid, shbuf);
  
  printf("Rilascio memoria condivisa e semafori\n");
  
  if( shmctl(shid, IPC_RMID, NULL) == -1)
    printf("Memoria condivisa non rilasciata\n");
  
  //La memoria condivisa deve essere rilasciata nel kernel, altrimenti persisterà anche dopo la chiusura dell'oggeto*/
  if( semctl(semid, 0, IPC_RMID, 1) == -1)
    printf("Semaforo non rilasciato\n");
  
  return 0;
}

void start(int shid, int semid, buffer* shbuf)
{  
  struct sembuf oper[2];
  while(1)
  {
    int dest;
    printf("Numero del processo con cui vuoi parlare? (0 per terminare)\n");
    scanf("%d", &dest);
    while( getchar() != '\n' );
    
    if(dest == 0)
    {
      shbuf->stop = 1;
      //Tolgo dall'attesa i client
      oper[0].sem_num = 0;
      oper[0].sem_op = shbuf->numberunit;
      oper[0].sem_flg = 0;
      
      if( semop(semid, oper, 1) == -1)
      {
	printf("Errore nella semop writer.1\n");
	exit(-1);
      }
      
      return;
    }
    
    shbuf->destinationproc = dest;
    char text[256];
    printf("Inserisci il messaggio da spedire al client %d\n", dest);
    fgets(text, 256, stdin);
    strcpy(shbuf->data, text);

    //Sblocco i client
    oper[0].sem_num = 0;
    oper[0].sem_op = shbuf->numberunit;
    oper[0].sem_flg = 0;
    
    if( semop(semid, oper, 1) == -1)
    {
      printf("Errore nella semop writer.2\n");
      exit(-1);
    }
    
    //Blocco la torre di controllo
    oper[0].sem_num = 1;
    oper[0].sem_op = -1;
    oper[0].sem_flg = 0;
    
    if( semop(semid, oper, 1) == -1)
    {
      printf("Errore nella semop writer.3\n");
      exit(-1);
    }
    
    printf("Il client risponde \n%s\n", shbuf->data);
    printf("\n");
      
  }
  
  return;
}