#include <stdio.h>

int main(int argc, char* argv[])
{
  char messaggio[30];
  int status = 0, pid, fd[2];
  
  //Creiamo una pipe dove fd[0] è il canale di lettura, fd[1] è il canale di scrittura
  if(pipe(fd) == -1)
  {
    printf("Impossibile creare una pipe\n");
    exit(-1);
  }
  
  if( (pid = fork()) == -1)
  {
    printf("Impossibile creare il processo lettore\n");
    exit(-1);
  }
  
  if(pid == 0)
  {
    //Processo figlio--------------------------------------
    
    //Chiusura canale 1 della pipe (canale di scrittura)
    close(fd[1]);
    //Fin chè c'è da leggere, attendi se il canale è aperto e non c'è nulla sul buffer
    while( read(fd[0], messaggio, 30) > 0 )
      printf("Messaggio: %s\n", messaggio);
    
    //Chiusura canale di lettura
    close(fd[0]);
  }
  else
  {
    //Processo padre--------------------------------------
    
    //Chiusura canale 0 della pipe (canale di lettura)
    close(fd[0]);
    //Fin che non inserisci quit
    do
    {
      printf("Inserisci il messaggio da spedire al figlio\n");
      fgets(messaggio, 30, stdin);
      //Scrivi sulla pipe
      write(fd[1], messaggio, 30);
    }
    while( strncmp(messaggio, "quit", strlen(messaggio) -1 ) != 0);
    
    //Chiusura del canale di scrittura, causa la terminazione dell'attesa sulla read di fd[0]
    close(fd[1]);
  }
  
  wait(&status);
}
