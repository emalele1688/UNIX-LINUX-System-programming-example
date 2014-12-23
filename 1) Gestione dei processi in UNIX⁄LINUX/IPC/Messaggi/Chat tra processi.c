#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>

#define MESSIZE 384

typedef struct
{
  //type esprime a chi recapitare il messaggio
  long type;
  char data[256];
  char username[128];
} message;

void *writer(void* msgid);
void *reader(void* msgid);

message* msg;
int msgid;

int main(int *argc, char **argv)
{
  int msgkey = 10;
  msgid = msgget(msgkey, IPC_CREAT|0666);
  if(msgid == -1)
  {
    printf("Impossibile creare una cosa per i messaggi.\n");
    exit(-1);
  }

  pthread_t wr, rd;
  int myid;
  msg = malloc(MESSIZE);
  printf("Username: \n");
  scanf("%s", msg->username);
  printf("Inserisci id di riconoscimento\n");
  scanf("%d", &myid);
  while(getchar() != '\n');
  
  if(pthread_create(&wr, NULL, writer, NULL) == -1)
  {
    printf("Impossibile creare un thread per lo scrittore\n");
    exit(-1);
  }
  
  if(pthread_create(&rd, NULL, reader, &myid) == -1)
  {
    printf("Impossibile creare un thread per lo lettore\n");
    exit(-1);
  } 
  
  if(pthread_join(wr, NULL) == -1)
    printf("Impossibile attendere il writer\n");
  if(pthread_join(rd, NULL) == -1)
    printf("Impossibile attendere il reader\n");
  
  //Non sarà eseguito perchè non faccio terminare i due thread(non giusto!)
  msgctl(msgid, IPC_RMID, NULL);
  
  free(msg);
  
  return 0;
}

void *writer(void* arg)
{
  printf("Start writer %s\n", msg->username);
  long dest = 0;
  char msd[256];
  while(1)
  {
    printf("Sceglia a chi inviare il messaggio \n");
    scanf("%d", &dest);
    while(getchar() != '\n');
    msg->type = dest;
    printf("Inserisci il messaggio\n");
    fgets(msd, 256, stdin);
    strcpy(msg->data, msd);
    if(msgsnd(msgid, msg, MESSIZE, IPC_NOWAIT) == -1)
      printf("Impossibile inviare il messaggio\n");
  }
  
  printf("Close writer %s\n", msg->username);
  pthread_exit(NULL);
}

void *reader(void* arg)
{
  printf("Start reader %s\n", msg->username);
  int *myid = (int*)arg;
  message mrcv;
  while(1)
  {
    if(msgrcv(msgid, &mrcv, MESSIZE, *myid, 0) == -1)
      printf("Impossibile ricevere il messaggio\n");
    printf("%s:\n", mrcv.username);
    printf("%s\n", mrcv.data);
  }
  
  printf("Close reader %s\n", msg->username);
  pthread_exit(NULL);
}
