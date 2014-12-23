#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>


int pipe_ds = 0;

void interrupt(int sig);
void start_daemon(const char* pname);
void exit_daemon(void);
void task(void);

void interrupt(int sig)
{
  syslog(LOG_NOTICE, "Terminazione per segnale SGTERM");
  exit_daemon();
}

void task(void)
{
  syslog(LOG_INFO, "Entro in sleep");
  //Task simulator
  sleep(30);
}

void start_daemon(const char* pname)
{ 
  //Imposto una nuova sessione per il processo demone
  if(setsid() == -1)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
    
  //Setup dei segnali
  struct sigaction sig;
  memset(&sig, 0, sizeof(sig));
  sigemptyset(&sig.sa_mask);
  sig.sa_handler = interrupt;
  sigaction(SIGTERM, &sig, NULL);
  
  //Ignoro il segnale di output sul terminale per il processo in background
  if(signal(SIGTTOU, SIG_IGN))
    printf("%s\n", strerror(errno));
  //Ignoro il segnale di input sul terminale per il processo in background
  if(signal(SIGTTIN, SIG_IGN))
    printf("%s\n", strerror(errno));
  //Ignoro l'azione della pressione del tasto stop su terminale
  if(signal(SIGTSTP, SIG_IGN))
    printf("%s\n", strerror(errno));
  //Ignoro l'azione di chiusura del terminale
  if(signal(SIGHUP, SIG_IGN))
    printf("%s\n", strerror(errno));
  //Ignoro l'azione di terminazione di un processo figlio
  if(signal(SIGCHLD, SIG_IGN))
    printf("%s\n", strerror(errno));
  
  //Imposto la maschera dei permessi predefinità (sulla shell dove il demone è stato lanciato potrebbe essere stata cambiata dall'utente)
  umask(0);
  
  //Cambio la directory di lavoro
  if(chdir("/") == -1)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
  
  //Apro una pipe per far conoscere il mio pid al processo closer che si preocuperà di uccidermi
  if(mkfifo("tmp/dmn", O_CREAT|0666) == -1)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
  pipe_ds = open("tmp/dmn", O_RDWR);
  if(pipe_ds == -1)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
  
  pid_t pid = getpid();
  while( write(pipe_ds, &pid, sizeof(pid_t)) < sizeof(pid_t))
  {
    if(errno == EINTR)
      continue;
        
    printf("%s\n", strerror(errno));
    close(pipe_ds);
    unlink("tmp/dmn");
    exit(1);      
  }
  

  //Chiudo gli standard file descriptor
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /* Preparo syslog (demone di supporto per le notifiche) */
  //Imposto la maschera: quale tipo di messaggi salvare in syslog
  setlogmask(LOG_UPTO (LOG_NOTICE));
  //Connessione al demone syslog: Nome programma, categorie del messaggio, modalità di scrittura
  openlog(pname, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);  
  
  //Eseguo il task con notifica in syslog
  syslog(LOG_NOTICE, "Daemon start");
  task();
  
  exit_daemon();
}

void exit_daemon(void)
{
  close(pipe_ds);
  unlink("tmp/dmn");
  exit(0);
}


int main(int argn, char* args[])
{
  int ret = 0;
  
  printf("Avvio del demone in corso\n");
  
  ret = fork();
  
  switch(ret)
  {
    case -1:
      printf("%s\n", strerror(errno));
      exit(1);
    case 0:
      start_daemon(args[0]);
    default:
      //Chiudo il processo padre
      exit(0);
  }
  
  return 0;
}