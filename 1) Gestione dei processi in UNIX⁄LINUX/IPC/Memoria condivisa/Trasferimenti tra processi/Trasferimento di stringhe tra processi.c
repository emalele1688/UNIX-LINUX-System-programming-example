#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#define DISP 20
 
void producer(int shid)
{
  char* shmempoint;
  char mess[256];
  
  //Preleva la shared memory puntata da shid in scrittura; Secondo parametro darebbe un indirizzo preferenziale su dove collegare la memoria.
  shmempoint = shmat(shid, NULL, SHM_W);
  do
  {
    printf("Inserisci il messaggio\n");
    scanf("%s", &mess);
    //Copia il messaggio in shmempoint di disp
    //Mi spiazzo in shmempoint. ATTENZIONE, massimo 1024byte di dimensione
    strncpy(shmempoint, mess, DISP);
    shmempoint += DISP;
  }
  while(strcmp(mess, "quit") != 0);
  
  //Scollego la memoria condivisa dal processo (le pagine della memoria condivisa non sono più presenti nella mia page table, ora le possiede solo il kernel)
  if(shmdt(shid) == -1)
    printf("La deallocazione della shared memory in produttore non è riuscita\n");
  exit(0);
}

void consumer(int shid)
{
  //Le pagine dell'area di memoria condivisa sono presenti anche su questo processo
  char* shmempoint = shmat(shid, NULL, SHM_R);
  if(shmempoint == -1)
  {
    printf("Impossibile chiamare la memoria %d\n", shid);
    exit(-1);
  }
  
  while(strcmp(shmempoint, "quit") != 0)
  {
    printf("%s\n", shmempoint);
    shmempoint += DISP;
  }
  
    
  printf("Alcune informazioni sulla memoria condivisa allocata\n");
  struct shmid_ds *buf;
  if( shmctl(shid, IPC_STAT, buf) == -1 )
  {
    printf("Impossibile ottenere certe informazioni\n");
    return -1;
  }
  
  printf("Grandezza del segmento condiviso %d \n", buf->shm_segsz);
  printf("Tempo trascorso dall'ultimo segmento collegato %d \n", buf->shm_atime);
  printf("Tempo trascorso dall'ultimo segmento scollegato %d \n", buf->shm_dtime);
  printf("Tempo trascorso dall'ultimo segmento cambiato %d \n", buf->shm_ctime);;
  printf("Pid del processo creatore %d che corrisponde all'ultimo processo che ha effettuato l'attach %d \n", buf->shm_cpid, buf->shm_cpid);
  
  if(shmdt(shid) == -1)
    printf("La deallocazione della shared memory in consumatore non è riuscita\n");
  
  exit(0);
}

int main(int argc, char** argv)
{
  int shid, status;
  long key = 30;
  size_t size = 1024;
  
  //Alloco un area condivisa tra processi nel Kernel, Tale area ha key = 30, 1024 byte e id restituito.
  shid = shmget(key, size, IPC_CREAT|0666);
  
  //Avvio prima il produttore poi il consumatore
  if(fork())
    wait(&status);
  else
    producer(shid);
  if(fork())
    wait(&status);
  else
    consumer(shid);
  
  if(shmctl(shid, IPC_RMID, NULL) == -1)
    printf("Errore nel deallocare la shared memory\n");
  
  return 0;
}
