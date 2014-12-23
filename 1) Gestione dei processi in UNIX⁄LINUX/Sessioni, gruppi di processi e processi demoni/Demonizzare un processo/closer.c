#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


/* Questo uccide il processo avviato come demone il cui programma è in Deamon process.c */

int main(int argn, char* args[])
{
  pid_t daemon_pid = 0;
  int pipe_ds;
  
  //Porto la directory di lacoro in root
  if(chdir("/") == -1)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
  
  if( (pipe_ds = open("/tmp/dmn", O_RDWR)) == -1)
  {
    printf("%s\n", strerror(errno));
    return 1;
  }
  
  if(read(pipe_ds, &daemon_pid, sizeof(daemon_pid)) == -1)
  {
    printf("%s\n", strerror(errno));
    close(pipe_ds);
    return 1;
  }
  
  printf("Segnale di terminazione inviato al processo demone\n");
  kill(daemon_pid, SIGTERM);
  close(pipe_ds);
  return 0;
}
