#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
//Arma un server a ricevere segnali da SO che gestisce una rutine che invia periodicamente segnali al client. se il client non riceve tali segnali il client termina
typedef struct
{
  char fiforesponse[6];
  char messaggio[15];
} 
messaggio;

int fd0, fd1;
messaggio mess;

void server_inraggiungibile()
{
  printf("Il Server non è più raggiungibile\n");
  close(fd0);
  close(fd1);
  unlink(mess.fiforesponse);
  exit(0);
}

int main(int arg, char* args[])
{
  int ret;
  strcpy(mess.fiforesponse, "clnt");
  
  signal(SIGUSR1, server_inraggiungibile);
  
  fd0 = open("serv", O_RDWR);
  if(fd0 == -1)
  {
    printf("Non è stato possibile contattare il server. E avvaito?\n");
    unlink(mess.fiforesponse);
    exit(1);
  }
  
  ret = mkfifo(mess.fiforesponse, O_CREAT|0666);
  if(ret == -1)
  {
    printf("Impossibile creare un canale di comunicazione verso il server\n");
    unlink(mess.fiforesponse);
    exit(1);
  }
  
  //Invio il mio pid al server
  int id = getpid();
  write(fd0, &id, sizeof(pid_t));
  printf("Inserire il messaggio da inviare al server\n");
  fgets(mess.messaggio, 15, stdin);
  
  write(fd0, &mess, sizeof(messaggio));
  
  fd1 = open(mess.fiforesponse, O_RDWR);
  if(fd1 == -1)
  {
    printf("Impossibile ricevere una risposta dal server\n");
    unlink(mess.fiforesponse);
    exit(1);
  }
  
  read(fd1, &mess, sizeof(messaggio));
  
  printf("Il server ci risponde con: %s", mess.messaggio);
  
  close(fd0);
  close(fd1);
  unlink(mess.fiforesponse);
  
  return 0;
}